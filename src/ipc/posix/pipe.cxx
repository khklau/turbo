#include "pipe.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>

namespace {

using namespace turbo::ipc::posix;

replace_result replace(int handle, int stdstream, int options)
{
    replace_result tmp = replace_result::success;
    if (handle >= 0 && dup3(handle, stdstream, options) == -1)
    {
	switch (errno)
	{
	    case EINTR:
	    {
		tmp = replace_result::interrupted;
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

} // anonymous namespace

namespace turbo {
namespace ipc {
namespace posix {

pipe_front::pipe_front(int options, const handle& handle) :
	options_(options),
	handle_(handle)
{ }

pipe_front::pipe_front(pipe_front&& other) noexcept :
	options_(other.options_),
	handle_(other.handle_)
{
    other.options_ = 0;
    other.handle_ = -1;
}

pipe_front::~pipe_front()
{
    if (handle_ >= 0)
    {
	close(handle_);
    }
}

pipe_front& pipe_front::operator=(pipe_front&& other)
{
    if (this != &other)
    {
	options_ = other.options_;
	handle_ = other.handle_;
	other.options_ = 0;
	other.handle_ = -1;
    }
    return *this;
}

replace_result pipe_front::replace_stdin()
{
    return ::replace(handle_, STDIN_FILENO, options_);
}

pipe_back::pipe_back(int options, const handle& handle) :
	options_(options),
	handle_(handle)
{ }

pipe_back::pipe_back(pipe_back&& other) noexcept :
	options_(other.options_),
	handle_(other.handle_)
{
    other.options_ = 0;
    other.handle_ = -1;
}

pipe_back::~pipe_back()
{
    if (handle_ >= 0)
    {
	close(handle_);
    }
}

pipe_back& pipe_back::operator=(pipe_back&& other)
{
    if (this != &other)
    {
	options_ = other.options_;
	handle_ = other.handle_;
	other.options_ = 0;
	other.handle_ = -1;
    }
    return *this;
}

replace_result pipe_back::replace_stdout()
{
    return ::replace(handle_, STDOUT_FILENO, options_);
}

replace_result pipe_back::replace_stderr()
{
    return ::replace(handle_, STDERR_FILENO, options_);
}

pipe2 make_pipe(std::vector<pipe_option>& options)
{
    int opt = 0;
    for (auto iter = options.cbegin(); iter != options.cend(); ++iter)
    {
	switch (*iter)
	{
	    case pipe_option::non_blocking:
	    {
		opt |= O_NONBLOCK;
		break;
	    }
	    case pipe_option::fork_compatible:
	    {
		opt |= O_CLOEXEC;
		break;
	    }
	}
    }
    int tmp[2];
    int result = ::pipe2(tmp, opt);
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
    return std::make_pair(pipe_front(opt, tmp[0]), pipe_back(opt, tmp[1]));
}

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
    int result = ::pipe2(tmp, opt);
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
