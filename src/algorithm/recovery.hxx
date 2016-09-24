#ifndef TURBO_ALGORITHM_RECOVERY_HXX
#define TURBO_ALGORITHM_RECOVERY_HXX

#include <turbo/algorithm/recovery.hpp>

namespace turbo {
namespace algorithm {
namespace recovery {

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
