#include <turbo/container/trie_key.hpp>
#include <gtest/gtest.h>

namespace tco = turbo::container;

TEST(uint_trie_key_test, unavailable_key)
{
    typedef tco::uint_trie_key<std::uint32_t, 2U> uint32_key;
    EXPECT_EQ(32U, uint32_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint32_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint32_key key1;
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_pushed_prefixes()))
	    << "get_pushed_prefixes succeeded on unavailable key";
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_popped_prefixes()))
	    << "get_popped_prefixes succeeded on unavailable key";
    EXPECT_TRUE(key1.is_unavailable()) << "is_unavailable returned false on unavailable key";
    EXPECT_FALSE(key1.is_full()) << "is_full returned false on unavailable key";
}

TEST(uint_trie_key_test, predefined_key)
{
    typedef tco::uint_trie_key<std::uint32_t, 2U> uint32_key;
    EXPECT_EQ(32U, uint32_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint32_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint32_key key1(1U);
    EXPECT_EQ(uint32_key::get_result::success, std::get<0>(key1.get_pushed_prefixes()))
	    << "get_pushed_prefixes failed on predefined key";
    EXPECT_EQ(1U, std::get<1>(key1.get_pushed_prefixes()))
	    << "get_pushed_prefixes did not return the predefined key";
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_popped_prefixes()))
	    << "get_popped_prefixes succeeded on unpopped predefined key";
    EXPECT_FALSE(key1.is_unavailable()) << "is_unavailable returned true on predefined key";
    EXPECT_TRUE(key1.is_full()) << "is_full returned false on predefined key";
}

TEST(uint_trie_key_test, push_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    EXPECT_EQ(uint8_key::push_result::success, key1.push(8U)) << "push failed";
    EXPECT_EQ(uint8_key::push_result::key_full, key1.push(8U)) << "push succeeded on full key";
    uint8_key key2(255U);
    EXPECT_EQ(uint8_key::push_result::key_full, key2.push(8U)) << "push succeeded on predefined key";
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

TEST(uint_trie_key_test, pop_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    EXPECT_EQ(uint8_key::pop_result::prefix_unavailable, std::get<0>(key1.pop())) << "pop succeeded on unavailable key";
    uint8_key key2(255U);
    EXPECT_EQ(uint8_key::pop_result::success, std::get<0>(key2.pop())) << "pop failed";
    EXPECT_EQ(uint8_key::pop_result::prefix_unavailable, std::get<0>(key2.pop())) << "pop succeeded on unavailable key";
}

TEST(uint_trie_key_test, pop_basic)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1(32768U + 2048U + 128U + 8U);
    auto result1a = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result1a)) << "pop failed";
    EXPECT_EQ(8U, std::get<1>(result1a)) << "pop failed";
    auto result1b = key1.get_popped_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result1b)) << "get failed after pop";
    EXPECT_EQ(32768U, std::get<1>(result1b)) << "pop failed";
    auto result2a = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result2a)) << "pop failed";
    EXPECT_EQ(8U, std::get<1>(result2a)) << "pop failed";
    auto result2b = key1.get_popped_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result2b)) << "get failed after pop";
    EXPECT_EQ(32768U + 2048U, std::get<1>(result2b)) << "pop failed";
    auto result3a = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result3a)) << "pop failed";
    EXPECT_EQ(8U, std::get<1>(result3a)) << "pop failed";
    auto result3b = key1.get_popped_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result3b)) << "get failed after pop";
    EXPECT_EQ(32768U + 2048U + 128U, std::get<1>(result3b)) << "pop failed";
    auto result4a = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result4a)) << "pop failed";
    EXPECT_EQ(8U, std::get<1>(result4a)) << "pop failed";
    auto result4b = key1.get_popped_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result4b)) << "get failed after pop";
    EXPECT_EQ(32768U + 2048U + 128U + 8U, std::get<1>(result4b)) << "pop failed";
    EXPECT_TRUE(key1.is_unavailable()) << "fully popped key is not unavailable";
    uint16_key key2(4096U + 256U + 16U + 1U);
    auto result5a = key2.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result5a)) << "pop failed";
    EXPECT_EQ(1U, std::get<1>(result5a)) << "pop failed";
    auto result5b = key2.get_popped_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result5b)) << "get failed after pop";
    EXPECT_EQ(4096U, std::get<1>(result5b)) << "pop failed";
    auto result6a = key2.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result6a)) << "pop failed";
    EXPECT_EQ(1U, std::get<1>(result6a)) << "pop failed";
    auto result6b = key2.get_popped_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result6b)) << "get failed after pop";
    EXPECT_EQ(4096U + 256U, std::get<1>(result6b)) << "pop failed";
    auto result7a = key2.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result7a)) << "pop failed";
    EXPECT_EQ(1U, std::get<1>(result7a)) << "pop failed";
    auto result7b = key2.get_popped_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result7b)) << "get failed after pop";
    EXPECT_EQ(4096U + 256U + 16U, std::get<1>(result7b)) << "pop failed";
    auto result8a = key2.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result8a)) << "pop failed";
    EXPECT_EQ(1U, std::get<1>(result8a)) << "pop failed";
    auto result8b = key2.get_popped_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result8b)) << "get failed after pop";
    EXPECT_EQ(4096U + 256U + 16U + 1U, std::get<1>(result8b)) << "pop failed";
    EXPECT_TRUE(key2.is_unavailable()) << "fully popped key is not unavailable";
}

TEST(uint_trie_key_test, push_pop_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    EXPECT_EQ(uint8_key::push_result::success, key1.push(255U)) << "push failed on unavailable key";
    EXPECT_EQ(uint8_key::pop_result::success, std::get<0>(key1.pop())) << "pop failed on filled key";
    EXPECT_EQ(uint8_key::push_result::key_full, key1.push(0U)) << "push on full key succeeded";
    EXPECT_EQ(uint8_key::pop_result::prefix_unavailable, std::get<0>(key1.pop())) << "pop succeeded on unavailable key";
}

TEST(uint_trie_key_test, push_pop_alternate)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    EXPECT_EQ(uint16_key::push_result::success, key1.push(1U)) << "push failed on empty key";
    auto result1 = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result1)) << "pop failed on pushed key";
    EXPECT_EQ(1U, std::get<1>(result1)) << "pop failed on pushed key";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(2U)) << "push failed on empty key";
    auto result2 = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result2)) << "pop failed on pushed key";
    EXPECT_EQ(2U, std::get<1>(result2)) << "pop failed on pushed key";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(3U)) << "push failed on empty key";
    auto result3 = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result3)) << "pop failed on pushed key";
    EXPECT_EQ(3U, std::get<1>(result3)) << "pop failed on pushed key";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(4U)) << "push failed on empty key";
    auto result4 = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result4)) << "pop failed on pushed key";
    EXPECT_EQ(4U, std::get<1>(result4)) << "pop failed on pushed key";
    EXPECT_EQ(uint16_key::push_result::key_full, key1.push(0U)) << "push on full key succeeded";
    EXPECT_EQ(uint16_key::pop_result::prefix_unavailable, std::get<0>(key1.pop())) << "pop succeeded on unavailable key";
}

TEST(uint_trie_key_test, push_then_pop)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    EXPECT_EQ(uint16_key::push_result::success, key1.push(4U)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(3U)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(2U)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::success, key1.push(1U)) << "push failed";
    EXPECT_EQ(uint16_key::push_result::key_full, key1.push(0U)) << "push on full key succeeded";
    auto result1 = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result1)) << "pop failed on pushed key";
    EXPECT_EQ(4U, std::get<1>(result1)) << "pop failed on pushed key";
    auto result2 = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result2)) << "pop failed on pushed key";
    EXPECT_EQ(3U, std::get<1>(result2)) << "pop failed on pushed key";
    auto result3 = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result3)) << "pop failed on pushed key";
    EXPECT_EQ(2U, std::get<1>(result3)) << "pop failed on pushed key";
    auto result4 = key1.pop();
    EXPECT_EQ(uint16_key::pop_result::success, std::get<0>(result4)) << "pop failed on pushed key";
    EXPECT_EQ(1U, std::get<1>(result4)) << "pop failed on pushed key";
    EXPECT_EQ(uint16_key::pop_result::prefix_unavailable, std::get<0>(key1.pop())) << "pop succeeded on unavailable key";
}
