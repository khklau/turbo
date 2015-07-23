#ifndef TURBO_PROCESS_POSIX_SPAWN_HPP
#define TURBO_PROCESS_POSIX_SPAWN_HPP

#include <unistd.h>
#include <turbo/ipc/posix/pipe.hpp>

namespace turbo {
namespace process {
namespace posix {

struct insufficient_resource_error { };

struct child
{
    pid_t pid;
    turbo::ipc::posix::pipe::back in;
    turbo::ipc::posix::pipe::front out;
    turbo::ipc::posix::pipe::front err;
    child(pid_t childpid, turbo::ipc::posix::pipe::back&& instream, turbo::ipc::posix::pipe::front&& outstream, turbo::ipc::posix::pipe::front&& errstream); 
    child(child&& other) noexcept;
    ~child();
    child& operator=(child&& other);
private:
    child() = delete;
    child(const child& other) = delete;
    child& operator=(const child& other) = delete;
};

child spawn(const char* exepath, char* const args[], char* const env[]);

} // namespace posix
} // namespace process
} // namespace turbo

#endif
