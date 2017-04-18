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
    inline node() noexcept;
    inline explicit node(const value_t& the_value);
    inline node(const node& other);
    node(node&&) = delete;
    ~node() = default;
    node& operator=(const node& other);
    node& operator=(node&&) = delete;
    inline bool operator==(const node& other) const;
    std::atomic<status> guard;
    value_t value;
};

template <class value_t>
struct alignas(LEVEL1_DCACHE_LINESIZE) atomic_node
{
    atomic_node() = default;
    inline explicit atomic_node(const value_t& the_value);
    inline atomic_node(const atomic_node& other);
    atomic_node(atomic_node&&) = delete;
    ~atomic_node() = default;
    atomic_node& operator=(const atomic_node& other);
    atomic_node& operator=(atomic_node&&) = delete;
    inline bool operator==(const atomic_node& other) const;
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
    ~mpmc_producer() = default;
    bool operator==(const mpmc_producer& other) const;
    result try_enqueue_copy(const value_t& input);
    result try_enqueue_move(value_t&& input);
private:
    mpmc_producer() = delete;
    mpmc_producer(mpmc_producer&&);
    mpmc_producer& operator=(const mpmc_producer&) = delete;
    mpmc_producer& operator=(mpmc_producer&&) = delete;
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
    ~mpmc_consumer() = default;
    bool operator==(const mpmc_consumer& other) const;
    result try_dequeue_copy(value_t& output);
    result try_dequeue_move(value_t& output);
private:
    mpmc_consumer() = delete;
    mpmc_consumer(mpmc_consumer&&) = delete;
    mpmc_consumer& operator=(const mpmc_consumer&) = delete;
    mpmc_consumer& operator=(mpmc_consumer&&) = delete;
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
    mpmc_ring_queue(const mpmc_ring_queue& other);
    ~mpmc_ring_queue() = default;
    mpmc_ring_queue& operator=(const mpmc_ring_queue& other);
    bool operator==(const mpmc_ring_queue& other) const;
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
	handle_list(const handle_list& other, mpmc_ring_queue<value_t, allocator_t>* queue = nullptr);
	handle_list(handle_list&& other) = delete;
	~handle_list() = default;
	bool operator==(const handle_list& other) const;
	handle_list& operator=(const handle_list&) = delete;
	handle_list& operator=(handle_list&&) = delete;
	std::atomic<uint16_t> counter;
	std::vector<handle_t, allocator_t<handle_t>> list;
    };
    mpmc_ring_queue() = delete;
    mpmc_ring_queue(mpmc_ring_queue&&) = delete;
    mpmc_ring_queue& operator=(mpmc_ring_queue&&) = delete;
    std::vector<node_type, allocator_t<node_type>> buffer_;
    std::atomic<uint32_t> head_;
    std::atomic<uint32_t> tail_;
    handle_list<mpmc_producer<value_t, allocator_t>> producer_list_;
    handle_list<mpmc_consumer<value_t, allocator_t>> consumer_list_;
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
    mpmc_ring_queue(const mpmc_ring_queue& other);
    ~mpmc_ring_queue() = default;
    mpmc_ring_queue& operator=(const mpmc_ring_queue& other);
    bool operator==(const mpmc_ring_queue& other) const;
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
	handle_list(const handle_list& other, mpmc_ring_queue<std::uint32_t, allocator_t>* queue = nullptr);
	~handle_list() = default;
	bool operator==(const handle_list& other) const;
	handle_list& operator=(const handle_list&) = delete;
	handle_list& operator=(handle_list&&) = delete;
	std::atomic<uint16_t> counter;
	std::vector<handle_t, allocator_t<handle_t>> list;
    };
    mpmc_ring_queue() = delete;
    mpmc_ring_queue(mpmc_ring_queue&&) = delete;
    mpmc_ring_queue& operator=(mpmc_ring_queue&&) = delete;
    std::vector<node_type, allocator_t<node_type>> buffer_;
    std::atomic<uint32_t> head_;
    std::atomic<uint32_t> tail_;
    handle_list<mpmc_producer<std::uint32_t, allocator_t>> producer_list_;
    handle_list<mpmc_consumer<std::uint32_t, allocator_t>> consumer_list_;
};

template <template <class type_t> class allocator_t>
class TURBO_SYMBOL_DECL mpmc_ring_queue<std::uint64_t, allocator_t>
{
public:
    typedef std::uint64_t value_type;
    typedef atomic_node<std::uint64_t> node_type;
    typedef mpmc_producer<std::uint64_t, allocator_t> producer;
    typedef mpmc_consumer<std::uint64_t, allocator_t> consumer;
    typedef mpmc_key<std::uint64_t, allocator_t> key;
    mpmc_ring_queue(uint32_t capacity);
    mpmc_ring_queue(uint32_t capacity, uint16_t handle_limit);
    mpmc_ring_queue(const mpmc_ring_queue& other);
    ~mpmc_ring_queue() = default;
    mpmc_ring_queue& operator=(const mpmc_ring_queue& other);
    bool operator==(const mpmc_ring_queue& other) const;
    producer& get_producer();
    consumer& get_consumer();
    typename producer::result try_enqueue_copy(value_type input);
    typename producer::result try_enqueue_move(value_type&& input);
    typename consumer::result try_dequeue_copy(value_type& output);
    typename consumer::result try_dequeue_move(value_type& output);
private:
    typedef std::vector<std::uint64_t, allocator_t<std::uint64_t>> vector_type;
    template <class handle_t>
    struct handle_list
    {
	handle_list(uint16_t limit, const key& the_key, mpmc_ring_queue<std::uint64_t, allocator_t>& queue);
	handle_list(const handle_list& other, mpmc_ring_queue<std::uint64_t, allocator_t>* queue = nullptr);
	~handle_list() = default;
	bool operator==(const handle_list& other) const;
	handle_list& operator=(const handle_list&) = delete;
	handle_list& operator=(handle_list&&) = delete;
	std::atomic<uint16_t> counter;
	std::vector<handle_t, allocator_t<handle_t>> list;
    };
    mpmc_ring_queue() = delete;
    mpmc_ring_queue(mpmc_ring_queue&&) = delete;
    mpmc_ring_queue& operator=(mpmc_ring_queue&&) = delete;
    std::vector<node_type, allocator_t<node_type>> buffer_;
    std::atomic<uint32_t> head_;
    std::atomic<uint32_t> tail_;
    handle_list<mpmc_producer<std::uint64_t, allocator_t>> producer_list_;
    handle_list<mpmc_consumer<std::uint64_t, allocator_t>> consumer_list_;
};

} // namespace container
} // namespace turbo

#endif
