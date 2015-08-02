#ifndef TURBO_PROCESS_POSIX_SPAWN_HPP
#define TURBO_PROCESS_POSIX_SPAWN_HPP

#include <unistd.h>
#include <turbo/ipc/posix/pipe.hpp>

namespace turbo {
namespace process {
namespace posix {

struct insufficient_resource_error { };

class child
{
public:
    turbo::ipc::posix::pipe::back in;
    turbo::ipc::posix::pipe::front out;
    turbo::ipc::posix::pipe::front err;
    child(const pid_t& childpid,
	    turbo::ipc::posix::pipe::back&& instream,
	    turbo::ipc::posix::pipe::front&& outstream,
	    turbo::ipc::posix::pipe::front&& errstream); 
    child(child&& other) noexcept;
    ~child();
    child& operator=(child&& other);
    inline const pid_t& get_pid() { return pid_; }
private:
    child() = delete;
    child(const child& other) = delete;
    child& operator=(const child& other) = delete;
    pid_t pid_;
};

child spawn(const char* exepath, char* const args[], char* const env[], std::size_t bufsize);

} // namespace posix
} // namespace process
} // namespace turbo

#endif
