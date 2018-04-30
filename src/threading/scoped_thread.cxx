#include "scoped_thread.hpp"
#include <thread>
#include <utility>

namespace turbo {
namespace threading {

scoped_thread::scoped_thread(std::thread&& thread) noexcept
    :
	thread_(std::move(thread))
{ }

scoped_thread::scoped_thread(scoped_thread&& other) noexcept
    :
	thread_(std::move(other.thread_))
{ }

scoped_thread::~scoped_thread()
{
    if (thread_.joinable())
    {
	thread_.join();
    }
}

scoped_thread& scoped_thread::operator=(scoped_thread&& other) noexcept
{
    thread_ = std::move(other.thread_);
    return *this;
}
    
} // namespace threading
} // namespace turbo
