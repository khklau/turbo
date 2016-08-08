#include "alignment.hpp"
#include <cstdint>

namespace turbo {
namespace memory {

void* align(std::size_t alignment, std::size_t element_size, void*& buffer, std::size_t& available_space)
{
    if (available_space < alignment)
    {
	return nullptr;
    }
    std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(buffer);
    std::uintptr_t aligned_ptr = (ptr + alignment - 1) & - alignment;
    std::size_t padding = aligned_ptr - ptr;
    if (available_space < (element_size + padding))
    {
	return nullptr;
    }
    available_space -= padding;
    buffer = reinterpret_cast<void*>(aligned_ptr);
    return buffer;
}

} // namespace memory
} // namespace turbo
