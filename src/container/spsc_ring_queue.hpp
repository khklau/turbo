#ifndef TURBO_CONTAINER_SPSC_RING_QUEUE_HPP
#define TURBO_CONTAINER_SPSC_RING_QUEUE_HPP

#include <cstdint>
#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace container {

template <class value_t, class allocator_t = std::allocator<value_t>> class spsc_key;

template <class value_t, class allocator_t = std::allocator<value_t>>
class spsc_producer
{
public:
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    typedef spsc_key<value_t, allocator_t> key;
    enum class result
    {
	success,
	queue_full
    };
    spsc_producer(const key&,
		    std::vector<value_t, allocator_t>& buffer,
		    std::atomic<uint32_t>& head,
		    std::atomic<uint32_t>& tail);
    result try_enqueue(const value_t& input);
    result try_enqueue(value_t&& input);
private:
    std::vector<value_t, allocator_t>& buffer_;
    std::atomic<uint32_t>& head_;
    std::atomic<uint32_t>& tail_;
};

template <class value_t, class allocator_t = std::allocator<value_t>>
class spsc_consumer
{
public:
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    typedef spsc_key<value_t, allocator_t> key;
    spsc_consumer(const key&,
		    std::vector<value_t, allocator_t>& buffer,
		    std::atomic<uint32_t>& head,
		    std::atomic<uint32_t>& tail);
    enum class result
    {
	success,
	queue_empty
    };
    result try_dequeue(value_t& output);
    result try_dequeue_swap(value_t& output);
private:
    std::vector<value_t, allocator_t>& buffer_;
    std::atomic<uint32_t>& head_;
    std::atomic<uint32_t>& tail_;
};

template <class value_t, class allocator_t = std::allocator<value_t>>
class spsc_ring_queue
{
public:
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    typedef spsc_producer<value_t, allocator_t> producer;
    typedef spsc_consumer<value_t, allocator_t> consumer;
    spsc_ring_queue(uint32_t capacity);
    producer& get_producer() { return producer_; }
    consumer& get_consumer() { return consumer_; }
private:
    typedef std::vector<value_t, allocator_t> vector_type;
    std::vector<value_t, allocator_t> buffer_;
    std::atomic<uint32_t> head_;
    std::atomic<uint32_t> tail_;
    producer producer_;
    consumer consumer_;
};

} // namespace container
} // namespace turbo

#endif
