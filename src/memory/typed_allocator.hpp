#ifndef TURBO_MEMORY_TYPED_ALLOCATOR_HPP
#define TURBO_MEMORY_TYPED_ALLOCATOR_HPP

#include <cstddef>
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
    inline typename value_t allocate(size_type quantity = 1U, value_t* hint = nullptr) { return std::allocator<value_t>::allocate(quantity, hint); }
    template <class value_t>
    inline void dellocate(value_t* pointer, size_type quantity) { std::allocator<value_t>::deallocate(pointer, quantity); }
};

} // namespace memory
} // namespace turbo

#endif
