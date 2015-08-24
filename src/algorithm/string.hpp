#ifndef TURBO_ALGORITHM_STRING_HPP
#define TURBO_ALGORITHM_STRING_HPP

#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace algorithm {
namespace string {

typedef std::function<bool(char)> match_predicate;

TURBO_SYMBOL_DECL match_predicate is_any_of(const char* target);
TURBO_SYMBOL_DECL match_predicate is_exactly(char target);
TURBO_SYMBOL_DECL std::vector<std::string::const_iterator> find_all(const std::string& str, const match_predicate& match);

} // namespace string
} // namespace algorithm
} // namespace turbo

#endif
