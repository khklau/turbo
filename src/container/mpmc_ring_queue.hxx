#ifndef TURBO_CONTAINER_MPMC_RING_QUEUE_HXX
#define TURBO_CONTAINER_MPMC_RING_QUEUE_HXX

#include <turbo/container/mpmc_ring_queue.hpp>
#include <limits>
#include <stdexcept>

namespace turbo {
namespace container {

template <class value_t, class allocator_t>
class mpmc_key
{
    mpmc_key() { }
    friend class mpmc_ring_queue<value_t, allocator_t>;
};

template <class value_t, class allocator_t>
mpmc_producer<value_t, allocator_t>::mpmc_producer(
	const key&,
	std::vector<value_t, allocator_t>& buffer,
	std::atomic<uint32_t>& head,
	std::atomic<uint32_t>& tail)
    :
	buffer_(buffer),
	head_(head),
	tail_(tail)
{ }

template <class value_t, class allocator_t>
mpmc_producer<value_t, allocator_t>::mpmc_producer(const mpmc_producer& other)
    :
	buffer_(other.buffer_),
	head_(other.head_),
	tail_(other.tail_)
{ }

template <class value_t, class allocator_t>
typename mpmc_producer<value_t, allocator_t>::result mpmc_producer<value_t, allocator_t>::try_enqueue_copy(const value_t& input)
{
    uint32_t head = head_.load(std::memory_order_consume);
    uint32_t tail = tail_.load(std::memory_order_consume);
    // for unsigned integrals nothing extra is needed to handle overflow
    if (head - tail == buffer_.capacity())
    {
	return result::queue_full;
    }
    buffer_[head % buffer_.capacity()] = input;
    head_.fetch_add(1, std::memory_order_seq_cst);
    return result::success;
}

template <class value_t, class allocator_t>
typename mpmc_producer<value_t, allocator_t>::result mpmc_producer<value_t, allocator_t>::try_enqueue_move(value_t&& input)
{
    uint32_t head = head_.load(std::memory_order_consume);
    uint32_t tail = tail_.load(std::memory_order_consume);
    // for unsigned integrals nothing extra is needed to handle overflow
    if (head - tail == buffer_.capacity())
    {
	return result::queue_full;
    }
    buffer_[head % buffer_.capacity()] = std::move(input);
    head_.fetch_add(1, std::memory_order_seq_cst);
    return result::success;
}

template <class value_t, class allocator_t>
mpmc_consumer<value_t, allocator_t>::mpmc_consumer(
	const key&,
	std::vector<value_t, allocator_t>& buffer,
	std::atomic<uint32_t>& head,
	std::atomic<uint32_t>& tail)
    :
	buffer_(buffer),
	head_(head),
	tail_(tail)
{ }

template <class value_t, class allocator_t>
mpmc_consumer<value_t, allocator_t>::mpmc_consumer(const mpmc_consumer& other)
    :
	buffer_(other.buffer_),
	head_(other.head_),
	tail_(other.tail_)
{ }

template <class value_t, class allocator_t>
typename mpmc_consumer<value_t, allocator_t>::result mpmc_consumer<value_t, allocator_t>::try_dequeue_copy(value_t& output)
{
    uint32_t head = head_.load(std::memory_order_consume);
    uint32_t tail = tail_.load(std::memory_order_consume);
    if (head == tail)
    {
	return result::queue_empty;
    }
    output = buffer_[tail % buffer_.capacity()];
    tail_.fetch_add(1, std::memory_order_seq_cst);
    return result::success;
}

template <class value_t, class allocator_t>
typename mpmc_consumer<value_t, allocator_t>::result mpmc_consumer<value_t, allocator_t>::try_dequeue_move(value_t& output)
{
    uint32_t head = head_.load(std::memory_order_consume);
    uint32_t tail = tail_.load(std::memory_order_consume);
    if (head == tail)
    {
	return result::queue_empty;
    }
    output = std::move(buffer_[tail % buffer_.capacity()]);
    tail_.fetch_add(1, std::memory_order_seq_cst);
    return result::success;
}

template <class value_t, class allocator_t>
mpmc_ring_queue<value_t, allocator_t>::mpmc_ring_queue(uint32_t capacity, uint16_t handle_limit)
    :
	buffer_(capacity),
	head_(0),
	tail_(0),
	producer_list(handle_limit, key(), buffer_, head_, tail_),
	consumer_list(handle_limit, key(), buffer_, head_, tail_)
{ }

template <class value_t, class allocator_t>
template <class handle_t>
mpmc_ring_queue<value_t, allocator_t>::mpmc_ring_queue::handle_list<handle_t>::handle_list(
	uint16_t limit,
	const key& the_key,
	std::vector<value_t, allocator_t>& buffer,
	std::atomic<uint32_t>& head,
	std::atomic<uint32_t>& tail)
    :
	counter(0),
	list(limit, handle_t(the_key, buffer, head, tail))
{ }

template <class value_t, class allocator_t>
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

template <class value_t, class allocator_t>
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

} // namespace container
} // namespace turbo

#endif
