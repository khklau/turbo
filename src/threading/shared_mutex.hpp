#ifndef TURBO_THREADING_SHARED_MUTEX_HPP
#define TURBO_THREADING_SHARED_MUTEX_HPP

#include <cstdint>
#include <condition_variable>
#include <mutex>

namespace turbo {
namespace threading {

class shared_mutex
{
public:
    typedef std::mutex::native_handle_type native_handle_type;
    shared_mutex();
    bool try_lock_shared();
    void lock_shared();
    void unlock_shared();
    bool try_lock();
    void lock();
    void unlock();
    inline native_handle_type native_handle() { return data_mutex_.native_handle(); }
private:
    inline void wait_shareable(std::unique_lock<std::mutex>& lock);
    inline void wait_exclusive(std::unique_lock<std::mutex>& lock);
    std::mutex counter_mutex_;
    std::mutex data_mutex_;
    std::condition_variable condition_;
    std::uint32_t read_counter_;
    std::uint32_t write_counter_;
};

} // namespace threading
} // namespace turbo

#endif
