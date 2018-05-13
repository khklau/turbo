#include "semaphore.hpp"

namespace turbo {
namespace threading {

semaphore::semaphore(std::uint32_t limit)
    :
	limit_(limit),
	mutex_(),
	condition_(),
	counter_(0U)
{ }

bool semaphore::try_lock()
{
    std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
    if (lock.try_lock())
    {
	wait(lock);
	return true;
    }
    else
    {
	return false;
    }
}

void semaphore::lock()
{
    std::unique_lock<std::mutex> lock(mutex_);
    wait(lock);
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

void semaphore::wait(std::unique_lock<std::mutex>& lock)
{
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

} // namespace threading
} // namespace turbo
