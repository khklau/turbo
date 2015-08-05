#include "string.hpp"
#include <cstring>
#include <utility>

namespace {

using namespace turbo::algorithm::string;

bool is_any_of_impl_(const char* target, char candidate)
{
    return (::strchr(target, candidate) != nullptr);
}

bool is_exactly_impl_(char target, char candidate)
{
    return target == candidate;
}

} // anonymous namespace

namespace turbo {
namespace algorithm {
namespace string {

match_predicate is_any_of(const char* target)
{
    return match_predicate(std::bind(::is_any_of_impl_, target, std::placeholders::_1));
}

match_predicate is_exactly(char target)
{
    return match_predicate(std::bind(::is_exactly_impl_, target, std::placeholders::_1));
}

std::vector<std::string::const_iterator> find_all(const std::string& str, const match_predicate& match)
{
    std::vector<std::string::const_iterator> result;
    for (auto iter = str.cbegin(); iter != str.cend(); ++iter)
    {
	if (match(*iter))
	{
	    result.push_back(iter);
	}
    }
    return std::move(result);
}

} // namespace string
} // namespace algorithm
} // namespace turbo
