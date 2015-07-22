#include "spawn.hpp"
#include <unistd.h>
#include <chrono>
#include <thread>
#include <vector>

namespace turbo {
namespace process {
namespace posix {

using turbo::ipc::posix::pipe;

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
