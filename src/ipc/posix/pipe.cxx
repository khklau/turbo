#include "pipe.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>

namespace {

using namespace turbo::ipc::posix::pipe;

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
namespace pipe {

class key
{
    friend end_pair make_pipe(std::vector<option>& options);
};

front::front(const key&, int options, const handle& handle) :
	options_(options),
	handle_(handle)
{ }

front::front(front&& other) noexcept :
	options_(other.options_),
	handle_(other.handle_)
{
    other.options_ = 0;
    other.handle_ = -1;
}

front::~front()
{
    if (handle_ >= 0)
    {
	close(handle_);
    }
}

front& front::operator=(front&& other)
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

replace_result front::replace_stdin()
{
    return ::replace(handle_, STDIN_FILENO, options_);
}

back::back(const key&, int options, const handle& handle) :
	options_(options),
	handle_(handle)
{ }

back::back(back&& other) noexcept :
	options_(other.options_),
	handle_(other.handle_)
{
    other.options_ = 0;
    other.handle_ = -1;
}

back::~back()
{
    if (handle_ >= 0)
    {
	close(handle_);
    }
}

back& back::operator=(back&& other)
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

replace_result back::replace_stdout()
{
    return ::replace(handle_, STDOUT_FILENO, options_);
}

replace_result back::replace_stderr()
{
    return ::replace(handle_, STDERR_FILENO, options_);
}

end_pair make_pipe(std::vector<option>& options)
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
    return std::make_pair(front(key(), opt, tmp[0]), back(key(), opt, tmp[1]));
}

} // namespace pipe
} // namespace posix
} // namespace ipc
} // namespace turbo
