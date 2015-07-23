#ifndef TURBO_PROCESS_POSIX_SPAWN_HPP
#define TURBO_PROCESS_POSIX_SPAWN_HPP

#include <unistd.h>
#include <memory>
#include <turbo/ipc/posix/pipe.hpp>

namespace turbo {
namespace process {
namespace posix {

struct stdstream
{
    typedef std::vector<turbo::ipc::posix::pipe::option> options; 
    stdstream(options& inopt, options& outopt, options& erropt);
    turbo::ipc::posix::pipe in;
    turbo::ipc::posix::pipe out;
    turbo::ipc::posix::pipe err;
};

struct insufficient_resource_error { };

std::unique_ptr<stdstream> spawn(const char* exepath, char* const args[], char* const env[]);

struct child
{
    pid_t pid;
    turbo::ipc::posix::pipe_back in;
    turbo::ipc::posix::pipe_front out;
    turbo::ipc::posix::pipe_front err;
    child(pid_t childpid, turbo::ipc::posix::pipe_back&& instream, turbo::ipc::posix::pipe_front&& outstream, turbo::ipc::posix::pipe_front&& errstream); 
    child(child&& other) noexcept;
    ~child();
    child& operator=(child&& other);
private:
    child() = delete;
    child(const child& other) = delete;
    child& operator=(const child& other) = delete;
};

child spawn2(const char* exepath, char* const args[], char* const env[]);

} // namespace posix
} // namespace process
} // namespace turbo

#endif
