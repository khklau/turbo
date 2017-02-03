#include <turbo/container/trie_prefix.hpp>
#include <gtest/gtest.h>

namespace tco = turbo::container;

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
