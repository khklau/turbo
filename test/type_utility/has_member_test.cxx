#include <turbo/type_utility/has_member.hpp>
#include <gtest/gtest.h>

namespace tt = turbo::type_utility;

struct foo
{
    typedef int some_member;
};

struct bar
{ };

TEST(has_member_test, basic_member_type)
{
    static_assert(tt::has_mem_type<foo>::result == true, "member type check failed on valid type");
    static_assert(tt::has_mem_type<bar>::result == false, "member type check succeeded on invalid type");
}
