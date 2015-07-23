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
using turbo::ipc::posix::pipe2;
using turbo::ipc::posix::replace_result;

child init_parent(pid_t pid, pipe2&& in, pipe2&& out, pipe2&& err)
{
    return child(pid, std::move(in.second), std::move(out.first), std::move(err.first));
}

void init_child(pipe2&& in, pipe2&& out, pipe2&& err)
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

using turbo::ipc::posix::pipe;
using turbo::ipc::posix::pipe2;
using turbo::ipc::posix::pipe_front;
using turbo::ipc::posix::pipe_back;
using turbo::ipc::posix::pipe_option;
using turbo::ipc::posix::make_pipe;

child::child(pid_t childpid, pipe_back&& instream, pipe_front&& outstream, pipe_front&& errstream) :
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

child spawn2(const char* exepath, char* const args[], char* const env[])
{
    std::vector<pipe_option> options {
	    pipe_option::non_blocking,
	    pipe_option::fork_compatible
    };
    pipe2&& in = make_pipe(options);
    pipe2&& out = make_pipe(options);
    pipe2&& err = make_pipe(options);
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

stdstream::stdstream(options& inopt, options& outopt, options& erropt) :
	in(inopt),
	out(outopt),
	err(erropt)
{ }

std::unique_ptr<stdstream> spawn(const char* exepath, char* const args[], char* const env[])
{
    std::vector<pipe::option> options {
	    pipe::option::non_blocking,
	    pipe::option::fork_compatible
    };
    std::unique_ptr<stdstream> pipes(new stdstream(options, options, options));
    pid_t pid = fork();
    if (pid == 0)
    {
	// child continues here
	while (pipes->in.replace_stdin() != pipe::result::success)
	{
	    std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	while (pipes->out.replace_stdout() != pipe::result::success)
	{
	    std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	while (pipes->err.replace_stderr() != pipe::result::success)
	{
	    std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	pipes.reset();
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
	return pipes;
    }
}


} // namespace posix
} // namespace process
} // namespace turbo
