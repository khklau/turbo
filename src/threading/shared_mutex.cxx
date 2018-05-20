#include "shared_mutex.hpp"

namespace turbo {
namespace threading {

shared_mutex::shared_mutex()
    :
	counter_mutex_(),
	data_mutex_(),
	condition_(),
	read_counter_(0U),
	write_counter_(0U)
{ }

bool shared_mutex::try_lock_shared()
{
    std::unique_lock<std::mutex> lock(counter_mutex_, std::defer_lock);
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
    std::unique_lock<std::mutex> lock(counter_mutex_);
    if (write_counter_ != 0U)
    {
	wait_shareable(lock);
    }
    ++read_counter_;
}

void shared_mutex::unlock_shared()
{
    std::lock_guard<std::mutex> lock(counter_mutex_);
    if (0U < read_counter_)
    {
	--read_counter_;
    }
    condition_.notify_one();
}

bool shared_mutex::try_lock()
{
    bool result = data_mutex_.try_lock();
    if (result)
    {
	std::unique_lock<std::mutex> lock(counter_mutex_);
	wait_exclusive(lock);
	++write_counter_;
    }
    return result;
}

void shared_mutex::lock()
{
    data_mutex_.lock();
    std::unique_lock<std::mutex> lock(counter_mutex_);
    wait_exclusive(lock);
    ++write_counter_;
}

void shared_mutex::unlock()
{
    std::lock_guard<std::mutex> lock(counter_mutex_);
    if (0U < write_counter_)
    {
	--write_counter_;
    }
    condition_.notify_all();
    data_mutex_.unlock();
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
