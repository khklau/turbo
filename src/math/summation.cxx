#include "summation.hpp"

namespace turbo {
namespace math {

std::uint64_t log_ceiling_in_series(std::uint32_t base, std::uint32_t exponent, std::uint64_t sum)
{
    std::uint64_t log = 0U;
    std::uint64_t current = base;
    for (std::uint64_t tally = 0U; tally < sum;)
    {
	tally += current;
	current *= exponent;
	++log;
    }
    return log;
}

} // namespace math
} // namespace turbo
