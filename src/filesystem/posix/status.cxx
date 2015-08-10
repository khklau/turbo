#include <turbo/filesystem/status.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <system_error>
#include <turbo/toolset/extension.hpp>

namespace {

using namespace turbo::filesystem::status;
using turbo::filesystem::path;

void handle_stat_error(const path&path, int error)
{
    switch (error)
    {
	case EACCES:
	{
	    throw permission_error(path.native());
	}
	case ELOOP:
	{
	    throw invalid_path("symbolic link loop encountered");
	}
	case ENAMETOOLONG:
	{
	    throw invalid_path("file name too long");
	}
	case ENOENT:
	{
	    throw invalid_path("path does not exist");
	}
	case ENOMEM:
	{
	    throw std::system_error(errno, std::system_category(), "error from stat system call");
	}
	case ENOTDIR:
	{
	    throw invalid_path("some component in path is not a directory");
	}
	case EBADF:
	case EFAULT:
	case EOVERFLOW:
	{
	    throw std::system_error(errno, std::system_category(), "error from stat system call");
	}
	default:
	{
	    throw std::system_error(errno, std::system_category(), "unknown error from stat system call");
	}
    }
}

} // anonymous namespace

namespace turbo {
namespace filesystem {
namespace status {

using turbo::filesystem::path;

bool exists(const path& arg)
{
    bool result = true;
    struct stat info;
    if (::stat(arg.c_str(), &info) != 0)
    {
	switch (errno)
	{
	    case ENOENT:
	    {
		result = false;
	    }
	    default:
	    {
		handle_stat_error(arg, errno);
	    }
	}
    }
    return result;
}

bool is_symlink(const path& arg)
{
    struct stat info;
    if (TURBO_UNLIKELY(::lstat(arg.c_str(), &info) != 0))
    {
	handle_stat_error(arg, errno);
    }
    return S_ISLNK(info.st_mode) != 0;
}

path read_symlink(const turbo::filesystem::path& arg)
{
    char buf[1024];
    buf[0] = '\0';
    ssize_t result = readlink(arg.c_str(), buf, sizeof(buf));
    if (TURBO_UNLIKELY(result == -1))
    {
	switch (errno)
	{
	    case EACCES:
	    {
		throw permission_error(arg.native());
	    }
	    case ELOOP:
	    {
		throw invalid_path("symbolic link loop encountered");
	    }
	    case ENAMETOOLONG:
	    {
		throw invalid_path("file name too long");
	    }
	    case ENOENT:
	    {
		throw invalid_path("path does not exist");
	    }
	    case ENOMEM:
	    {
		throw std::system_error(errno, std::system_category(), "error from stat system call");
	    }
	    case ENOTDIR:
	    {
		throw invalid_path("some component in path is not a directory");
	    }
	    case EFAULT:
	    case EINVAL:
	    case EIO:
	    {
		throw std::system_error(errno, std::system_category(), "error from stat system call");
	    }
	    default:
	    {
		throw std::system_error(errno, std::system_category(), "unknown error from stat system call");
	    }
	}
    }
    else
    {
	std::size_t count = static_cast<std::size_t>(result);
	if (count < (sizeof(buf) - 1))
	{
	    buf[count] = '\0';
	}
	else
	{
	    throw invalid_path("symlink value is too long");
	}
    }
    return path(buf);
}

} // namespace status
} // namespace filesystem
} // namespace turbo
