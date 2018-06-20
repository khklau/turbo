#include <turbo/type_utility/function_traits.hpp>
#include <gtest/gtest.h>
#include <string>
#include <type_traits>

namespace ttu = turbo::type_utility;

std::string blah(int arg)
{
    return std::to_string(arg);
};

struct foo
{
    int bar(const std::string&) const
    {
	return 0;
    }
};

template <typename func_t>
using first_arg_type = typename std::tuple_element<0U, typename ttu::function_traits<func_t>::arg_types>::type;

TEST(function_traits_test, basic_first_arg_type)
{
    static_assert(std::is_same<int, first_arg_type<decltype(blah)>>::value,
	    "failed to detect first arg type for function");
    static_assert(std::is_same<int, first_arg_type<decltype(&blah)>>::value,
	    "failed to detect first arg type for function pointer");
    static_assert(std::is_same<const std::string&, first_arg_type<decltype(&foo::bar)>>::value,
	    "failed to detect first arg type for member function pointer");
    auto lambda = [](const std::string&) { };
    static_assert(std::is_same<const std::string&, first_arg_type<decltype(lambda)>>::value,
	    "failed to detect first arg type for lambda function");
}

TEST(function_traits_test, basic_return_type)
{
    static_assert(std::is_same<std::string, typename ttu::function_traits<decltype(blah)>::return_type>::value,
	    "failed to detect return type for function");
    static_assert(std::is_same<std::string, typename ttu::function_traits<decltype(&blah)>::return_type>::value,
	    "failed to detect return type for function pointer");
    static_assert(std::is_same<int, typename ttu::function_traits<decltype(&foo::bar)>::return_type>::value,
	    "failed to detect return type for member function pointer");
    auto lambda = [](const std::string&) -> void { };
    static_assert(std::is_same<void, typename ttu::function_traits<decltype(lambda)>::return_type>::value,
	    "failed to detect return type for lambda function");
}
