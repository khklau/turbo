#include <turbo/math/power.hpp>
#include <gtest/gtest.h>

namespace tma = turbo::math;

TEST(power_test, basic_power)
{
    static_assert(1U == tma::power<3, 0>::result, "Compile time pow failed");
    static_assert(3U == tma::power<3, 1>::result, "Compile time pow failed");
    static_assert(9U == tma::power<3, 2>::result, "Compile time pow failed");
    static_assert(1024U == tma::power<2, 10>::result, "Compile time pow failed");
}
