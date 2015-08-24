#ifndef TURBO_FILESYSTEM_PATH_HPP
#define TURBO_FILESYSTEM_PATH_HPP

#if defined(_WIN32)
#include <wstring>
#else
#include <string>
#endif
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace filesystem {

/// TODO: replace with std::filesystem::path when C++17 is available
class TURBO_SYMBOL_DECL path
{
public:
#if defined(_WIN32)
    typedef wchar_t value_type;
    static const value_type preferred_separator = L'\\';
#else
    typedef char value_type;
    static const value_type preferred_separator = '/';
#endif
    typedef std::basic_string<value_type>  string_type;
    path(const value_type* path);
    path(const string_type& path);
    path(const path& other);
    path& operator=(const path& other);
    path& operator/=(const string_type& path);
    path& operator/=(const value_type* path);
    path& operator/=(const path& path);
    const string_type& native() const { return pathname_; }
    const value_type* c_str() const { return pathname_.c_str(); }
    inline bool empty() const { return pathname_.empty(); }
    path parent_path() const;
private:
    string_type pathname_;
};

inline TURBO_SYMBOL_DECL path operator/(const path& lhs, const path& rhs)  { return path(lhs) /= rhs; }

} // namespace filesystem
} // namespace turbo

#endif
