#ifndef TURBO_CONTAINER_MPMC_RING_QUEUE_HPP
#define TURBO_CONTAINER_MPMC_RING_QUEUE_HPP

#include <cstdint>
#include <atomic>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace container {

template <class value_t>
struct alignas(LEVEL1_DCACHE_LINESIZE) node
{
    enum class status : uint8_t
    {
	used,
	unused
    };
    std::atomic<status> guard;
    value_t value;
};

template <class value_t>
struct alignas(LEVEL1_DCACHE_LINESIZE) atomic_node
{
    std::atomic<value_t> value;
};

template <class value_t, template <class type_t> class allocator_t = std::allocator> class mpmc_key;
template <class value_t, template <class type_t> class allocator_t = std::allocator> class mpmc_ring_queue;

template <class value_t, template <class type_t> class allocator_t = std::allocator>
class alignas(LEVEL1_DCACHE_LINESIZE) mpmc_producer
{
public:
    typedef value_t value_type;
    typedef mpmc_key<value_t, allocator_t> key;
    enum class result
    {
	success,
	beaten,
	busy,
	queue_full
    };
    mpmc_producer(const key&, mpmc_ring_queue<value_t, allocator_t>& queue);
    mpmc_producer(const mpmc_producer& other);
    result try_enqueue_copy(const value_t& input);
    result try_enqueue_move(value_t&& input);
private:
    mpmc_producer() = delete;
    mpmc_producer& operator=(const mpmc_producer& other) = delete;
    mpmc_ring_queue<value_t, allocator_t>& queue_;
};

template <class value_t, template <class type_t> class allocator_t = std::allocator>
class alignas(LEVEL1_DCACHE_LINESIZE) mpmc_consumer
{
public:
    typedef value_t value_type;
    typedef mpmc_key<value_t, allocator_t> key;
    enum class result
    {
	success,
	beaten,
	busy,
	queue_empty
    };
    mpmc_consumer(const key&, mpmc_ring_queue<value_t, allocator_t>& queue);
    mpmc_consumer(const mpmc_consumer& other);
    result try_dequeue_copy(value_t& output);
    result try_dequeue_move(value_t& output);
private:
    mpmc_consumer() = delete;
    mpmc_consumer& operator=(const mpmc_consumer& other) = delete;
    mpmc_ring_queue<value_t, allocator_t>& queue_;
};

template <class value_t, template <class type_t> class allocator_t>
class TURBO_SYMBOL_DECL mpmc_ring_queue
{
public:
    typedef value_t value_type;
    typedef node<value_t> node_type;
    typedef mpmc_producer<value_t, allocator_t> producer;
    typedef mpmc_consumer<value_t, allocator_t> consumer;
    typedef mpmc_key<value_t, allocator_t> key;
    mpmc_ring_queue(uint32_t capacity);
    mpmc_ring_queue(uint32_t capacity, uint16_t handle_limit);
    producer& get_producer();
    consumer& get_consumer();
    typename producer::result try_enqueue_copy(const value_t& input);
    typename producer::result try_enqueue_move(value_t&& input);
    typename consumer::result try_dequeue_copy(value_t& output);
    typename consumer::result try_dequeue_move(value_t& output);
private:
    typedef std::vector<value_t, allocator_t<value_t>> vector_type;
    template <class handle_t>
    struct handle_list
    {
	handle_list(uint16_t limit, const key& the_key, mpmc_ring_queue<value_t, allocator_t>& queue);
	std::atomic<uint16_t> counter;
	std::vector<handle_t, allocator_t<handle_t>> list;
    };
    alignas(LEVEL1_DCACHE_LINESIZE) std::vector<node_type, allocator_t<node_type>> buffer_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t> head_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t> tail_;
    alignas(LEVEL1_DCACHE_LINESIZE) handle_list<mpmc_producer<value_t, allocator_t>> producer_list;
    alignas(LEVEL1_DCACHE_LINESIZE) handle_list<mpmc_consumer<value_t, allocator_t>> consumer_list;
};

template <template <class type_t> class allocator_t>
class TURBO_SYMBOL_DECL mpmc_ring_queue<std::uint32_t, allocator_t>
{
public:
    typedef std::uint32_t value_type;
    typedef atomic_node<std::uint32_t> node_type;
    typedef mpmc_producer<std::uint32_t, allocator_t> producer;
    typedef mpmc_consumer<std::uint32_t, allocator_t> consumer;
    typedef mpmc_key<std::uint32_t, allocator_t> key;
    mpmc_ring_queue(uint32_t capacity);
    mpmc_ring_queue(uint32_t capacity, uint16_t handle_limit);
    producer& get_producer();
    consumer& get_consumer();
    typename producer::result try_enqueue_copy(value_type input);
    typename producer::result try_enqueue_move(value_type&& input);
    typename consumer::result try_dequeue_copy(value_type& output);
    typename consumer::result try_dequeue_move(value_type& output);
private:
    typedef std::vector<std::uint32_t, allocator_t<std::uint32_t>> vector_type;
    template <class handle_t>
    struct handle_list
    {
	handle_list(uint16_t limit, const key& the_key, mpmc_ring_queue<std::uint32_t, allocator_t>& queue);
	std::atomic<uint16_t> counter;
	std::vector<handle_t, allocator_t<handle_t>> list;
    };
    alignas(LEVEL1_DCACHE_LINESIZE) std::vector<node_type, allocator_t<node_type>> buffer_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t> head_;
    alignas(LEVEL1_DCACHE_LINESIZE) std::atomic<uint32_t> tail_;
    alignas(LEVEL1_DCACHE_LINESIZE) handle_list<mpmc_producer<std::uint32_t, allocator_t>> producer_list;
    alignas(LEVEL1_DCACHE_LINESIZE) handle_list<mpmc_consumer<std::uint32_t, allocator_t>> consumer_list;
};

} // namespace container
} // namespace turbo

#endif
