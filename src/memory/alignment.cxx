#include "alignment.hpp"
#include <cstdint>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace memory {

void* align(std::size_t alignment, std::size_t element_size, void*& buffer, std::size_t& available_space)
{
    if (available_space < alignment || buffer == nullptr)
    {
	return nullptr;
    }
    const std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(buffer);
    const std::uintptr_t aligned_ptr = (ptr + alignment - 1) & - alignment;
    const std::size_t padding = aligned_ptr - ptr;
    if (available_space < (element_size + padding))
    {
	return nullptr;
    }
    available_space -= padding;
    buffer = reinterpret_cast<void*>(aligned_ptr);
    return buffer;
}

std::size_t calc_total_aligned_size(std::size_t value_size, std::size_t value_alignment, std::size_t quantity)
{
    const std::size_t total_size = value_size * quantity;
    if (total_size == 0U || value_alignment == 0U || value_size == value_alignment || (value_alignment < value_size && value_size % value_alignment == 0U))
    {
	return total_size;
    }
    else
    {
	return ((value_size + value_alignment) / value_alignment) * value_alignment * quantity;
    }
}

} // namespace memory
} // namespace turbo
