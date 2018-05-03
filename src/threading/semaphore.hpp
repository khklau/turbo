#ifndef TURBO_THREADING_SEMAPHORE_HPP
#define TURBO_THREADING_SEMAPHORE_HPP

#include <cstdint>
#include <condition_variable>
#include <mutex>

namespace turbo {
namespace threading {

class semaphore
{
public:
    typedef std::mutex::native_handle_type native_handle_type;
    semaphore(std::uint32_t limit);
    bool try_lock();
    void lock();
    void unlock();
    inline native_handle_type native_handle() { return mutex_.native_handle(); }
private:
    inline void wait(std::unique_lock<std::mutex>& lock);
    const std::uint32_t limit_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::uint32_t counter_;
};

} // namespace threading
} // namespace turbo

#endif
