#include <turbo/math/summation.hpp>
#include <gtest/gtest.h>

namespace tma = turbo::math;

TEST(summation_test, basic_ceiling_bound)
{
    EXPECT_EQ(1U, tma::ceiling_bound(4U, 2U, 2U)) << "Wrong logarithm for sum that is less than base";
    EXPECT_EQ(1U, tma::ceiling_bound(4U, 2U, 4U)) << "Wrong logarithm for sum equal to base";
    EXPECT_EQ(2U, tma::ceiling_bound(4U, 2U, 5U)) << "Wrong logarithm for sum just large than base";
    EXPECT_EQ(2U, tma::ceiling_bound(4U, 2U, 12U)) << "Wrong logarithm for sum of 2 exponents";
    EXPECT_EQ(3U, tma::ceiling_bound(4U, 2U, 13U)) << "Wrong logarithm for sum just larger than base squared";
    EXPECT_EQ(3U, tma::ceiling_bound(4U, 2U, 28U)) << "Wrong logarithm for sum of 3 exponents";
}

TEST(summation_test, invalid_ceiling_bound)
{
    EXPECT_EQ(0U, tma::ceiling_bound(4U, 2U, 0U)) << "Wrong logarithm for sum of 0";
    ASSERT_THROW(tma::ceiling_bound(4U, 0U, 12U), tma::invalid_multiplier_error) << "Exception not thrown for invalid multiplier";
    ASSERT_THROW(tma::ceiling_bound(4U, 1U, 6U), tma::invalid_multiplier_error) << "Exception not thrown for invalid multiplier";
}
