#include <turbo/container/trie_prefix.hpp>
#include <random>
#include <gtest/gtest.h>

namespace tco = turbo::container;

/*
TEST(uint_trie_prefix_test, get_prefix)
{
    typedef tco::uint_trie_prefix<std::uint32_t, 2U> uint32_prefix;
    EXPECT_EQ(32U, uint32_prefix::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint32_prefix::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint32_prefix prefix1(0xf0000000U);
    EXPECT_EQ(1U, prefix1.get_next_prefix()) << "get_prefix failed when most significant bit is 1";
    uint32_prefix prefix2(0x0000000fU);
    EXPECT_EQ(0U, prefix2.get_next_prefix()) << "get_prefix failed when most significant bit is 1";
}
*/

TEST(uint_trie_prefix_perf_test, perf_test_read)
{
    typedef tco::uint_trie_prefix<std::uint16_t, 2U> uint16_prefix;
    std::random_device device;
    std::uint16_t output = 0U;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	uint16_prefix prefix(static_cast<std::uint16_t>(device() >> 16U));
	for (; prefix.get_usage_count() < uint16_prefix::max_usage(); prefix = prefix << 1U)
	{
	    output += prefix.get_next_prefix();
	}
    }
    EXPECT_NE(0U, output);
}

TEST(uint_trie_prefix_perf_test, perf_test_preceding)
{
    typedef tco::uint_trie_prefix<std::uint16_t, 2U> uint16_prefix;
    std::random_device device;
    std::uint16_t output = 0U;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	uint16_prefix prefix(static_cast<std::uint16_t>(device() >> 16U));
	for (; prefix.get_usage_count() < uint16_prefix::max_usage(); prefix = prefix << 1U)
	{
	    output += prefix.get_common_prefix();
	}
    }
    EXPECT_NE(0U, output);
}

TEST(uint_trie_prefix_perf_test, perf_test_count)
{
    typedef tco::uint_trie_prefix<std::uint16_t, 2U> uint16_prefix;
    std::random_device device;
    std::uint16_t output = 0U;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	uint16_prefix prefix(static_cast<std::uint16_t>(device() >> 16U));
	for (; prefix.get_usage_count() < uint16_prefix::max_usage(); prefix = prefix << 1U)
	{
	    output += prefix.get_usage_count();
	}
    }
    EXPECT_NE(1U, output);
}

TEST(uint_trie_prefix_perf_test, perf_test_shift)
{
    typedef tco::uint_trie_prefix<std::uint16_t, 2U> uint16_prefix;
    std::random_device device;
    std::uint16_t output = 0U;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	uint16_prefix prefix(static_cast<std::uint16_t>(device() >> 16U));
	while (prefix.get_usage_count() < uint16_prefix::max_usage())
	{
	    prefix = prefix << 2U;
	    ++output;
	}
    }
    EXPECT_NE(1U, output);
}
