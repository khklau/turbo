#ifndef TURBO_ALGORITHM_RECOVERY_HXX
#define TURBO_ALGORITHM_RECOVERY_HXX

#include <turbo/algorithm/recovery.hpp>
#include <chrono>
#include <random>

namespace turbo {
namespace algorithm {
namespace recovery {

inline std::random_device& get_device()
{
    thread_local std::random_device device;
    return device;
}

template <typename func_t>
void retry_with_random_backoff(func_t func, uint64_t max_backoff)
{
    uint64_t limit = 0U;
    while (func() == try_state::retry)
    {
	limit = get_device()() % max_backoff;
	for (uint64_t iter = 0U; iter < limit; ++iter) { };
    }
}

template <class try_clause_t, class ensure_clause_t>
inline void try_and_ensure(const try_clause_t& try_clause, const ensure_clause_t& ensure_clause)
{
    struct impl_
    {
	impl_(const ensure_clause_t& clause)
	    :
		clause_(clause)
	{ }
	~impl_() noexcept
	{
	    try
	    {
		clause_();
	    }
	    catch (...)
	    {
		// do nothing
	    }
	}
	const ensure_clause_t& clause_;
    };
    {
	impl_ tmp(ensure_clause);
	try_clause();
    }
}

} // namespace recovery
} // namespace algorithm
} // namespace turbo

#endif
