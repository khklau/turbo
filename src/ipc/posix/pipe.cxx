#include "pipe.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>

namespace turbo {
namespace ipc {
namespace posix {

pipe::pipe(std::vector<option>& options) :
	pipe(init(options))
{ }

std::tuple<int, pipe::handle, pipe::handle> pipe::init(std::vector<pipe::option>& options)
{
    int opt = 0;
    for (auto iter = options.cbegin(); iter != options.cend(); ++iter)
    {
	switch (*iter)
	{
	    case option::non_blocking:
	    {
		opt |= O_NONBLOCK;
		break;
	    }
	    case option::fork_compatible:
	    {
		opt |= O_CLOEXEC;
		break;
	    }
	}
    }
    int tmp[2];
    int result = pipe2(tmp, opt);
    switch (result)
    {
	case EMFILE:
	{
	    throw process_limit_reached_error();
	}
	case ENFILE:
	{
	    throw system_limit_reached_error();
	}
	case EFAULT:
	case EINVAL:
	{
	    assert(false);
	}
    }
    return std::make_tuple(opt, tmp[0], tmp[1]);
}

pipe::pipe(std::tuple<int, handle, handle> args) :
	options_(std::get<0>(args)),
	front_(std::get<1>(args)),
	back_(std::get<2>(args))
{ }

pipe::~pipe()
{
    close(back_);
    close(front_);
}

pipe::result pipe::replace_stdin()
{
    return replace(front_, STDIN_FILENO);
}

pipe::result pipe::replace_stdout()
{
    return replace(back_, STDOUT_FILENO);
}

pipe::result pipe::replace_stderr()
{
    return replace(back_, STDERR_FILENO);
}

pipe::result pipe::replace(handle end, handle stdstream)
{
    result tmp = result::success;
    if (dup3(end, stdstream, options_) == -1)
    {
	switch (errno)
	{
	    case EINTR:
	    {
		tmp = result::interrupted;
		break;
	    }
	    case EBUSY:
	    {
		throw race_condition_error();
	    }
	    case EMFILE:
	    {
		throw process_limit_reached_error();
	    }
	    case EBADF:
	    case EINVAL:
	    {
		assert(false);
	    }
	}
    }
    return tmp;
}

} // namespace posix
} // namespace ipc
} // namespace turbo
