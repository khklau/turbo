#ifndef TURBO_MEMORY_POOL_HPP
#define TURBO_MEMORY_POOL_HPP

#include <cstdint>
#include <cstdlib>
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
#include <turbo/toolset/attribute.hpp>

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

struct block_config
{
    block_config();
    block_config(std::size_t size, capacity_type capacity);
    block_config(std::size_t size, capacity_type capacity, std::size_t growth_factor);
    bool operator<(const block_config& other) const;
    bool operator==(const block_config& other) const;
    std::size_t block_size;
    capacity_type initial_capacity;
    std::size_t growth_factor;
};

class TURBO_SYMBOL_DECL block_list
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
	explicit inline invalid_dereference(const std::string& what) : out_of_range(what) { }
	explicit inline invalid_dereference(const char* what) : out_of_range(what) { }
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
	block* operator->();
	iterator& operator++();
	iterator operator++(int);
	inline bool is_valid() const { return pointer_ != nullptr; }
	inline bool is_last() const { return is_valid() && pointer_->is_last(); }
	append_result try_append(std::unique_ptr<node> successor);
    private:
	node* pointer_;
    };
    block_list(std::size_t value_size, block::capacity_type capacity);
    block_list(std::size_t value_size, block::capacity_type capacity, std::size_t growth_factor);
    block_list(const block_config& config); // allow implicit conversion
    inline std::size_t get_value_size() const { return value_size_; }
    inline std::size_t get_growth_factor() const { return growth_factor_; }
    inline iterator begin() noexcept { return iterator(&first_); }
    inline iterator end() noexcept { return iterator(); }
    std::unique_ptr<node> create_node(block::capacity_type capacity);
private:
    class node
    {
    public:
	node(std::size_t value_size, block::capacity_type capacity);
	~node() noexcept;
	inline block& get_block() { return block_; }
	inline std::atomic<node*>& get_next() { return next_; }
	inline bool is_last() const { return next_.load(std::memory_order_acquire) == nullptr; }
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
    std::size_t value_size_;
    std::size_t growth_factor_;
    node first_;
};

class TURBO_SYMBOL_DECL pool
{
public:
    pool(block::capacity_type default_capacity, const std::vector<block_config>& config);
    inline std::size_t find_block_bucket(std::size_t allocation_size) const;
    template <class value_t, class... args_t>
    std::pair<make_result, pool_unique_ptr<value_t>> make_unique(args_t&&... args);
    template <class value_t, class... args_t>
    std::pair<make_result, std::shared_ptr<value_t>> make_shared(args_t&&... args);
    template <class value_t>
    inline value_t* allocate(capacity_type quantity, const value_t* hint)
    {
	return static_cast<value_t*>(allocate(sizeof(value_t), alignof(value_t), quantity, hint));
    }
    template <class value_t>
    inline value_t* allocate()
    {
	return allocate(1U, static_cast<const value_t*>(nullptr));
    }
    template <class value_t>
    inline value_t* allocate(capacity_type quantity)
    {
	return allocate(quantity, static_cast<const value_t*>(nullptr));
    }
    template <class value_t>
    inline value_t* allocate(const value_t* hint)
    {
	return allocate(1U, hint);
    }
    template <class value_t>
    inline void deallocate(value_t* pointer, capacity_type quantity)
    {
	deallocate(sizeof(value_t), alignof(value_t), pointer, quantity);
    }
    template <class value_t>
    inline void deallocate(value_t* pointer)
    {
	deallocate(pointer, 1U);
    }
private:
    pool(const std::vector<block_config>& config, block::capacity_type default_capacity);
    void* allocate(std::size_t value_size, std::size_t value_alignment, capacity_type quantity, const void* hint);
    void deallocate(std::size_t value_size, std::size_t value_alignment, void* pointer, capacity_type quantity);
    template <class value_t>
    inline void unmake(value_t* pointer);
    block::capacity_type default_capacity_;
    std::size_t smallest_block_exponent_;
    std::vector<block_list> block_map_;
};

std::vector<block_config> calibrate(const std::vector<block_config>& config);

} // namespace memory
} // namespace turbo

#endif
