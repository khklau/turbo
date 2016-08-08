#ifndef TURBO_MEMORY_ALIGNMENT_HPP
#define TURBO_MEMORY_ALIGNMENT_HPP

#include <cstdlib>

namespace turbo {
namespace memory {

///
/// std::align is missing from Gcc 4.8 so implement out own for now
///
void* align(std::size_t alignment, std::size_t element_size, void*& buffer, std::size_t& available_space);

} // namespace memory
} // namespace turbo

#endif
