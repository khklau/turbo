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
    semaphore(std::uint32_t limit);
    void lock();
    void unlock();
private:
    const std::uint32_t limit_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::uint32_t counter_;
};

} // namespace threading
} // namespace turbo

#endif
