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

template <typename func_t>
TURBO_SYMBOL_DECL inline void retry_with_random_backoff(func_t func, uint64_t max_backoff = 8U);

template <class try_clause_t, class ensure_clause_t>
TURBO_SYMBOL_DECL inline void try_and_ensure(const try_clause_t& try_clause, const ensure_clause_t& ensure_clause);

} // namespace recovery
} // namespace algorithm
} // namespace turbo

#endif
