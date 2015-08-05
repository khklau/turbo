#ifndef TURBO_ALGORITHM_STRING_HPP
#define TURBO_ALGORITHM_STRING_HPP

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace turbo {
namespace algorithm {
namespace string {

typedef std::function<bool(char)> match_predicate;

match_predicate is_any_of(const char* target);
match_predicate is_exactly(char target);
std::vector<std::string::const_iterator> find_all(const std::string& str, const match_predicate& match);

} // namespace string
} // namespace algorithm
} // namespace turbo

#endif
