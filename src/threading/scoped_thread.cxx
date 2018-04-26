#ifndef TURBO_THREADING_SCOPED_THREAD_HXX
#define TURBO_THREADING_SCOPED_THREAD_HXX

#include <thread>

namespace turbo {
namespace threading {

scoped_thread::scoped_thread(std::thread&& thread)
    :
	thread_(thread)
{ }

scoped_thread::~scoped_thread()
{
    if (thread_.joinable())
    {
	thread_.join();
    }
}
    
} // namespace threading
} // namespace turbo

#endif

