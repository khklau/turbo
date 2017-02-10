#include <turbo/container/trie_key.hpp>
#include <gtest/gtest.h>

namespace tco = turbo::container;

TEST(uint_trie_key_test, empty_key)
{
    typedef tco::uint_trie_key<std::uint32_t, 2U> uint32_key;
    EXPECT_EQ(32U, uint32_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint32_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint32_key key1;
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_written_prefixes()))
	    << "get_written_prefixes succeeded on empty key";
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_read_prefixes()))
	    << "get_read_prefixes succeeded on empty key";
    EXPECT_TRUE(key1.is_unavailable()) << "is_unavailable returned false on empty key";
    EXPECT_FALSE(key1.is_full()) << "is_full returned false on empty key";
}

TEST(uint_trie_key_test, predefined_key)
{
    typedef tco::uint_trie_key<std::uint32_t, 2U> uint32_key;
    EXPECT_EQ(32U, uint32_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint32_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint32_key key1(1U);
    EXPECT_EQ(uint32_key::get_result::success, std::get<0>(key1.get_written_prefixes()))
	    << "get_written_prefixes failed on predefined key";
    EXPECT_EQ(1U, std::get<1>(key1.get_written_prefixes()))
	    << "get_written_prefixes did not return the predefined key";
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_read_prefixes()))
	    << "get_read_prefixes succeeded on unread predefined key";
    EXPECT_FALSE(key1.is_unavailable()) << "is_unavailable returned true on predefined key";
    EXPECT_TRUE(key1.is_full()) << "is_full returned false on predefined key";
}

TEST(uint_trie_key_test, write_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    EXPECT_EQ(uint8_key::write_result::success, key1.write(8U)) << "write failed";
    EXPECT_EQ(uint8_key::write_result::key_full, key1.write(8U)) << "write succeeded on full key";
    uint8_key key2(255U);
    EXPECT_EQ(uint8_key::write_result::key_full, key2.write(8U)) << "write succeeded on predefined key";
}

TEST(uint_trie_key_test, write_basic)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(8U)) << "write failed";
    auto result1 = key1.get_written_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result1)) << "get failed after write";
    EXPECT_EQ(32768U, std::get<1>(result1)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(8U)) << "write failed";
    auto result2 = key1.get_written_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result2)) << "get failed after write";
    EXPECT_EQ(32768U + 2048U, std::get<1>(result2)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(8U)) << "write failed";
    auto result3 = key1.get_written_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result3)) << "get failed after write";
    EXPECT_EQ(32768U + 2048U + 128U, std::get<1>(result3)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(8U)) << "write failed";
    auto result4 = key1.get_written_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result4)) << "get failed after write";
    EXPECT_EQ(32768U + 2048U + 128U + 8U, std::get<1>(result4)) << "write failed";
    uint16_key key2;
    EXPECT_EQ(uint16_key::write_result::success, key2.write(1U)) << "write failed";
    auto result5 = key2.get_written_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result5)) << "get failed after write";
    EXPECT_EQ(4096U, std::get<1>(result5)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key2.write(1U)) << "write failed";
    auto result6 = key2.get_written_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result6)) << "get failed after write";
    EXPECT_EQ(4096U + 256U, std::get<1>(result6)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key2.write(1U)) << "write failed";
    auto result7 = key2.get_written_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result7)) << "get failed after write";
    EXPECT_EQ(4096U + 256U + 16U, std::get<1>(result7)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key2.write(1U)) << "write failed";
    auto result8 = key2.get_written_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result8)) << "get failed after write";
    EXPECT_EQ(4096U + 256U + 16U + 1U, std::get<1>(result8)) << "write failed";
}

TEST(uint_trie_key_test, read_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    EXPECT_EQ(uint8_key::read_result::prefix_unavailable, std::get<0>(key1.read())) << "read succeeded on empty key";
    uint8_key key2(255U);
    EXPECT_EQ(uint8_key::read_result::success, std::get<0>(key2.read())) << "read failed";
    EXPECT_EQ(uint8_key::read_result::prefix_unavailable, std::get<0>(key2.read())) << "read succeeded on empty key";
}

TEST(uint_trie_key_test, read_basic)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1(32768U + 2048U + 128U + 8U);
    auto result1a = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result1a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result1a)) << "read failed";
    auto result1b = key1.get_read_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result1b)) << "get failed after read";
    EXPECT_EQ(32768U, std::get<1>(result1b)) << "read failed";
    auto result2a = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result2a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result2a)) << "read failed";
    auto result2b = key1.get_read_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result2b)) << "get failed after read";
    EXPECT_EQ(32768U + 2048U, std::get<1>(result2b)) << "read failed";
    auto result3a = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result3a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result3a)) << "read failed";
    auto result3b = key1.get_read_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result3b)) << "get failed after read";
    EXPECT_EQ(32768U + 2048U + 128U, std::get<1>(result3b)) << "read failed";
    auto result4a = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result4a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result4a)) << "read failed";
    auto result4b = key1.get_read_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result4b)) << "get failed after read";
    EXPECT_EQ(32768U + 2048U + 128U + 8U, std::get<1>(result4b)) << "read failed";
    EXPECT_TRUE(key1.is_unavailable()) << "fully read key is not unavailable";
    uint16_key key2(4096U + 256U + 16U + 1U);
    auto result5a = key2.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result5a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result5a)) << "read failed";
    auto result5b = key2.get_read_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result5b)) << "get failed after read";
    EXPECT_EQ(4096U, std::get<1>(result5b)) << "read failed";
    auto result6a = key2.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result6a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result6a)) << "read failed";
    auto result6b = key2.get_read_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result6b)) << "get failed after read";
    EXPECT_EQ(4096U + 256U, std::get<1>(result6b)) << "read failed";
    auto result7a = key2.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result7a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result7a)) << "read failed";
    auto result7b = key2.get_read_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result7b)) << "get failed after read";
    EXPECT_EQ(4096U + 256U + 16U, std::get<1>(result7b)) << "read failed";
    auto result8a = key2.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result8a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result8a)) << "read failed";
    auto result8b = key2.get_read_prefixes();
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result8b)) << "get failed after read";
    EXPECT_EQ(4096U + 256U + 16U + 1U, std::get<1>(result8b)) << "read failed";
    EXPECT_TRUE(key2.is_unavailable()) << "fully read key is not unavailable";
}

TEST(uint_trie_key_test, write_read_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    EXPECT_EQ(uint8_key::write_result::success, key1.write(255U)) << "write failed on empty key";
    EXPECT_EQ(uint8_key::read_result::success, std::get<0>(key1.read())) << "read failed on filled key";
    EXPECT_EQ(uint8_key::write_result::key_full, key1.write(0U)) << "write on full key succeeded";
    EXPECT_EQ(uint8_key::read_result::prefix_unavailable, std::get<0>(key1.read())) << "read succeeded on empty key";
}

TEST(uint_trie_key_test, write_read_alternate)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(1U)) << "write failed on empty key";
    auto result1 = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result1)) << "read failed on written key";
    EXPECT_EQ(1U, std::get<1>(result1)) << "read failed on written key";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(2U)) << "write failed on empty key";
    auto result2 = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result2)) << "read failed on written key";
    EXPECT_EQ(2U, std::get<1>(result2)) << "read failed on written key";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(3U)) << "write failed on empty key";
    auto result3 = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result3)) << "read failed on written key";
    EXPECT_EQ(3U, std::get<1>(result3)) << "read failed on written key";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(4U)) << "write failed on empty key";
    auto result4 = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result4)) << "read failed on written key";
    EXPECT_EQ(4U, std::get<1>(result4)) << "read failed on written key";
    EXPECT_EQ(uint16_key::write_result::key_full, key1.write(0U)) << "write on full key succeeded";
    EXPECT_EQ(uint16_key::read_result::prefix_unavailable, std::get<0>(key1.read())) << "read succeeded on empty key";
}

TEST(uint_trie_key_test, write_then_read)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(4U)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(3U)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(2U)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(1U)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::key_full, key1.write(0U)) << "write on full key succeeded";
    auto result1 = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result1)) << "read failed on written key";
    EXPECT_EQ(4U, std::get<1>(result1)) << "read failed on written key";
    auto result2 = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result2)) << "read failed on written key";
    EXPECT_EQ(3U, std::get<1>(result2)) << "read failed on written key";
    auto result3 = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result3)) << "read failed on written key";
    EXPECT_EQ(2U, std::get<1>(result3)) << "read failed on written key";
    auto result4 = key1.read();
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result4)) << "read failed on written key";
    EXPECT_EQ(1U, std::get<1>(result4)) << "read failed on written key";
    EXPECT_EQ(uint16_key::read_result::prefix_unavailable, std::get<0>(key1.read())) << "read succeeded on empty key";
}
