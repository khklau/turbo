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

template <std::uint32_t base, std::uint32_t exponent>
class power
{
private:
    template <std::uint32_t total_, std::uint32_t base_, std::uint32_t remaining_>
    struct impl
    {
	static const std::uint32_t result = impl<total_ * base_, base_, remaining_ - 1>::result;
    };
    template <std::uint32_t total_, std::uint32_t base_>
    struct impl<total_, base_, 0U>
    {
	static const std::uint32_t result = total_;
    };
public:
    static const std::uint32_t result = impl<1U, base, exponent>::result;
};

} // namespace math
} // namespace turbo

#endif
