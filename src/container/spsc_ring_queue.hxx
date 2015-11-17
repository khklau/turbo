#ifndef TURBO_CONTAINER_SPSC_RING_QUEUE_HXX
#define TURBO_CONTAINER_SPSC_RING_QUEUE_HXX

#include <turbo/container/spsc_ring_queue.hpp>
#include <limits>

namespace turbo {
namespace container {

template <class value_t, class allocator_t>
class spsc_key
{
    spsc_key() { }
    friend class spsc_ring_queue<value_t, allocator_t>;
};

template <class value_t, class allocator_t>
spsc_producer<value_t, allocator_t>::spsc_producer(
		const key&,
		std::vector<value_t, allocator_t>& buffer,
		std::atomic<uint32_t>& head,
		std::atomic<uint32_t>& tail) :
	buffer_(buffer),
	head_(head),
	tail_(tail)
{ }

template <class value_t, class allocator_t>
typename spsc_producer<value_t, allocator_t>::result spsc_producer<value_t, allocator_t>::try_enqueue(const value_t& input)
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
spsc_consumer<value_t, allocator_t>::spsc_consumer(
		const key&,
		std::vector<value_t, allocator_t>& buffer,
		std::atomic<uint32_t>& head,
		std::atomic<uint32_t>& tail) :
	buffer_(buffer),
	head_(head),
	tail_(tail)
{ }

template <class value_t, class allocator_t>
typename spsc_consumer<value_t, allocator_t>::result spsc_consumer<value_t, allocator_t>::try_dequeue(value_t& output)
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
spsc_ring_queue<value_t, allocator_t>::spsc_ring_queue(uint32_t capacity) :
	buffer_(capacity),
	head_(0),
	tail_(0),
	producer_(typename producer::key(), buffer_, head_, tail_),
	consumer_(typename consumer::key(), buffer_, head_, tail_)
{ }

} // namespace container
} // namespace turbo

#endif
