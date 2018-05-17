#include "shared_mutex.hpp"

namespace turbo {
namespace threading {

shared_mutex::shared_mutex()
    :
	mutex_(),
	condition_(),
	read_counter_(0U),
	write_counter_(0U)
{ }

bool shared_mutex::try_lock_shared()
{
    std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
    if (lock.try_lock())
    {
	if (write_counter_ != 0U)
	{
	    wait_shareable(lock);
	}
	++read_counter_;
	return true;
    }
    return false;
}

void shared_mutex::lock_shared()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (write_counter_ != 0U);
    {
	wait_shareable(lock);
    }
    ++read_counter_;
}

void shared_mutex::unlock_shared()
{
    {
	std::lock_guard<std::mutex> lock(mutex_);
	if (0U < read_counter_)
	{
	    --read_counter_;
	}
    }
    condition_.notify_all();
}

bool shared_mutex::try_lock()
{
    std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
    if (lock.try_lock())
    {
	wait_exclusive(lock);
	return true;
    }
    else
    {
	return false;
    }
}

void shared_mutex::lock()
{
    std::unique_lock<std::mutex> lock(mutex_);
    wait_exclusive(lock);
}

void shared_mutex::unlock()
{
    {
	std::lock_guard<std::mutex> lock(mutex_);
	if (0U < write_counter_)
	{
	    --write_counter_;
	}
    }
    condition_.notify_all();
}

void shared_mutex::wait_shareable(std::unique_lock<std::mutex>& lock)
{
    condition_.wait(lock, [&]() -> bool
    {
	return (write_counter_ == 0U);
    });
}

void shared_mutex::wait_exclusive(std::unique_lock<std::mutex>& lock)
{
    condition_.wait(lock, [&]() -> bool
    {
	return (read_counter_ == 0U && write_counter_ == 0U);
    });
}

} // namespace threading
} // namespace turbo
