#ifndef TURBO_TYPE_UTILITY_FUNCTION_TRAITS_HPP
#define TURBO_TYPE_UTILITY_FUNCTION_TRAITS_HPP

#include <tuple>

namespace turbo {
namespace type_utility {

template <typename func_t>
struct function_traits : function_traits<decltype(&func_t::operator())>
{ };

template <class result_t, class... args_t>
struct function_traits<result_t(args_t...)>
{
    using return_type = result_t;
    using arg_types = std::tuple<args_t...>;
};

template <class result_t, class... args_t>
struct function_traits<result_t(*)(args_t...)>
{
    using return_type = result_t;
    using arg_types = std::tuple<args_t...>;
};

template <class class_t, class result_t, class... args_t>
struct function_traits<result_t(class_t::*)(args_t...)>
{
    using return_type = result_t;
    using arg_types = std::tuple<args_t...>;
};

template <class class_t, class result_t, class... args_t>
struct function_traits<result_t(class_t::*)(args_t...) const>
{
    using return_type = result_t;
    using arg_types = std::tuple<args_t...>;
};

} // namespace type_utility
} // namespace turbo

#endif
