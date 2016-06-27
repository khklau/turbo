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
typename spsc_producer<value_t, allocator_t>::result spsc_producer<value_t, allocator_t>::try_enqueue_copy(const value_t& input)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    // for unsigned integrals nothing extra is needed to handle overflow
    if (head - tail == buffer_.capacity())
    {
	return result::queue_full;
    }
    // release fence is sufficient; acquire not required since there are no further read operations
    uint32_t exclusive = head_.fetch_add(1, std::memory_order_release);
    buffer_[exclusive % buffer_.capacity()] = input;
    return result::success;
}

template <class value_t, class allocator_t>
typename spsc_producer<value_t, allocator_t>::result spsc_producer<value_t, allocator_t>::try_enqueue_move(value_t&& input)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    // for unsigned integrals nothing extra is needed to handle overflow
    if (head - tail == buffer_.capacity())
    {
	return result::queue_full;
    }
    // release fence is sufficient; acquire not required since there are no further read operations
    uint32_t exclusive = head_.fetch_add(1, std::memory_order_release);
    buffer_[exclusive % buffer_.capacity()] = std::move(input);
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
typename spsc_consumer<value_t, allocator_t>::result spsc_consumer<value_t, allocator_t>::try_dequeue_copy(value_t& output)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    if (head == tail)
    {
	return result::queue_empty;
    }
    // release fence is sufficient; acquire not required since there are no further read operations
    uint32_t exclusive = tail_.fetch_add(1, std::memory_order_release);
    output = buffer_[exclusive % buffer_.capacity()];
    return result::success;
}

template <class value_t, class allocator_t>
typename spsc_consumer<value_t, allocator_t>::result spsc_consumer<value_t, allocator_t>::try_dequeue_move(value_t& output)
{
    uint32_t head = head_.load(std::memory_order_acquire);
    uint32_t tail = tail_.load(std::memory_order_acquire);
    if (head == tail)
    {
	return result::queue_empty;
    }
    // release fence is sufficient; acquire not required since there are no further read operations
    uint32_t exclusive = tail_.fetch_add(1, std::memory_order_release);
    output = std::move(buffer_[exclusive % buffer_.capacity()]);
    return result::success;
}

template <class value_t, class allocator_t>
spsc_ring_queue<value_t, allocator_t>::single_lock::single_lock()
    :
	mutex(),
	lock(mutex, std::defer_lock)
{ }

template <class value_t, class allocator_t>
spsc_ring_queue<value_t, allocator_t>::spsc_ring_queue(uint32_t capacity) :
	buffer_(capacity),
	head_(0),
	tail_(0),
	producer_(typename producer::key(), buffer_, head_, tail_),
	producer_lock_(),
	consumer_(typename consumer::key(), buffer_, head_, tail_),
	consumer_lock_()
{ }

template <class value_t, class allocator_t>
typename spsc_ring_queue<value_t, allocator_t>::producer& spsc_ring_queue<value_t, allocator_t>::get_producer()
{
    producer_lock_.lock.try_lock();
    return producer_;
}

template <class value_t, class allocator_t>
typename spsc_ring_queue<value_t, allocator_t>::consumer& spsc_ring_queue<value_t, allocator_t>::get_consumer()
{
    consumer_lock_.lock.try_lock();
    return consumer_;
}

} // namespace container
} // namespace turbo

#endif
