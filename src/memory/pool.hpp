#ifndef TURBO_MEMORY_POOL_HPP
#define TURBO_MEMORY_POOL_HPP

#include <cstdint>
#include <atomic>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <turbo/container/mpmc_ring_queue.hpp>
#include <turbo/memory/block.hpp>

namespace turbo {
namespace memory {

template <class value_t>
using pool_unique_ptr = std::unique_ptr<value_t, std::function<void (value_t*)>>;

typedef std::uint32_t capacity_type;

enum class make_result
{
    success,
    pool_full
};

template <std::size_t block_size_c, template <class type_t> class allocator_t = std::allocator>
class block_pool
{
public:
    block_pool(capacity_type capacity);
    std::size_t get_byte_size() const;
    const void* get_base_address() const;
    std::pair<make_result, void*> allocate();
    void free(void* pointer);
    template <class value_t, class... args_t>
    std::pair<make_result, pool_unique_ptr<value_t>> make_unique(args_t&&... args);
    template <class value_t, class... args_t>
    std::pair<make_result, std::shared_ptr<value_t>> make_shared(args_t&&... args);
private:
    typedef typename std::aligned_storage<block_size_c>::type block_type;
    typedef turbo::container::mpmc_ring_queue<capacity_type, allocator_t<capacity_type>> free_list_type;
    free_list_type free_list_;
    typename std::vector<block_type, allocator_t<block_type>> block_list_;
};

struct block_config
{
    block_config();
    block_config(std::size_t size, capacity_type capacity);
    bool operator<(const block_config& other) const;
    bool operator==(const block_config& other) const;
    std::size_t block_size;
    capacity_type initial_capacity;
};

template <template <class type_t> class allocator_t = std::allocator>
class range_pool
{
public:
    range_pool(capacity_type default_capacity, const std::vector<block_config>& config);
    range_pool(capacity_type default_capacity, const std::vector<block_config>& config, std::uint8_t step_factor);
    static std::vector<block_config> sanitise(const std::vector<block_config>& config, std::uint8_t step_factor);
private:
    capacity_type default_capacity_;
    std::size_t step_factor_;
    std::size_t smallest_block_;
};

class block_list
{
private:
    class node;
public:
    enum class append_result
    {
	success,
	beaten
    };
    class invalid_dereference : public std::out_of_range
    {
    public:
	explicit invalid_dereference(const std::string& what);
	explicit invalid_dereference(const char* what);
    };
    class iterator : public std::forward_iterator_tag
    {
    public:
	iterator();
	iterator(node* pointer);
	iterator(const iterator& other);
	iterator& operator=(const iterator& other);
	~iterator() = default;
	bool operator==(const iterator& other) const;
	inline bool operator!=(const iterator& other) const { return !(*this == other); }
	block& operator*();
	iterator& operator++();
	iterator operator++(int);
    private:
	node* pointer_;
    };
    block_list(std::size_t value_size, block::capacity_type capacity);
    inline iterator begin() noexcept { return iterator(&front_); }
    inline iterator end() noexcept { return iterator(); }
    node* create_node(std::size_t value_size, block::capacity_type capacity);
    append_result try_append(iterator& current, const node* successor);
private:
    class node
    {
    public:
	node(std::size_t value_size, block::capacity_type capacity);
	inline block& get_block() { return block_; }
	inline std::atomic<node*>& get_next() { return next_; }
    private:
	node() = delete;
	node(const node&) = delete;
	node(node&&) = delete;
	node& operator=(const node&) = delete;
	node& operator=(node&&) = delete;
	block block_;
	std::atomic<node*> next_;
    };
    block_list() = delete;
    block_list(const block_list&) = delete;
    block_list(block_list&&) = delete;
    block_list& operator=(const block_list&) = delete;
    block_list& operator=(block_list&&) = delete;
    node front_;
};

class pool
{
public:
    pool(block::capacity_type default_capacity, const std::vector<block_config>& config);
    pool(block::capacity_type default_capacity, const std::vector<block_config>& config, std::uint8_t step_factor);
private:
    block::capacity_type default_capacity_;
    std::size_t step_factor_;
    std::size_t smallest_block_;
};

std::vector<block_config> calibrate(const std::vector<block_config>& config, std::uint8_t step_factor);

} // namespace memory
} // namespace turbo

#endif
