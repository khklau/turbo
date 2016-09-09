#include <turbo/toolset/intrinsic.hpp>
#include <cstdint>
#include <limits>
#include <gtest/gtest.h>

namespace tto = turbo::toolset;

TEST(intrinsic_test, basic_uint32_count_leading_zero)
{
    for (std::uint8_t index = 0U; index < 32U; ++index)
    {
	ASSERT_EQ(std::numeric_limits<std::uint32_t>::digits - (index + 1U), tto::count_leading_zero(static_cast<std::uint32_t>(1U << index))) << "Incorrect count for 2 pow " << index;
    }
}

TEST(intrinsic_test, invalid_uint32_count_leading_zero)
{
    ASSERT_EQ(std::numeric_limits<std::uint32_t>::digits, tto::count_leading_zero(static_cast<std::uint32_t>(0U))) << "Incorrect count for 0";
}

TEST(intrinsic_test, basic_uint64_count_leading_zero)
{
    for (std::uint8_t index = 0U; index < 64U; ++index)
    {
	ASSERT_EQ(std::numeric_limits<std::uint64_t>::digits - (index + 1U), tto::count_leading_zero(static_cast<std::uint64_t>(1ULL << index))) << "Incorrect count for 2 pow " << index;
    }
}

TEST(intrinsic_test, invalid_uint64_count_leading_zero)
{
    ASSERT_EQ(std::numeric_limits<std::uint64_t>::digits, tto::count_leading_zero(static_cast<std::uint64_t>(0ULL))) << "Incorrect count for 0";
}
