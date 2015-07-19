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

std::pair<pipe::front::handle, pipe::back::handle> pipe::init(std::vector<option>& options)
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
	    throw process_limit_reached_error();
	case ENFILE:
	    throw system_limit_reached_error();
	case EFAULT:
	case EINVAL:
	{
	    assert(false);
	}
    }
    return std::make_pair(tmp[0], tmp[1]);
}

pipe::pipe(std::pair<front::handle, back::handle> handles) :
	front_(handles.first),
	back_(handles.second)
{ }

pipe::front::front(handle front) :
	front_(front)
{ }

pipe::front::~front()
{
    close(front_);
}

pipe::back::back(handle back) :
	back_(back)
{ }

pipe::back::~back()
{
    close(back_);
}

} // namespace posix
} // namespace ipc
} // namespace turbo
