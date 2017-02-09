#include <turbo/container/trie_key.hpp>
#include <gtest/gtest.h>

namespace tco = turbo::container;

TEST(uint_trie_key_test, empty_key)
{
    typedef tco::uint_trie_key<std::uint32_t, 2U> uint32_key;
    EXPECT_EQ(32U, uint32_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint32_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint32_key key1;
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_pushed_prefixes()))
	    << "get_pushed_prefixes succeeded on empty key";
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_popped_prefixes()))
	    << "get_popped_prefixes succeeded on empty key";
    EXPECT_TRUE(key1.is_empty()) << "is_empty returned false on empty key";
    EXPECT_FALSE(key1.is_full()) << "is_full returned false on empty key";
}

TEST(uint_trie_key_test, unused_key)
{
    typedef tco::uint_trie_key<std::uint32_t, 2U> uint32_key;
    EXPECT_EQ(32U, uint32_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint32_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint32_key key1(0xf0000000U);
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_pushed_prefixes()))
	    << "get_pushed_prefixes succeeded on an unused key";
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_popped_prefixes()))
	    << "get_popped_prefixes succeeded on an unused key";
    uint32_key key2(0x0000000fU);
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_pushed_prefixes()))
	    << "get_pushed_prefixes succeeded on an unused key";
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_popped_prefixes()))
	    << "get_popped_prefixes succeeded on an unused key";
}

TEST(uint_trie_key_test, push_basic)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    EXPECT_EQ(uint16_key::push_result::success, key1.push(8U)) << "push failed";
    auto result1 = key1.get_pushed_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result1)) << "get failed after push";
    EXPECT_EQ(32768U, std::get<1>(result1)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(8U)) << "push failed";
    auto result2 = key1.get_pushed_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result2)) << "get failed after push";
    EXPECT_EQ(32768U + 2048U, std::get<1>(result2)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(8U)) << "push failed";
    auto result3 = key1.get_pushed_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result3)) << "get failed after push";
    EXPECT_EQ(32768U + 2048U + 128U, std::get<1>(result3)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(8U)) << "push failed";
    auto result4 = key1.get_pushed_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result4)) << "get failed after push";
    EXPECT_EQ(32768U + 2048U + 128U + 8U, std::get<1>(result4)) << "push failed";
    uint16_key key2;
    EXPECT_EQ(uint16_key::push_result::success, key2.push(1U)) << "push failed";
    auto result5 = key2.get_pushed_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result5)) << "get failed after push";
    EXPECT_EQ(4096U, std::get<1>(result5)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key2.push(1U)) << "push failed";
    auto result6 = key2.get_pushed_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result6)) << "get failed after push";
    EXPECT_EQ(4096U + 256U, std::get<1>(result6)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key2.push(1U)) << "push failed";
    auto result7 = key2.get_pushed_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result7)) << "get failed after push";
    EXPECT_EQ(4096U + 256U + 16U, std::get<1>(result7)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key2.push(1U)) << "push failed";
    auto result8 = key2.get_pushed_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result8)) << "get failed after push";
    EXPECT_EQ(4096U + 256U + 16U + 1U, std::get<1>(result8)) << "push failed";
}
