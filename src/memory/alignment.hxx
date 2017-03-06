#ifndef TURBO_MEMORY_ALIGNMENT_HXX
#define TURBO_MEMORY_ALIGNMENT_HXX

#include <turbo/memory/alignment.hpp>
#include <cmath>
#include <cstdint>

namespace turbo {
namespace memory {

std::size_t calc_total_aligned_size(std::size_t value_size, std::size_t value_alignment, std::size_t quantity)
{
    const std::size_t total_size = value_size * quantity;
    if (total_size == 0U || value_alignment == 0U || value_size == value_alignment || (value_alignment < value_size && value_size % value_alignment == 0U))
    {
	return total_size;
    }
    else
    {
	double exponent = std::log2(value_alignment);
	if (static_cast<double>(static_cast<std::int64_t>(exponent)) == exponent)
	{
	    // Avoid the cost of division if alignment is a power of 2
	    return ((value_size + value_alignment) >> static_cast<std::uint64_t>(exponent)) * value_alignment * quantity;
	}
	else
	{
	    return ((value_size + value_alignment) / value_alignment) * value_alignment * quantity;
	}
    }
}

} // namespace memory
} // namespace turbo

#endif
