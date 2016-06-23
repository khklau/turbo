#ifndef TURBO_CONTAINER_MPMC_RING_QUEUE_HXX
#define TURBO_CONTAINER_MPMC_RING_QUEUE_HXX

#include <turbo/container/mpmc_ring_queue.hpp>
#include <limits>

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
mpmc_ring_queue<value_t, allocator_t>::mpmc_ring_queue(uint32_t capacity) :
	buffer_(capacity),
	head_(0),
	tail_(0),
	producer_(typename producer::key(), buffer_, head_, tail_),
	consumer_(typename consumer::key(), buffer_, head_, tail_)
{ }

} // namespace container
} // namespace turbo

#endif
