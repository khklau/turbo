#ifndef TURBO_CONTAINER_MPMC_RING_QUEUE_HXX
#define TURBO_CONTAINER_MPMC_RING_QUEUE_HXX

#include <turbo/container/mpmc_ring_queue.hpp>
#include <limits>
#include <stdexcept>
#include <turbo/algorithm/recovery.hxx>

namespace turbo {
namespace container {

template <class value_t, template <class type_t> class allocator_t>
class mpmc_key
{
    mpmc_key() { }
    friend class mpmc_ring_queue<value_t, allocator_t>;
};

template <class value_t, template <class type_t> class allocator_t>
mpmc_producer<value_t, allocator_t>::mpmc_producer(const key&, mpmc_ring_queue<value_t, allocator_t>& queue)
    :
	queue_(queue)
{ }

template <class value_t, template <class type_t> class allocator_t>
mpmc_producer<value_t, allocator_t>::mpmc_producer(const mpmc_producer& other)
    :
	queue_(other.queue_)
{ }

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_producer<value_t, allocator_t>::result mpmc_producer<value_t, allocator_t>::try_enqueue_copy(const value_t& input)
{
    return queue_.try_enqueue_copy(input);
}

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_producer<value_t, allocator_t>::result mpmc_producer<value_t, allocator_t>::try_enqueue_move(value_t&& input)
{
    return queue_.try_enqueue_move(std::move(input));
}

template <class value_t, template <class type_t> class allocator_t>
mpmc_consumer<value_t, allocator_t>::mpmc_consumer(const key&, mpmc_ring_queue<value_t, allocator_t>& queue)
    :
	queue_(queue)
{ }

template <class value_t, template <class type_t> class allocator_t>
mpmc_consumer<value_t, allocator_t>::mpmc_consumer(const mpmc_consumer& other)
    :
	queue_(other.queue_)
{ }

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_consumer<value_t, allocator_t>::result mpmc_consumer<value_t, allocator_t>::try_dequeue_copy(value_t& output)
{
    return queue_.try_dequeue_copy(output);
}

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_consumer<value_t, allocator_t>::result mpmc_consumer<value_t, allocator_t>::try_dequeue_move(value_t& output)
{
    return queue_.try_dequeue_move(output);
}

template <class value_t, template <class type_t> class allocator_t>
mpmc_ring_queue<value_t, allocator_t>::mpmc_ring_queue(uint32_t capacity)
    :
	mpmc_ring_queue(capacity, 0U)
{ }

template <class value_t, template <class type_t> class allocator_t>
mpmc_ring_queue<value_t, allocator_t>::mpmc_ring_queue(uint32_t capacity, uint16_t handle_limit)
    :
	buffer_(capacity),
	head_(0),
	tail_(0),
	producer_list(handle_limit, key(), *this),
	consumer_list(handle_limit, key(), *this)
{
    // TODO: when a constexpr version of is_lock_free is available do this check as a static_assert
    if (!head_.is_lock_free() || !tail_.is_lock_free())
    {
	throw std::invalid_argument("uin32_t is not atomic on this platform");
    }
    for (auto iter = buffer_.begin(); iter != buffer_.end(); ++iter)
    {
	if (!iter->guard.is_lock_free())
	{
	    throw std::invalid_argument("uin8_t is not atomic on this platform");
	}
	iter->guard.store(node_type::status::unused, std::memory_order_release);
    }
}

template <class value_t, template <class type_t> class allocator_t>
template <class handle_t>
mpmc_ring_queue<value_t, allocator_t>::mpmc_ring_queue::handle_list<handle_t>::handle_list(
	uint16_t limit,
	const key& the_key,
	mpmc_ring_queue<value_t, allocator_t>& queue)
    :
	counter(0),
	list(limit, handle_t(the_key, queue))
{ }

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_ring_queue<value_t, allocator_t>::producer& mpmc_ring_queue<value_t, allocator_t>::get_producer()
{
    uint16_t count = 0;
    do
    {
	count = producer_list.counter.load(std::memory_order_acquire);
	if (count >= producer_list.list.size())
	{
	    throw std::range_error("No more producers are available");
	}
    }
    while (!producer_list.counter.compare_exchange_strong(count, count + 1, std::memory_order_release));
    return producer_list.list[count];
}

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_ring_queue<value_t, allocator_t>::consumer& mpmc_ring_queue<value_t, allocator_t>::get_consumer()
{
    uint16_t count = 0;
    do
    {
	count = consumer_list.counter.load(std::memory_order_acquire);
	if (count >= consumer_list.list.size())
	{
	    throw std::range_error("No more consumers are available");
	}
    }
    while (!consumer_list.counter.compare_exchange_strong(count, count + 1, std::memory_order_release));
    return consumer_list.list[count];
}

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_producer<value_t, allocator_t>::result mpmc_ring_queue<value_t, allocator_t>::try_enqueue_copy(const value_t& input)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    // for unsigned integrals nothing extra is needed to handle overflow
    if (head - tail == buffer_.capacity())
    {
	return producer::result::queue_full;
    }
    typename node_type::status guard = buffer_[head % buffer_.capacity()].guard.load(std::memory_order_acquire);
    if (guard == node_type::status::unused)
    {
	if (head_.compare_exchange_strong(head, head + 1, std::memory_order_release))
	{
	    buffer_[head % buffer_.capacity()].value = input;
	    buffer_[head % buffer_.capacity()].guard.store(node_type::status::used, std::memory_order_release);
	    return producer::result::success;
	}
	else
	{
	    return producer::result::beaten;
	}
    }
    else
    {
	return producer::result::busy;
    }
}

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_producer<value_t, allocator_t>::result mpmc_ring_queue<value_t, allocator_t>::try_enqueue_move(value_t&& input)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    // for unsigned integrals nothing extra is needed to handle overflow
    if (head - tail == buffer_.capacity())
    {
	return producer::result::queue_full;
    }
    typename node_type::status guard = buffer_[head % buffer_.capacity()].guard.load(std::memory_order_acquire);
    if (guard == node_type::status::unused)
    {
	if (head_.compare_exchange_strong(head, head + 1, std::memory_order_release))
	{
	    buffer_[head % buffer_.capacity()].value = std::move(input);
	    buffer_[head % buffer_.capacity()].guard.store(node_type::status::used, std::memory_order_release);
	    return producer::result::success;
	}
	else
	{
	    return producer::result::beaten;
	}
    }
    else
    {
	return producer::result::busy;
    }
}

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_consumer<value_t, allocator_t>::result mpmc_ring_queue<value_t, allocator_t>::try_dequeue_copy(value_t& output)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    if (head == tail)
    {
	return consumer::result::queue_empty;
    }
    typename node_type::status guard = buffer_[tail % buffer_.capacity()].guard.load(std::memory_order_acquire);
    if (guard == node_type::status::used)
    {
	if (tail_.compare_exchange_strong(tail, tail + 1, std::memory_order_release))
	{
	    turbo::algorithm::recovery::try_and_ensure(
	    [&] ()
	    {
		output = buffer_[tail % buffer_.capacity()].value;
	    },
	    [&] ()
	    {
		buffer_[tail % buffer_.capacity()].guard.store(node_type::status::unused, std::memory_order_release);
	    });
	    return consumer::result::success;
	}
	else
	{
	    return consumer::result::beaten;
	}
    }
    else
    {
	return consumer::result::busy;
    }
}

template <class value_t, template <class type_t> class allocator_t>
typename mpmc_consumer<value_t, allocator_t>::result mpmc_ring_queue<value_t, allocator_t>::try_dequeue_move(value_t& output)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    if (head == tail)
    {
	return consumer::result::queue_empty;
    }
    typename node_type::status guard = buffer_[tail % buffer_.capacity()].guard.load(std::memory_order_acquire);
    if (guard == node_type::status::used)
    {
	if (tail_.compare_exchange_strong(tail, tail + 1, std::memory_order_release))
	{
	    turbo::algorithm::recovery::try_and_ensure(
	    [&] ()
	    {
		output = std::move(buffer_[tail % buffer_.capacity()].value);
	    },
	    [&] ()
	    {
		buffer_[tail % buffer_.capacity()].guard.store(node_type::status::unused, std::memory_order_release);
	    });
	    return consumer::result::success;
	}
	else
	{
	    return consumer::result::beaten;
	}
    }
    else
    {
	return consumer::result::busy;
    }
}

template <template <class type_t> class allocator_t>
mpmc_ring_queue<std::uint32_t, allocator_t>::mpmc_ring_queue(uint32_t capacity)
    :
	mpmc_ring_queue(capacity, 0U)
{ }

template <template <class type_t> class allocator_t>
mpmc_ring_queue<std::uint32_t, allocator_t>::mpmc_ring_queue(uint32_t capacity, uint16_t handle_limit)
    :
	buffer_(capacity),
	head_(0),
	tail_(0),
	producer_list(handle_limit, key(), *this),
	consumer_list(handle_limit, key(), *this)
{
    // TODO: when a constexpr version of is_lock_free is available do this check as a static_assert
    if (!head_.is_lock_free() || !tail_.is_lock_free())
    {
	throw std::invalid_argument("std::uint32_t is not atomic on this platform");
    }
}

template <template <class type_t> class allocator_t>
template <class handle_t>
mpmc_ring_queue<std::uint32_t, allocator_t>::mpmc_ring_queue::handle_list<handle_t>::handle_list(
	uint16_t limit,
	const key& the_key,
	mpmc_ring_queue<std::uint32_t, allocator_t>& queue)
    :
	counter(0),
	list(limit, handle_t(the_key, queue))
{ }

template <template <class type_t> class allocator_t>
typename mpmc_ring_queue<std::uint32_t, allocator_t>::producer& mpmc_ring_queue<std::uint32_t, allocator_t>::get_producer()
{
    uint16_t count = 0;
    do
    {
	count = producer_list.counter.load(std::memory_order_acquire);
	if (count >= producer_list.list.size())
	{
	    throw std::range_error("No more producers are available");
	}
    }
    while (!producer_list.counter.compare_exchange_strong(count, count + 1, std::memory_order_release));
    return producer_list.list[count];
}

template <template <class type_t> class allocator_t>
typename mpmc_ring_queue<std::uint32_t, allocator_t>::consumer& mpmc_ring_queue<std::uint32_t, allocator_t>::get_consumer()
{
    uint16_t count = 0;
    do
    {
	count = consumer_list.counter.load(std::memory_order_acquire);
	if (count >= consumer_list.list.size())
	{
	    throw std::range_error("No more consumers are available");
	}
    }
    while (!consumer_list.counter.compare_exchange_strong(count, count + 1, std::memory_order_release));
    return consumer_list.list[count];
}

template <template <class type_t> class allocator_t>
typename mpmc_producer<std::uint32_t, allocator_t>::result mpmc_ring_queue<std::uint32_t, allocator_t>::try_enqueue_copy(value_type input)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    // for unsigned integrals nothing extra is needed to handle overflow
    if (head - tail == buffer_.capacity())
    {
	return producer::result::queue_full;
    }
    else if (head_.compare_exchange_strong(head, head + 1, std::memory_order_release))
    {
	buffer_[head % buffer_.capacity()].value.store(input, std::memory_order_release);
	return producer::result::success;
    }
    else
    {
	return producer::result::beaten;
    }
}

template <template <class type_t> class allocator_t>
typename mpmc_producer<std::uint32_t, allocator_t>::result mpmc_ring_queue<std::uint32_t, allocator_t>::try_enqueue_move(value_type&& input)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    // for unsigned integrals nothing extra is needed to handle overflow
    if (head - tail == buffer_.capacity())
    {
	return producer::result::queue_full;
    }
    else if (head_.compare_exchange_strong(head, head + 1, std::memory_order_release))
    {
	buffer_[head % buffer_.capacity()].value.store(input, std::memory_order_release);
	return producer::result::success;
    }
    else
    {
	return producer::result::beaten;
    }
}

template <template <class type_t> class allocator_t>
typename mpmc_consumer<std::uint32_t, allocator_t>::result mpmc_ring_queue<std::uint32_t, allocator_t>::try_dequeue_copy(value_type& output)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    if (head == tail)
    {
	return consumer::result::queue_empty;
    }
    else if (tail_.compare_exchange_strong(tail, tail + 1, std::memory_order_release))
    {
	output = buffer_[tail % buffer_.capacity()].value.load(std::memory_order_acquire);
	return consumer::result::success;
    }
    else
    {
	return consumer::result::beaten;
    }
}

template <template <class type_t> class allocator_t>
typename mpmc_consumer<std::uint32_t, allocator_t>::result mpmc_ring_queue<std::uint32_t, allocator_t>::try_dequeue_move(value_type& output)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    if (head == tail)
    {
	return consumer::result::queue_empty;
    }
    else if (tail_.compare_exchange_strong(tail, tail + 1, std::memory_order_release))
    {
	output = std::move(buffer_[tail % buffer_.capacity()].value.load(std::memory_order_acquire));
	return consumer::result::success;
    }
    else
    {
	return consumer::result::beaten;
    }
}

} // namespace container
} // namespace turbo

#endif
