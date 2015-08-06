#include <turbo/filesystem/path.hpp>
#include <stdexcept>
#include <turbo/toolset/extension.hpp>
#include <turbo/algorithm/string.hpp>

namespace {

using namespace turbo::filesystem;

static const path::value_type alt_separator = '/';
static const path::string_type dot(".");
static const path::string_type dotdot("..");

inline bool is_separator(path::value_type candidate)
{
    return candidate == ::alt_separator ||
	    candidate == path::preferred_separator;
}

inline bool has_trailing_separator(const path::string_type& arg)
{
    return !arg.empty() && ::is_separator(*(arg.end() - 1));
}

inline bool is_absolute(const path::string_type& arg)
{
    return ::is_separator(arg[0]);
}

inline bool is_relative(const path::string_type& arg)
{
    return !::is_separator(arg[0]);
}

inline bool is_root(const path::string_type& arg)
{
    return (arg.length() == 1 && is_absolute(arg));
}

void ensure_separator_at_end(path::string_type& arg)
{
    if (!arg.empty() && !::is_separator(*(arg.end() - 1)))
    {
	arg += path::preferred_separator;
    }
}

const char* ensure_valid_cstring(const char* str)
{
    if (str == nullptr)
    {
	throw std::invalid_argument("Null C string is not valid");
    }
    return str;
}

} // anonymous namespace

namespace turbo {
namespace filesystem {

using turbo::algorithm::string::is_exactly;
using turbo::algorithm::string::find_all;

path::path(const value_type* path) :
	pathname_(ensure_valid_cstring(path))
{ }

path::path(const string_type& path) :
	pathname_(path)
{ }

path::path(const path& other) :
	pathname_(other.pathname_)
{ }

path& path::operator=(const path& other)
{
    if (this != &other)
    {
	this->pathname_ = other.pathname_;
    }
    return *this;
}

path& path::operator/=(const string_type& arg)
{
    bool end_of_path_is_sep = ::has_trailing_separator(pathname_);
    bool start_of_arg_is_sep = ::is_absolute(arg);
    if (TURBO_UNLIKELY(arg.empty()))
    {
	return *this;
    }
    else if (TURBO_UNLIKELY(this->empty()))
    {
	pathname_ += arg;
    }
    else if (end_of_path_is_sep && start_of_arg_is_sep)
    {
	pathname_.append(arg.begin() + 1, arg.end());
    }
    else if (end_of_path_is_sep && !start_of_arg_is_sep)
    {
	pathname_ += arg;
    }
    else if (!end_of_path_is_sep && start_of_arg_is_sep)
    {
	pathname_ += arg;
    }
    else
    {
	ensure_separator_at_end(pathname_);
	pathname_ += arg;
    }
    return *this;
}

path& path::operator/=(const value_type* path)
{
    return *this /= std::string(ensure_valid_cstring(path));
}

path& path::operator/=(const path& path)
{
    return *this /= path.pathname_;
}

path path::parent_path() const
{
    std::vector<std::string::const_iterator>&& separators = find_all(pathname_, is_exactly(preferred_separator));
    bool is_absolute = ::is_absolute(pathname_);
    bool has_trailing_separator = ::has_trailing_separator(pathname_);
    if (is_absolute && has_trailing_separator && separators.size() <= 2)
    {
	return path(std::string(pathname_.begin(), pathname_.begin() + 1));
    }
    else if (is_absolute && !has_trailing_separator && separators.size() <= 1)
    {
	return path(std::string(pathname_.begin(), pathname_.begin() + 1));
    }
    else if (is_absolute && has_trailing_separator && separators.size() > 2)
    {
	return path(std::string(pathname_.begin(), *(separators.end() - 2)));
    }
    else if (is_absolute && !has_trailing_separator && separators.size() > 1)
    {
	return path(std::string(pathname_.begin(), *(separators.end() - 1)));
    }
    else if (!is_absolute && has_trailing_separator && separators.size() <= 1)
    {
	return path(dot);
    }
    else if (!is_absolute && !has_trailing_separator && separators.size() == 0)
    {
	return path(dot);
    }
    else if (!is_absolute && has_trailing_separator && separators.size() > 1)
    {
	return path(std::string(pathname_.begin(), *(separators.end() - 2)));
    }
    else if (!is_absolute && !has_trailing_separator && separators.size() > 0)
    {
	return path(std::string(pathname_.begin(), *(separators.end() - 1)));
    }
    else
    {
	return path("");
    }
}

} // namespace filesystem
} // namespace turbo
