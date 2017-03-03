#ifndef TURBO_ALGORITHM_RECOVERY_HXX
#define TURBO_ALGORITHM_RECOVERY_HXX

#include <turbo/algorithm/recovery.hpp>
#include <chrono>
#include <random>

namespace turbo {
namespace algorithm {
namespace recovery {

template <typename func_t>
void retry_with_random_backoff(func_t func, uint64_t max_backoff)
{
    //std::random_device device;
    std::mt19937 device(std::chrono::system_clock::now().time_since_epoch().count());
    uint64_t limit = 0U;
    while (func() == try_state::retry)
    {
	limit = device() % max_backoff;
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
