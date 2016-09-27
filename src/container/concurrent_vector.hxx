#ifndef TURBO_CONTAINER_CONCURRENT_VECTOR_HXX
#define TURBO_CONTAINER_CONCURRENT_VECTOR_HXX

#include <turbo/container/concurrent_vector.hpp>
#include <limits>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/math/power.hpp>
#include <turbo/math/summation.hpp>
#include <turbo/toolset/extension.hpp>
#include <turbo/toolset/intrinsic.hpp>

namespace turbo {
namespace container {

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::descriptor::descriptor()
    :
	size(0U),
	capacity(0U),
	expected_version(0U),
	new_value(),
	location(nullptr)
{
    has_pending_write.store(false, std::memory_order_release);
}

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::descriptor::descriptor(
	std::size_t size_,
	std::size_t capacity_,
	bool pending_,
	std::uint16_t version_,
	value_t&& value_,
	node* location_)
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
	segments_(),
	descriptors_(),
	current_descriptor_({ 0U, 0U })
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
	segments_.reset(new std::atomic<node*>[max_exponent_ - initial_exponent_]);
	const std::size_t first_segment_capacity = turbo::math::pow(capacity_base_, initial_exponent_);
	segments_[0] = new node[first_segment_capacity];
	const std::size_t range = max_exponent_ - initial_exponent_;
	for (std::size_t iter = 1U; iter < range; ++iter)
	{
	    segments_[iter] = nullptr;
	}
	const std::size_t descriptor_count = (max_concurrent_writes == 0U ) ?
		std::numeric_limits<std::uint16_t>::max() :
		max_concurrent_writes;
	descriptors_.reset(new descriptor[descriptor_count]);
	new (&(descriptors_[0])) descriptor(0U, first_segment_capacity, false, 0U, value_t(), nullptr);
    }
}

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::~concurrent_vector()
{
    const std::size_t range = max_exponent_ - initial_exponent_;
    for (std::size_t iter = 0U; iter < range; ++iter)
    {
	node* tmp = segments_[iter].load(std::memory_order_acquire);
	if (tmp != nullptr)
	{
	    delete[] tmp;
	    segments_[iter].store(nullptr, std::memory_order_release);
	}
    }
}

template <class value_t, template <class type_t> class allocator_t>
value_t& concurrent_vector<value_t, allocator_t>::operator[](capacity_type index)
{
    subscript_type subscript = find_subscript(index);
    return segments_[subscript.first][subscript.second].value;
}

template <class value_t, template <class type_t> class allocator_t>
const value_t& concurrent_vector<value_t, allocator_t>::operator[](capacity_type index) const
{
    subscript_type subscript = find_subscript(index);
    return segments_[subscript.first][subscript.second].value;
}

template <class value_t, template <class type_t> class allocator_t>
value_t& concurrent_vector<value_t, allocator_t>::at(capacity_type index)
{
    range_check(index);
    return (*this)[index];
}


template <class value_t, template <class type_t> class allocator_t>
const value_t& concurrent_vector<value_t, allocator_t>::at(capacity_type index) const
{
    range_check(index);
    return (*this)[index];
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
    capacity_type segment = high_bit - initial_exponent_ + 1U;
    capacity_type position = index ^ (turbo::math::pow(capacity_base_, high_bit));
    return std::make_pair(segment, position);
}

template <class value_t, template <class type_t> class allocator_t>
void concurrent_vector<value_t, allocator_t>::range_check(capacity_type index) const
{
    subscript_type subscript = find_subscript(index);
    const std::size_t range = max_exponent_ - initial_exponent_;
    if (range < subscript.first)
    {
	throw std::out_of_range("Requested element is not in range");
    }
    else if (segments_[subscript.first] == nullptr)
    {
	throw std::out_of_range("Requested element is not in range");
    }
}

template <class value_t, template <class type_t> class allocator_t>
void concurrent_vector<value_t, allocator_t>::complete_write(descriptor& operation)
{
    bool is_pending = operation.has_pending_write.load(std::memory_order_acquire);
    if (is_pending)
    {
	if (operation.location != nullptr)
	{
	    typename node::versioned_guard guard = operation.location->guard.load(std::memory_order_acquire);
	    if (guard.guard_status == node::status::ready && guard.guard_version == operation.expected_version)
	    {
		operation.location.guard.store({ node::status::updating, guard.guard_version}, std::memory_order_release);
		turbo::algorithm::recovery::try_and_ensure(
		[&] ()
		{
		    operation.location.value = operation.new_value;
		},
		[&] ()
		{
		    operation.location.guard.store({ node::status::ready, guard.guard_version + 1}, std::memory_order_release);
		});
	    }
	}
	operation.has_pending_write.store(false, std::memory_order_release);
    }
}

} // namespace container
} // namespace turbo

#endif
