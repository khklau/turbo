#include <turbo/process/status.hpp>
#include <stdexcept>
#include <turbo/toolset/extension.hpp>
#include <turbo/filesystem/path.hpp>
#include <turbo/filesystem/status.hpp>

namespace tf = turbo::filesystem;

namespace turbo {
namespace process {
namespace status {

tf::path current_exe_path()
{
    tf::path proc_exe("/proc/self/exe");
    if (TURBO_UNLIKELY(!tf::status::exists(proc_exe)))
    {
	throw std::runtime_error("Invalid Linux system: /proc/self/exe does not exist");
    }
    if (TURBO_UNLIKELY(!tf::status::is_symlink(proc_exe)))
    {
	throw std::runtime_error("Invalid Linux system: /proc/self/exe is not a symbolic link");
    }
    return tf::status::read_symlink(proc_exe);
}

} // namespace status
} // namespace process
} // namespace turbo
