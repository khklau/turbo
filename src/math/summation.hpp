#ifndef TURBO_MATH_SUMMATION_HPP
#define TURBO_MATH_SUMMATION_HPP

#include <cstdint>
#include <stdexcept>
#include <string>

namespace turbo {
namespace math {

class invalid_multiplier_error : std::invalid_argument
{
public:
    invalid_multiplier_error(const char* what) : invalid_argument(what) { }
    invalid_multiplier_error(const std::string& what) : invalid_argument(what) { }
};

std::uint64_t ceiling_bound(std::uint32_t base, std::uint32_t multiplier, std::uint32_t sum);


} // namespace math
} // namespace turbo

#endif
