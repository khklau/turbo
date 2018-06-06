#ifndef TURBO_TYPE_UTILITY_TYPE_SELECTION_HPP
#define TURBO_TYPE_UTILITY_TYPE_SELECTION_HPP

namespace turbo {
namespace type_utility {

template <bool predicate, typename first_t, typename second_t>
struct binary_select
{
    typename first_t result;
};

template <typename first_t, typename second_t>
struct binary_select<false, first_t, second_t>
{
    typename second_t result;
};

} // namespace type_utility
} // namespace turbo

#endif
