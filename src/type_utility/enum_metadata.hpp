#ifndef TURBO_TYPE_UTILITY_ENUM_METADATA_HPP
#define TURBO_TYPE_UTILITY_ENUM_METADATA_HPP

#include <cstddef>
#include <type_traits>

namespace turbo {
namespace type_utility {

template <class enum_t>
constexpr std::size_t enum_count(enum_t first, enum_t last)
{
    typedef typename std::underlying_type<enum_t>::type underlying;
    return static_cast<underlying>(last) - static_cast<underlying>(first) + 1U;
}

} // namespace type_utility
} // namespace turbo

#endif
