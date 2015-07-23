#include "spawn.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace turbo::process::posix;
using turbo::ipc::posix::pipe::end_pair;
using turbo::ipc::posix::pipe::replace_result;

child init_parent(pid_t pid, end_pair&& in, end_pair&& out, end_pair&& err)
{
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

child::child(pid_t childpid, back&& instream, front&& outstream, front&& errstream) :
	pid(childpid),
	in(std::move(instream)),
	out(std::move(outstream)),
	err(std::move(errstream))
{ }

child::child(child&& other) noexcept :
	pid(other.pid),
	in(std::move(other.in)),
	out(std::move(other.out)),
	err(std::move(other.err))
{
    other.pid = -1;
}

child::~child()
{
    int status = 0;
    if (pid >= 0)
    {
	waitpid(pid, &status, 0);
    }
}

child& child::operator=(child&& other)
{
    if (this != &other)
    {
	pid = other.pid;
	in = std::move(other.in);
	out = std::move(other.out);
	err = std::move(other.err);
	other.pid = -1;
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
	if (execve(exepath, args, env) == -1)
	{
	    exit(errno);
	}
	else
	{
	    exit(0);
	}
    }
    else if (pid == -1)
    {
	// fork failure
	throw insufficient_resource_error();
    }
    else
    {
	// parent continues here
	return init_parent(pid, std::move(in), std::move(out), std::move(err));
    }
}

} // namespace posix
} // namespace process
} // namespace turbo
