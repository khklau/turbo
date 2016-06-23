#ifndef TURBO_CONTAINER_MPMC_RING_QUEUE_HPP
#define TURBO_CONTAINER_MPMC_RING_QUEUE_HPP

#include <cstdint>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace container {

template <class value_t, class allocator_t = std::allocator<value_t>> class mpmc_key;

template <class value_t, class allocator_t = std::allocator<value_t>>
class mpmc_producer
{
public:
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    typedef mpmc_key<value_t, allocator_t> key;
    enum class result
    {
	success,
	queue_full
    };
    mpmc_producer(const key&,
		    std::vector<value_t, allocator_t>& buffer,
		    std::atomic<uint32_t>& head,
		    std::atomic<uint32_t>& tail);
    result try_enqueue_copy(const value_t& input);
    result try_enqueue_move(value_t&& input);
private:
    alignas(LEVEL1_DCACHE_LINESIZE) std::vector<value_t, allocator_t>& buffer_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t>& head_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t>& tail_;
};

template <class value_t, class allocator_t = std::allocator<value_t>>
class mpmc_consumer
{
public:
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    typedef mpmc_key<value_t, allocator_t> key;
    mpmc_consumer(const key&,
		    std::vector<value_t, allocator_t>& buffer,
		    std::atomic<uint32_t>& head,
		    std::atomic<uint32_t>& tail);
    enum class result
    {
	success,
	queue_empty
    };
    result try_dequeue_copy(value_t& output);
    result try_dequeue_move(value_t& output);
private:
    alignas(LEVEL1_DCACHE_LINESIZE) std::vector<value_t, allocator_t>& buffer_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t>& head_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t>& tail_;
};

template <class value_t, class allocator_t = std::allocator<value_t>>
class mpmc_ring_queue
{
public:
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    typedef mpmc_producer<value_t, allocator_t> producer;
    typedef mpmc_consumer<value_t, allocator_t> consumer;
    mpmc_ring_queue(uint32_t capacity);
    producer& get_producer() { return producer_; }
    consumer& get_consumer() { return consumer_; }
private:
    typedef std::vector<value_t, allocator_t> vector_type;
    alignas(LEVEL1_DCACHE_LINESIZE) std::vector<value_t, allocator_t> buffer_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t> head_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t> tail_;
    alignas(LEVEL1_DCACHE_LINESIZE) producer producer_;
    alignas(LEVEL1_DCACHE_LINESIZE) consumer consumer_;
};

} // namespace container
} // namespace turbo

#endif
