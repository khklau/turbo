#ifndef TURBO_CINTEROP_UNTYPED_ALLOCATOR_HPP
#define TURBO_CINTEROP_UNTYPED_ALLOCATOR_HPP

#include <cstdint>
#include <turbo/container/bitwise_trie.hpp>
#include <turbo/memory/slab_allocator.hpp>
#include <turbo/toolset/attribute.hpp>

namespace turbo {

namespace memory {

class block;

} // namespace memory

namespace cinterop {

class untyped_allocator_tester;

class TURBO_SYMBOL_DECL untyped_allocator final
{
public:
    static const std::size_t growth_contingency = 2U;
    untyped_allocator(std::uint32_t contingency_capacity, const std::vector<turbo::memory::block_config>& config);
    untyped_allocator(const untyped_allocator& other);
    ~untyped_allocator();
    untyped_allocator& operator=(const untyped_allocator& other);
    inline std::size_t get_block_count()
    {
	return address_map_.size();
    }
    void* malloc(std::size_t size);
    void free(void* ptr);
    friend class untyped_allocator_tester;
private:
    typedef turbo::container::bitwise_trie<std::uintptr_t, std::size_t, turbo::memory::concurrent_sized_slab> trie_type;
    untyped_allocator() = delete;
    untyped_allocator(untyped_allocator&&) = delete;
    untyped_allocator& operator=(untyped_allocator&&) = delete;
    static std::vector<turbo::memory::block_config> derive_trie_config(const std::vector<turbo::memory::block_config>& alloc_config);
    void init_address_map();
    turbo::memory::concurrent_sized_slab allocation_slab_;
    turbo::memory::concurrent_sized_slab trie_slab_;
    trie_type address_map_;
};

} // namespace cinterop

} // namespace turbo

#endif
