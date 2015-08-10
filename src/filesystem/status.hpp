#ifndef TURBO_FILESYSTEM_STATUS_HPP
#define TURBO_FILESYSTEM_STATUS_HPP

#include <stdexcept>
#include <turbo/filesystem/path.hpp>

namespace turbo {
namespace filesystem {
namespace status {

struct permission_error : public std::runtime_error
{
    permission_error(const std::string& what) : std::runtime_error(what) { }
    permission_error(const char* what) : std::runtime_error(what) { }
};

struct invalid_path : public std::runtime_error
{
    invalid_path(const std::string& what) : std::runtime_error(what) { }
    invalid_path(const char* what) : std::runtime_error(what) { }
};

bool exists(const turbo::filesystem::path& path);
bool is_symlink(const turbo::filesystem::path& path);
turbo::filesystem::path read_symlink(const turbo::filesystem::path& path);

} // namespace status
} // namespace filesystem
} // namespace turbo

#endif
