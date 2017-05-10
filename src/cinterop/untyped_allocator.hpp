#ifndef TURBO_CINTEROP_UNTYPED_ALLOCATOR_HPP
#define TURBO_CINTEROP_UNTYPED_ALLOCATOR_HPP

#include <cstdint>
#include <turbo/container/bitwise_trie.hpp>
#include <turbo/memory/pool.hpp>
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
    untyped_allocator(std::uint32_t default_capacity, const std::vector<turbo::memory::block_config>& config);
    ~untyped_allocator();
    void* malloc(std::size_t size);
    void free(void* ptr);
    friend class untyped_allocator_tester;
private:
    typedef turbo::container::bitwise_trie<std::uintptr_t, std::size_t, turbo::memory::pool> trie_type;
    static std::vector<turbo::memory::block_config> derive_trie_config(const std::vector<turbo::memory::block_config>& alloc_config);
    turbo::memory::pool allocation_pool_;
    turbo::memory::pool trie_pool_;
    trie_type address_map_;
};

} // namespace cinterop

} // namespace turbo

#endif
