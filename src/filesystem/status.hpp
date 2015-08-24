#ifndef TURBO_FILESYSTEM_STATUS_HPP
#define TURBO_FILESYSTEM_STATUS_HPP

#include <stdexcept>
#include <turbo/filesystem/path.hpp>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace filesystem {
namespace status {

struct TURBO_SYMBOL_DECL permission_error : public std::runtime_error
{
    permission_error(const std::string& what) : std::runtime_error(what) { }
    permission_error(const char* what) : std::runtime_error(what) { }
};

struct TURBO_SYMBOL_DECL invalid_path : public std::runtime_error
{
    invalid_path(const std::string& what) : std::runtime_error(what) { }
    invalid_path(const char* what) : std::runtime_error(what) { }
};

TURBO_SYMBOL_DECL bool exists(const turbo::filesystem::path& path);
TURBO_SYMBOL_DECL bool is_symlink(const turbo::filesystem::path& path);
TURBO_SYMBOL_DECL turbo::filesystem::path read_symlink(const turbo::filesystem::path& path);

} // namespace status
} // namespace filesystem
} // namespace turbo

#endif
