#ifndef TURBO_CONTAINER_SPSC_RING_QUEUE_HPP
#define TURBO_CONTAINER_SPSC_RING_QUEUE_HPP

#include <cstdint>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace container {

template <class value_t, class allocator_t = std::allocator<value_t>> class spsc_key;

template <class value_t, class allocator_t = std::allocator<value_t>>
class TURBO_SYMBOL_DECL spsc_producer
{
public:
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    typedef spsc_key<value_t, allocator_t> key;
    enum class result
    {
	success,
	failure,
	queue_full
    };
    spsc_producer(const key&,
		    std::vector<value_t, allocator_t>& buffer,
		    std::atomic<uint32_t>& head,
		    std::atomic<uint32_t>& tail);
    result try_enqueue_copy(const value_t& input);
    result try_enqueue_move(value_t&& input);
private:
    std::vector<value_t, allocator_t>& buffer_;
    std::atomic<uint32_t>& head_;
    std::atomic<uint32_t>& tail_;
};

template <class value_t, class allocator_t = std::allocator<value_t>>
class TURBO_SYMBOL_DECL spsc_consumer
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
	failure,
	queue_empty
    };
    result try_dequeue_copy(value_t& output);
    result try_dequeue_move(value_t& output);
private:
    std::vector<value_t, allocator_t>& buffer_;
    std::atomic<uint32_t>& head_;
    std::atomic<uint32_t>& tail_;
};

template <class value_t, class allocator_t = std::allocator<value_t>>
class TURBO_SYMBOL_DECL spsc_ring_queue
{
public:
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    typedef spsc_producer<value_t, allocator_t> producer;
    typedef spsc_consumer<value_t, allocator_t> consumer;
    spsc_ring_queue(uint32_t capacity);
    producer& get_producer();
    consumer& get_consumer();
private:
    typedef std::vector<value_t, allocator_t> vector_type;
    struct single_lock
    {
	single_lock();
	std::mutex mutex;
	std::unique_lock<std::mutex> lock;
    };
    alignas(LEVEL1_DCACHE_LINESIZE) std::vector<value_t, allocator_t> buffer_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t> head_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t> tail_;
    alignas(LEVEL1_DCACHE_LINESIZE) producer producer_;
    alignas(LEVEL1_DCACHE_LINESIZE) single_lock producer_lock_;
    alignas(LEVEL1_DCACHE_LINESIZE) consumer consumer_;
    alignas(LEVEL1_DCACHE_LINESIZE) single_lock consumer_lock_;
};

} // namespace container
} // namespace turbo

#endif
