#include "summation.hpp"

namespace turbo {
namespace math {

std::uint32_t ceiling_bound(std::uint32_t base, std::uint32_t multiplier, std::uint32_t sum)
{
    if (multiplier < 2U)
    {
	throw invalid_multiplier_error("Multiplier cannot be less than 2");
    }
    std::uint32_t log = 0U;
    std::uint64_t current = base;
    for (std::uint64_t tally = 0U; tally < sum;)
    {
	tally += current;
	current *= multiplier;
	++log;
    }
    return log;
}

} // namespace math
} // namespace turbo
