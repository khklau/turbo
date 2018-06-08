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

TEST(power_test, basic_power_of_2_ceil)
{
    EXPECT_EQ(2U, tma::power_of_2_ceil(0U)) << "0 did not ceil to 2";
    EXPECT_EQ(2U, tma::power_of_2_ceil(1U)) << "1 did not ceil to 2";
    EXPECT_EQ(2U, tma::power_of_2_ceil(2U)) << "1 did not ceil to 2";
    EXPECT_EQ(4U, tma::power_of_2_ceil(3U)) << "3 did not ceil to 4";
    EXPECT_EQ(4U, tma::power_of_2_ceil(4U)) << "4 did not ceil to 4";
    EXPECT_EQ(256U, tma::power_of_2_ceil(129U)) << "129 did not ceil to 256";
    EXPECT_EQ(256U, tma::power_of_2_ceil(255U)) << "255 did not ceil to 256";
    EXPECT_EQ(256U, tma::power_of_2_ceil(256U)) << "256 did not ceil to 256";
    EXPECT_EQ(512U, tma::power_of_2_ceil(257U)) << "257 did not ceil to 512";
    EXPECT_EQ(65536U, tma::power_of_2_ceil(32769U)) << "32769 did not ceil to 65536";
    EXPECT_EQ(65536U, tma::power_of_2_ceil(65535U)) << "65535 did not ceil to 65536";
    EXPECT_EQ(65536U, tma::power_of_2_ceil(65536U)) << "65536 did not ceil to 65536";
    EXPECT_EQ(131072U, tma::power_of_2_ceil(65537U)) << "65537 did not ceil to 131072";
    EXPECT_EQ(16777216U, tma::power_of_2_ceil(8388609U)) << "8388609 did not ceil to 16777216";
    EXPECT_EQ(16777216U, tma::power_of_2_ceil(16777215U)) << "16777215 did not ceil to 16777216";
    EXPECT_EQ(16777216U, tma::power_of_2_ceil(16777216U)) << "16777216 did not ceil to 16777216";
    EXPECT_EQ(4294967296U, tma::power_of_2_ceil(2147483649U)) << "2147483649 did not ceil to 4294967296";
    EXPECT_EQ(4294967296U, tma::power_of_2_ceil(4294967295U)) << "4294967295 did not ceil to 4294967296";
    EXPECT_EQ(4294967296U, tma::power_of_2_ceil(4294967296U)) << "4294967296 did not ceil to 4294967296";
}
