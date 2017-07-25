#ifndef TURBO_MEMORY_SLAB_ALLOCATOR_HPP
#define TURBO_MEMORY_SLAB_ALLOCATOR_HPP

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
using slab_unique_ptr = std::unique_ptr<value_t, std::function<void (value_t*)>>;

enum class make_result
{
    success,
    slab_full
};

class TURBO_SYMBOL_DECL concurrent_sized_slab_tester;

class TURBO_SYMBOL_DECL concurrent_sized_slab
{
private:
    typedef std::vector<block_list> block_map_type;
public:
    typedef block_map_type::iterator iterator;
    concurrent_sized_slab(block::capacity_type contingency_capacity, const std::vector<block_config>& config);
    concurrent_sized_slab(const concurrent_sized_slab& other);
    ~concurrent_sized_slab() = default;
    concurrent_sized_slab& operator=(const concurrent_sized_slab& other);
    bool operator==(const concurrent_sized_slab& other) const;
    const std::vector<block_config> get_block_config() const;
    inline iterator begin()
    {
	return block_map_.begin();
    }
    inline iterator end()
    {
	return block_map_.end();
    }
    template <class value_t, class... args_t>
    std::pair<make_result, slab_unique_ptr<value_t>> make_unique(args_t&&... args);
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
    inline void* malloc(std::size_t size)
    {
	return allocate(size, size, 1U, nullptr);
    }
    inline void free(void* ptr, std::size_t size)
    {
	return deallocate(size, size, ptr, 1U);
    }
    inline bool in_configured_range(std::size_t value_size) const;
    inline const block_list& at(std::size_t size) const;
    inline block_list& at(std::size_t size);
    friend class concurrent_sized_slab_tester;
private:
    concurrent_sized_slab() = delete;
    concurrent_sized_slab(const std::vector<block_config>& config);
    concurrent_sized_slab(concurrent_sized_slab&&) = delete;
    concurrent_sized_slab& operator=(concurrent_sized_slab&&) = delete;
    inline std::size_t find_block_bucket(std::size_t allocation_size) const;
    void* allocate(std::size_t value_size, std::size_t value_alignment, capacity_type quantity, const void* hint);
    void deallocate(std::size_t value_size, std::size_t value_alignment, void* pointer, capacity_type quantity);
    template <class value_t>
    inline void unmake(value_t* pointer);
    std::size_t smallest_block_exponent_;
    block_map_type block_map_;
};

std::vector<block_config> calibrate(block::capacity_type contingency_capacity, const std::vector<block_config>& config);

} // namespace memory
} // namespace turbo

#endif
