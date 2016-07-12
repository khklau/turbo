#include "recovery.hpp"
#include <random>

namespace turbo {
namespace algorithm {
namespace recovery {

void retry_with_random_backoff(const std::function<try_state ()>& func, uint64_t max_backoff)
{
    std::random_device device;
    uint64_t limit = 0U;
    while (func() == try_state::retry)
    {
	limit = device() % max_backoff;
	for (uint64_t iter = 0U; iter < limit; ++iter) { };
    }
}

} // namespace recovery
} // namespace algorithm
} // namespace turbo
