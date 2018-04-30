#ifndef TURBO_THREADING_SCOPED_THREAD_HPP
#define TURBO_THREADING_SCOPED_THREAD_HPP

#include <thread>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace threading {

class TURBO_SYMBOL_DECL scoped_thread
{
public:
    scoped_thread(std::thread&& thread) noexcept;
    scoped_thread(scoped_thread&& other) noexcept;
    ~scoped_thread();
    scoped_thread& operator=(scoped_thread&& other) noexcept;
private:
    scoped_thread() = delete;
    scoped_thread(const scoped_thread&) = delete;
    scoped_thread& operator=(const scoped_thread&) = delete;
    std::thread thread_;
};
    
} // namespace threading
} // namespace turbo

#endif
