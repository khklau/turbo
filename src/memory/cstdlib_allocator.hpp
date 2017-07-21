#ifndef TURBO_MEMORY_TYPED_ALLOCATOR_HPP
#define TURBO_MEMORY_TYPED_ALLOCATOR_HPP

#include <cstdlib>
#include <memory>

namespace turbo {
namespace memory {

/***
 * Default typed allocator
 ***/
class typed_allocator
{
public:
    typedef std::size_t size_type;
    template <class value_t>
    inline value_t* allocate() { return std::allocator<value_t>::allocate(1U); }
    template <class value_t>
    inline value_t* allocate(size_type quantity) { return std::allocator<value_t>::allocate(quantity); }
    template <class value_t>
    inline value_t* allocate(const value_t* hint) { return std::allocator<value_t>::allocate(1U, hint); }
    template <class value_t>
    inline value_t* allocate(size_type quantity, const value_t* hint) { return std::allocator<value_t>::allocate(quantity, hint); }
    template <class value_t>
    inline void dellocate(value_t* pointer) { std::allocator<value_t>::deallocate(pointer, 1U); }
    template <class value_t>
    inline void dellocate(value_t* pointer, size_type quantity) { std::allocator<value_t>::deallocate(pointer, quantity); }
};

} // namespace memory
} // namespace turbo

#endif
