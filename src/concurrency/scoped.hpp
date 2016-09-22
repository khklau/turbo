#ifndef TURBO_CONCURRENCY_SCOPED_HPP
#define TURBO_CONCURRENCY_SCOPED_HPP

#include <atomic>

namespace turbo {
namespace concurrency {

template <class value_t, value_t new_value_c>
class scoped_update
{
public:
    scoped_update(std::atomic<value_t>& value)
	:
	    value_(value),
	    enabled_(true)
    { }
    inline ~scoped_update()
    {
	if (enabled_)
	{
	    value_.store(new_value_c, std::memory_order::release);
	}
    }
    inline disable()
    {
	enabled_ = false;
    }
private:
    std::atomic<value_t>& value_;
    bool enabled_;
};

} // namespace concurrency
} // namespace turbo

#endif
