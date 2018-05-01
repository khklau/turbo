#include "semaphore.hpp"
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hxx>

namespace turbo {
namespace threading {

semaphore::semaphore(std::uint32_t limit)
    :
	limit_(limit),
	mutex_(),
	condition_(),
	counter_(0U)
{ }

void semaphore::lock()
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [&]() -> bool
    {
	if (counter_ < limit_)
	{
	    ++counter_;
	    return true;
	}
	return false;
    });
}

void semaphore::unlock()
{
    {
	std::lock_guard<std::mutex> lock(mutex_);
	if (0U < counter_)
	{
	    --counter_;
	}
    }
    condition_.notify_all();
}

} // namespace threading
} // namespace turbo
