#ifndef TURBO_THREADING_SHARED_LOCK_HPP
#define TURBO_THREADING_SHARED_LOCK_HPP

#include <mutex>

namespace turbo {
namespace threading {

template <class mutex_t>
class shared_lock
{
public:
    typedef mutex_t mutex_type;
    shared_lock();
    shared_lock(mutex_t& mutex);
    shared_lock(mutex_t& mutex, std::adopt_lock_t);
    shared_lock(mutex_t& mutex, std::defer_lock_t);
    shared_lock(mutex_t& mutex, std::try_to_lock_t);
    ~shared_lock();
    inline mutex_t* mutex() const { return mutex_; }
    inline bool owns_lock() const { return owns_lock_; }
    inline explicit operator bool() const { return owns_lock_; }
    mutex_t* release();
    void lock();
    bool try_lock();
    void unlock();
private:
    mutex_t* mutex_;
    bool owns_lock_;
};

} // namespace threading
} // namespace turbo

#endif
