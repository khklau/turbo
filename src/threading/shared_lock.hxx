#ifndef TURBO_THREADING_SHARED_LOCK_HXX
#define TURBO_THREADING_SHARED_LOCK_HXX

#include <turbo/threading/shared_lock.hpp>

namespace turbo {
namespace threading {

template <class mutex_t>
shared_lock<mutex_t>::shared_lock()
    :
	mutex_(nullptr),
	owns_lock_(false)
{ }

template <class mutex_t>
shared_lock<mutex_t>::shared_lock(mutex_t& mutex)
    :
	mutex_(&mutex),
	owns_lock_(false)
{
    mutex_->lock_shared();
    owns_lock_ = true;
}

template <class mutex_t>
shared_lock<mutex_t>::shared_lock(mutex_t& mutex, std::adopt_lock_t)
    :
	shared_lock(mutex)
{ }

template <class mutex_t>
shared_lock<mutex_t>::shared_lock(mutex_t& mutex, std::defer_lock_t)
    :
	mutex_(&mutex),
	owns_lock_(false)
{ }

template <class mutex_t>
shared_lock<mutex_t>::shared_lock(mutex_t& mutex, std::try_to_lock_t)
    :
	mutex_(&mutex),
	owns_lock_(false)
{
    owns_lock_ = mutex_->try_lock_shared();
}

template <class mutex_t>
shared_lock<mutex_t>::~shared_lock()
{
    unlock();
}

template <class mutex_t>
mutex_t* shared_lock<mutex_t>::release()
{
    mutex_t* result = mutex_;
    mutex_ = nullptr;
    owns_lock_ = false;
    return result;
}

template <class mutex_t>
void shared_lock<mutex_t>::lock()
{
    if (mutex_)
    {
	mutex_->lock_shared();
	owns_lock_ = true;
    }
}

template <class mutex_t>
bool shared_lock<mutex_t>::try_lock()
{
    owns_lock_ = (mutex_) ? mutex_->try_lock_shared() : false;
    return owns_lock_;
}

template <class mutex_t>
void shared_lock<mutex_t>::unlock()
{
    if (mutex_ && owns_lock_)
    {
	mutex_->unlock_shared();
    }
}

} // namespace threading
} // namespace turbo

#endif
