#ifndef TURBO_ALGORITHM_RECOVERY_HPP
#define TURBO_ALGORITHM_RECOVERY_HPP

#include <functional>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace algorithm {
namespace recovery {

enum class try_state
{
    done,
    retry
};

TURBO_SYMBOL_DECL void retry_with_random_backoff(const std::function<try_state ()>& func, uint64_t max_backoff = 32U);

} // namespace recovery
} // namespace algorithm
} // namespace turbo

#endif
