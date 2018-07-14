#ifndef TURBO_CONTAINER_CONCURRENT_VECTOR_HXX
#define TURBO_CONTAINER_CONCURRENT_VECTOR_HXX

#include <turbo/container/concurrent_vector.hpp>
#include <limits>
#include <turbo/algorithm/recovery.hh>
#include <turbo/container/mpmc_ring_queue.hh>
#include <turbo/math/power.hpp>
#include <turbo/math/summation.hpp>
#include <turbo/toolset/extension.hpp>
#include <turbo/toolset/intrinsic.hpp>

namespace turbo {
namespace container {

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::node::node()
    :
	value(),
	guard(versioned_guard::create(0U, node::status::ready))
{ }

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::descriptor::descriptor()
    :
	size(0U),
	capacity(0U),
	has_pending_write(false),
	expected_version(0U),
	new_value(),
	location(0U)
{ }

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::descriptor::descriptor(
	capacity_type size_,
	capacity_type capacity_,
	bool pending_,
	std::uint16_t version_,
	value_t&& value_,
	capacity_type location_)
    :
	size(size_),
	capacity(capacity_),
	expected_version(version_),
	new_value(std::move(value_)),
	location(location_)
{
    has_pending_write.store(pending_, std::memory_order_release);
}

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::concurrent_vector(
	std::uint8_t initial_capacity_exponent,
	std::uint8_t max_capacity_exponent)
    :
	concurrent_vector(initial_capacity_exponent, max_capacity_exponent, 0U)
{ }

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::concurrent_vector(
	std::uint8_t initial_capacity_exponent,
	std::uint8_t max_capacity_exponent,
	std::uint16_t max_concurrent_writes)
    :
	initial_exponent_(initial_capacity_exponent),
	max_exponent_(max_capacity_exponent),
	buckets_(),
	descriptors_(),
	current_descriptor_(descriptor_reference::create(0U, 0U)),
	free_descriptors_(max_concurrent_writes, 0U)
{
    if (std::numeric_limits<capacity_type>::digits < max_exponent_)
    {
	throw invalid_capacity_argument("Maximum capacity exponent cannot exceed the number of digit bits in capacity_type");
    }
    else if (max_exponent_ < initial_exponent_)
    {
	throw invalid_capacity_argument("Maximum capacity cannot be less than initial capacity");
    }
    else
    {
	buckets_.reset(new std::atomic<node*>[max_exponent_ - initial_exponent_]);
	const std::size_t first_bucket_capacity = turbo::math::pow(capacity_base_, initial_exponent_);
	buckets_[0] = new node[first_bucket_capacity];
	const std::size_t range = max_exponent_ - initial_exponent_;
	for (std::size_t iter = 1U; iter < range; ++iter)
	{
	    buckets_[iter] = nullptr;
	}
	const std::size_t descriptor_count = (max_concurrent_writes == 0U ) ?
		std::numeric_limits<std::uint16_t>::max() :
		max_concurrent_writes;
	descriptors_.reset(new descriptor[descriptor_count]);
	new (&(descriptors_[0])) descriptor(0U, first_bucket_capacity, false, 0U, value_t(), 0U);
	for (throughput_type index = 1U; index < max_concurrent_writes; ++index)
	{
	    free_descriptors_.try_enqueue_copy(index);
	}
    }
}

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::~concurrent_vector()
{
    const std::size_t range = max_exponent_ - initial_exponent_;
    for (std::size_t iter = 0U; iter < range; ++iter)
    {
	node* tmp = buckets_[iter].load(std::memory_order_acquire);
	if (tmp != nullptr)
	{
	    delete[] tmp;
	    buckets_[iter].store(nullptr, std::memory_order_release);
	}
    }
}

template <class value_t, template <class type_t> class allocator_t>
value_t& concurrent_vector<value_t, allocator_t>::operator[](capacity_type index)
{
    subscript_type subscript = find_subscript(index);
    return buckets_[subscript.first][subscript.second].value;
}

template <class value_t, template <class type_t> class allocator_t>
const value_t& concurrent_vector<value_t, allocator_t>::operator[](capacity_type index) const
{
    subscript_type subscript = find_subscript(index);
    return buckets_[subscript.first][subscript.second].value;
}

template <class value_t, template <class type_t> class allocator_t>
value_t& concurrent_vector<value_t, allocator_t>::at(capacity_type index)
{
    check_range(index);
    return (*this)[index];
}


template <class value_t, template <class type_t> class allocator_t>
const value_t& concurrent_vector<value_t, allocator_t>::at(capacity_type index) const
{
    check_range(index);
    return (*this)[index];
}

template <class value_t, template <class type_t> class allocator_t>
typename concurrent_vector<value_t, allocator_t>::change_result concurrent_vector<value_t, allocator_t>::try_pushback(value_t&& value)
{
    typename descriptor_reference::type current_reference = current_descriptor_.load(std::memory_order_acquire);
    descriptor& current = descriptors_[descriptor_reference::value(current_reference)];
    change_result write_result = complete_write(current);
    if (write_result != change_result::success)
    {
	return write_result;
    }
    const capacity_type first_bucket_capacity = static_cast<capacity_type>(turbo::math::pow(capacity_base_, initial_exponent_));
    capacity_type bucket_index = find_subscript(current.size + first_bucket_capacity).first - find_subscript(first_bucket_capacity).first;
    const capacity_type total_capacity = current.capacity + allocate_bucket(bucket_index);
    throughput_type free_descriptor = 0U;
    change_result result = change_result::success;
    turbo::algorithm::recovery::retry_with_random_backoff([&] ()
    {
	switch (free_descriptors_.try_dequeue_copy(free_descriptor))
	{
	    case decltype(free_descriptors_)::consumer::result::success:
	    {
		result = change_result::success;
		return turbo::algorithm::recovery::try_state::done;
	    }
	    case decltype(free_descriptors_)::consumer::result::queue_empty:
	    {
		result = change_result::max_write_reached;
		return turbo::algorithm::recovery::try_state::done;
	    }
	    default:
	    {
		return turbo::algorithm::recovery::try_state::retry;
	    }
	}
    });
    if (result != change_result::success)
    {
	return result;
    }
    typename node::versioned_guard::type current_guard = get_node(current.size).guard.load(std::memory_order_acquire);
    new (&(descriptors_[free_descriptor])) descriptor(
	    current.size + 1,
	    total_capacity,
	    true,
	    node::versioned_guard::version(current_guard),
	    std::move(value),
	    current.size);
    typename descriptor_reference::type new_reference = descriptor_reference::create(descriptor_reference::version(current_reference) + 1, free_descriptor);
    if (current_descriptor_.compare_exchange_strong(current_reference, new_reference, std::memory_order_acq_rel))
    {
	return complete_write(descriptors_[descriptor_reference::value(new_reference)]);
    }
    else
    {
	turbo::algorithm::recovery::retry_with_random_backoff([&] ()
	{
	    switch (free_descriptors_.try_enqueue_copy(free_descriptor))
	    {
		case decltype(free_descriptors_)::producer::result::success:
		{
		    return turbo::algorithm::recovery::try_state::done;
		}
		case decltype(free_descriptors_)::producer::result::queue_full:
		{
		    // It means that somehow the descriptor free list is corrupt!
		    // Better to throw here than cause memory corruption later on
		    throw corrupt_vector_error("Descriptor free list should not be full because an entry was just dequeued attempting a pushback");
		}
		default:
		{
		    return turbo::algorithm::recovery::try_state::retry;
		}
	    }
	});
	return change_result::beaten;
    }
}

template <class value_t, template <class type_t> class allocator_t>
typename concurrent_vector<value_t, allocator_t>::node& concurrent_vector<value_t, allocator_t>::get_node(capacity_type index)
{
    subscript_type subscript = find_subscript(index);
    return buckets_[subscript.first][subscript.second];
}

template <class value_t, template <class type_t> class allocator_t>
const typename concurrent_vector<value_t, allocator_t>::node& concurrent_vector<value_t, allocator_t>::get_node(capacity_type index) const
{
    subscript_type subscript = find_subscript(index);
    return buckets_[subscript.first][subscript.second];
}

template <class value_t, template <class type_t> class allocator_t>
typename concurrent_vector<value_t, allocator_t>::subscript_type concurrent_vector<value_t, allocator_t>::find_subscript(capacity_type index) const
{
    if (index == 0)
    {
	return std::make_pair(0, 0);
    }
    capacity_type high_bit = std::numeric_limits<capacity_type>::digits - turbo::toolset::count_leading_zero(index) - 1U;
    if (high_bit < initial_exponent_)
    {
	return std::make_pair(0, index);
    }
    capacity_type bucket_index = high_bit - initial_exponent_ + 1U;
    capacity_type node_index = index ^ (turbo::math::pow(capacity_base_, high_bit));
    return std::make_pair(bucket_index, node_index);
}

template <class value_t, template <class type_t> class allocator_t>
void concurrent_vector<value_t, allocator_t>::check_range(capacity_type index) const
{
    subscript_type subscript = find_subscript(index);
    const std::size_t range = max_exponent_ - initial_exponent_;
    if (range < subscript.first)
    {
	throw std::out_of_range("Requested element is not in range");
    }
    else if (buckets_[subscript.first] == nullptr)
    {
	throw std::out_of_range("Requested element is not in range");
    }
}

// TODO: current reference should be the argument?
template <class value_t, template <class type_t> class allocator_t>
typename concurrent_vector<value_t, allocator_t>::change_result concurrent_vector<value_t, allocator_t>::complete_write(descriptor& operation)
{
    change_result result = change_result::success;
    bool is_pending = operation.has_pending_write.load(std::memory_order_acquire);
    if (is_pending)
    {
	typename node::versioned_guard::type current = get_node(operation.location).guard.load(std::memory_order_acquire);
	if (node::versioned_guard::value(current) == node::status::ready && node::versioned_guard::version(current) == operation.expected_version)
	{
	    typename node::versioned_guard::type tmp = node::versioned_guard::create(node::versioned_guard::version(current), node::status::updating);
	    if (get_node(operation.location).guard.compare_exchange_strong(current, tmp, std::memory_order_acq_rel))
	    {
		turbo::algorithm::recovery::try_and_ensure(
		[&] ()
		{
		    get_node(operation.location).value = operation.new_value;
		    result = change_result::success;
		},
		[&] ()
		{
		    get_node(operation.location).guard.store(
			    node::versioned_guard::create(static_cast<std::uint16_t>(node::versioned_guard::version(current) + 1U), node::status::ready),
			    std::memory_order_release);
		});
	    }
	    else
	    {
		result = change_result::beaten;
	    }
	}
	else if (node::versioned_guard::version(current) != operation.expected_version)
	{
	    result = change_result::beaten;
	}
	else
	{
	    result = change_result::busy;
	}
	operation.has_pending_write.compare_exchange_strong(is_pending, false, std::memory_order_release);
    }
    else
    {
	result = change_result::success;
    }
    return result;
}

template <class value_t, template <class type_t> class allocator_t>
typename concurrent_vector<value_t, allocator_t>::capacity_type concurrent_vector<value_t, allocator_t>::allocate_bucket(capacity_type bucket_index)
{
    const capacity_type bucket_size = static_cast<capacity_type>(turbo::math::pow(capacity_base_, initial_exponent_ + bucket_index));
    node* current_bucket = buckets_[bucket_index].load(std::memory_order_acquire);
    if (current_bucket == nullptr)
    {
	node* new_bucket = new node[bucket_size];
	if (!buckets_[bucket_index].compare_exchange_strong(current_bucket, new_bucket, std::memory_order_acq_rel))
	{
	    delete[] new_bucket;
	}
    }
    return bucket_size;
}

} // namespace container
} // namespace turbo

#endif
