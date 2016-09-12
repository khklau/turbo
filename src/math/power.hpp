#ifndef TURBO_MATH_POWER_HPP
#define TURBO_MATH_POWER_HPP

#include <cmath>
#include <cstdint>

namespace turbo {
namespace math {

inline std::uint32_t pow(std::uint32_t base, std::uint32_t exponent)
{
    return static_cast<std::uint32_t>(std::pow(static_cast<double>(base), static_cast<double>(exponent)));
}

} // namespace math
} // namespace turbo

#endif
