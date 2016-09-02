#include <turbo/math/summation.hpp>
#include <gtest/gtest.h>

namespace tma = turbo::math;

TEST(summation_test, basic_log_ceiling_in_series)
{
    EXPECT_EQ(0U, tma::log_ceiling_in_series(4U, 2U, 0U)) << "Wrong logarithm for sum of 0";
    EXPECT_EQ(1U, tma::log_ceiling_in_series(4U, 2U, 2U)) << "Wrong logarithm for sum that is less than base";
    EXPECT_EQ(1U, tma::log_ceiling_in_series(4U, 2U, 4U)) << "Wrong logarithm for sum equal to base";
    EXPECT_EQ(2U, tma::log_ceiling_in_series(4U, 2U, 5U)) << "Wrong logarithm for sum just large than base";
    EXPECT_EQ(2U, tma::log_ceiling_in_series(4U, 2U, 12U)) << "Wrong logarithm for sum of 2 exponents";
    EXPECT_EQ(3U, tma::log_ceiling_in_series(4U, 2U, 13U)) << "Wrong logarithm for sum just larger than base squared";
}
