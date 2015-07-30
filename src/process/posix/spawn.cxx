#include "spawn.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <utility>
#include <vector>

namespace {

using turbo::ipc::posix::pipe::end_pair;
using turbo::ipc::posix::pipe::replace_result;
using turbo::process::posix::child;

child init_parent(pid_t pid, end_pair&& in, end_pair&& out, end_pair&& err)
{
    // only keep the pipe ends the parent need and let the rest fall of out scope
    return child(pid, std::move(in.second), std::move(out.first), std::move(err.first));
}

void init_child(end_pair&& in, end_pair&& out, end_pair&& err)
{
    while (in.first.replace_stdin() != replace_result::success)
    {
	std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    while (out.second.replace_stdout() != replace_result::success)
    {
	std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    while (err.second.replace_stderr() != replace_result::success)
    {
	std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    // after standard streams have been replaced, the child no longer needs
    // the pipes, so let them fall out of scope
}

} // anonymous namespace

namespace turbo {
namespace process {
namespace posix {

using turbo::ipc::posix::pipe::end_pair;
using turbo::ipc::posix::pipe::front;
using turbo::ipc::posix::pipe::back;
using turbo::ipc::posix::pipe::option;
using turbo::ipc::posix::pipe::make_pipe;

child::child(const pid_t& pid, back&& instream, front&& outstream, front&& errstream) :
	in(std::move(instream)),
	out(std::move(outstream)),
	err(std::move(errstream)),
	pid_(pid)
{ }

child::child(child&& other) noexcept :
	in(std::move(other.in)),
	out(std::move(other.out)),
	err(std::move(other.err)),
	pid_(other.pid_)
{
    other.pid_ = -1;
}

child::~child()
{
    int status = 0;
    if (pid_ >= 0)
    {
	waitpid(pid_, &status, 0);
    }
}

child& child::operator=(child&& other)
{
    if (this != &other)
    {
	in = std::move(other.in);
	out = std::move(other.out);
	err = std::move(other.err);
	pid_ = other.pid_;
	other.pid_ = -1;
    }
    return *this;
}

child spawn(const char* exepath, char* const args[], char* const env[])
{
    std::vector<option> options {
	    option::non_blocking,
	    option::fork_compatible
    };
    end_pair&& in = make_pipe(options);
    end_pair&& out = make_pipe(options);
    end_pair&& err = make_pipe(options);
    pid_t pid = fork();
    if (pid == 0)
    {
	// child continues here
	init_child(std::move(in), std::move(out), std::move(err));
	execve(exepath, args, env);
	// arriving here means the child has some error
	exit(errno);
    }
    else if (pid == -1)
    {
	// fork failure
	throw insufficient_resource_error();
    }
    else
    {
	// parent continues here
	return std::move(init_parent(pid, std::move(in), std::move(out), std::move(err)));
    }
}

} // namespace posix
} // namespace process
} // namespace turbo
