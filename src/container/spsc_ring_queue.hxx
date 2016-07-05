#ifndef TURBO_CONTAINER_SPSC_RING_QUEUE_HXX
#define TURBO_CONTAINER_SPSC_RING_QUEUE_HXX

#include <turbo/container/spsc_ring_queue.hpp>
#include <limits>
#include <stdexcept>

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
    buffer_[head % buffer_.capacity()] = input;
    if (head_.compare_exchange_strong(head, head + 1, std::memory_order_release))
    {
	return result::success;
    }
    else
    {
	return result::failure;
    }
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
    buffer_[head % buffer_.capacity()] = std::move(input);
    if (head_.compare_exchange_strong(head, head + 1, std::memory_order_release))
    {
	return result::success;
    }
    else
    {
	return result::failure;
    }
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
    output = buffer_[tail % buffer_.capacity()];
    if (tail_.compare_exchange_strong(tail, tail + 1, std::memory_order_release))
    {
	return result::success;
    }
    else
    {
	return result::failure;
    }
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
    output = std::move(buffer_[tail % buffer_.capacity()]);
    if (tail_.compare_exchange_strong(tail, tail + 1, std::memory_order_release))
    {
	return result::success;
    }
    else
    {
	return result::failure;
    }
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
{
    // TODO: when a constexpr version of is_lock_free is available do this check as a static_assert
    if (!head_.is_lock_free() || !tail_.is_lock_free())
    {
	throw std::invalid_argument("uin32_t is not atomic on this platform");
    }
}

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
