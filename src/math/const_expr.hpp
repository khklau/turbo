#ifndef TURBO_MATH_CONST_EXPR_HPP
#define TURBO_MATH_CONST_EXPR_HPP

#include <cstdint>

namespace turbo {
namespace math {
namespace const_expr {

constexpr std::int64_t ceil(double arg)
{
    return (static_cast<double>(static_cast<std::int64_t>(arg)) == arg)
	    ? static_cast<std::int64_t>(arg)
	    : static_cast<std::int64_t>(arg) + ((0 < arg) ? 1 : 0);
}

constexpr std::int64_t floor(double arg)
{
    return (static_cast<double>(static_cast<std::int64_t>(arg)) == arg)
	    ? static_cast<std::int64_t>(arg)
	    : static_cast<std::int64_t>(arg) - ((arg < 0) ? 1 : 0);
}

constexpr std::int64_t trunc(double arg)
{
    return static_cast<std::int64_t>(arg);
}

} // namespace const_expr
} // namespace math
} // turbo

#endif
