#include "pipe.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <system_error>
#include <turbo/toolset/extension.hpp>

namespace {

using namespace turbo::ipc::posix::pipe;

replace_result replace(int handle, int stdstream)
{
    if (handle < 0)
    {
	return replace_result::success;
    }
    replace_result tmp = replace_result::interrupted;
    int flags = fcntl(handle, F_GETFL, 0);
    if (dup2(handle, stdstream) != -1)
    {
	fcntl(stdstream, F_SETFL, flags);
	tmp = replace_result::success;
    }
    else
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
	    {
		throw std::system_error(errno, std::system_category(), "dup3 produced EBADF error");
	    }
	    case EINVAL:
	    {
		throw std::system_error(errno, std::system_category(), "dup3 produced EINVAL error");
	    }
	    default:
	    {
		throw std::system_error(errno, std::system_category(), "dup3 produced unknown error");
	    }
	}
    }
    return tmp;
}

void set_size(int handle, std::size_t size, const char* mode)
{
    FILE* file = fdopen(handle, mode);
    if (file == NULL)
    {
	throw std::system_error(errno, std::system_category(), "fdopen produced unknown error");
    }
    else
    {
	if (setvbuf(file, NULL, _IOFBF, size) != 0)
	{
	    throw std::system_error(errno, std::system_category(), "setvbuf produced unknown error");
	}
    }
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

front::front(const key&, const handle& handle) :
	handle_(handle)
{ }

front::front(front&& other) noexcept :
	handle_(other.handle_)
{
    other.handle_ = -1;
}

front::~front()
{
    if (is_open())
    {
	close(handle_);
    }
}

front& front::operator=(front&& other)
{
    if (TURBO_LIKELY(this != &other))
    {
	handle_ = other.handle_;
	other.handle_ = -1;
    }
    return *this;
}

bool front::is_open() const
{
    return (handle_ >= 0);
}

io_result front::read(void* buf, std::size_t requested_bytes, std::size_t& actual_bytes)
{
    ssize_t result = ::read(handle_, buf, requested_bytes);
    if (TURBO_LIKELY(result > 0))
    {
	actual_bytes = static_cast<std::size_t>(result);
    }
    else if (result == 0)
    {
	actual_bytes = 0;
    }
    else
    {
	switch (errno)
	{
	    case EAGAIN:
	    {
		return io_result::would_block;
	    }
	    case EBADF:
	    {
		throw used_after_move_error();
	    }
	    case EFAULT:
	    {
		throw invalid_buffer_error();
	    }
	    case EINTR:
	    {
		return io_result::interrupted;
	    }
	    default:
	    {
		throw std::system_error(errno, std::system_category(), "read produced unexpected error");
	    }
	}
    }
    return io_result::success;
}

void front::read_all(void* buf, std::size_t requested_bytes)
{
    unsigned char* pos = static_cast<unsigned char*>(buf);
    unsigned char* end = pos + requested_bytes;
    std::size_t actual_bytes = 0;
    while (pos < end)
    {
	if (read(pos, end - pos, actual_bytes) == io_result::success)
	{
	    if (actual_bytes == 0)
	    {
		break;
	    }
	    else
	    {
		pos += actual_bytes;
	    }
	}
    }
}

replace_result front::replace_stdin()
{
    return ::replace(handle_, STDIN_FILENO);
}

back::back(const key&, const handle& handle) :
	handle_(handle)
{ }

back::back(back&& other) noexcept :
	handle_(other.handle_)
{
    other.handle_ = -1;
}

back::~back()
{
    if (is_open())
    {
	close(handle_);
    }
}

back& back::operator=(back&& other)
{
    if (this != &other)
    {
	handle_ = other.handle_;
	other.handle_ = -1;
    }
    return *this;
}

bool back::is_open() const
{
    return (handle_ >= 0);
}

io_result back::write(void* buf, std::size_t requested_bytes, std::size_t& actual_bytes)
{
    ssize_t result = ::write(handle_, buf, requested_bytes);
    if (TURBO_LIKELY(result > 0))
    {
	actual_bytes = static_cast<std::size_t>(result);
    }
    else if (result == 0)
    {
	actual_bytes = 0;
    }
    else
    {
	switch (errno)
	{
	    case EAGAIN:
	    {
		return io_result::would_block;
	    }
	    case EBADF:
	    {
		throw used_after_move_error();
	    }
	    case EFAULT:
	    {
		throw invalid_buffer_error();
	    }
	    case EINTR:
	    {
		return io_result::interrupted;
	    }
	    case ENOSPC:
	    {
		return io_result::pipe_full;
	    }
	    default:
	    {
		throw std::system_error(errno, std::system_category(), "write produced unexpected error");
	    }
	}
    }
    return io_result::success;
}

replace_result back::replace_stdout()
{
    return ::replace(handle_, STDOUT_FILENO);
}

replace_result back::replace_stderr()
{
    return ::replace(handle_, STDERR_FILENO);
}

end_pair make_pipe(std::vector<option>& options, std::size_t bufsize)
{
    int flags = 0;
    for (auto iter = options.cbegin(); iter != options.cend(); ++iter)
    {
	switch (*iter)
	{
	    case option::non_blocking:
	    {
		flags |= O_NONBLOCK;
		break;
	    }
	    case option::fork_compatible:
	    {
		flags |= O_CLOEXEC;
		break;
	    }
	}
    }
    int tmp[2];
    int result = ::pipe2(tmp, flags);
    if (result == -1)
    {
	switch (errno)
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
	    {
		throw std::system_error(errno, std::system_category(), "pipe2 produced EFAULT error");
	    }
	    case EINVAL:
	    {
		throw std::system_error(errno, std::system_category(), "pipe2 produced EINVAL error");
	    }
	    default:
	    {
		throw std::system_error(errno, std::system_category(), "pipe2 produced unknown error");
	    }
	}
    }
    set_size(tmp[0], bufsize, "r");
    set_size(tmp[1], bufsize, "w");
    return std::make_pair(front(key(), tmp[0]), back(key(), tmp[1]));
}

} // namespace pipe
} // namespace posix
} // namespace ipc
} // namespace turbo
