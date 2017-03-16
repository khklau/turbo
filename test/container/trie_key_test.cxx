#include <turbo/container/trie_key.hpp>
#include <random>
#include <gtest/gtest.h>

namespace turbo {
namespace container {

template <class uint_t, std::size_t radix>
class uint_trie_key_tester
{
public:
    typedef uint_trie_key<uint_t, radix> key_type;
    uint_trie_key_tester(key_type& key)
	:
	    key_(key)
    { }
    inline static typename key_type::uint_type align(typename key_type::uint_type prefix, typename key_type::iterator iter)
    {
	return key_type::align(prefix, iter);
    }
    inline static typename key_type::uint_type clear_prefix(typename key_type::uint_type key, typename key_type::iterator iter)
    {
	return key_type::clear_prefix(key, iter);
    }
private:
    key_type& key_;
};


} // namespace container
} // namespace turbo

namespace tco = turbo::container;

TEST(uint_trie_key_test, empty_key)
{
    typedef tco::uint_trie_key<std::uint32_t, 2U> uint32_key;
    EXPECT_EQ(32U, uint32_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint32_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint32_key key1;
    EXPECT_TRUE(key1.begin().is_valid()) << "begin iterator is not valid";
    EXPECT_FALSE(key1.end().is_valid()) << "end iterator is valid";
    EXPECT_NE(key1.begin(), key1.end()) << "begin and end iterators are equal";
    auto iter1 = key1.begin();
    EXPECT_EQ(uint32_key::get_result::unavailable, std::get<0>(key1.get_preceding_prefixes(iter1))) << "empty key is not 0";
    ++iter1;
    for (; iter1 != key1.end(); ++iter1)
    {
	EXPECT_EQ(uint32_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "empty key is not 0";
	EXPECT_EQ(0U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "empty key is not 0";
    }
}

TEST(uint_trie_key_test, predefined_key)
{
    typedef tco::uint_trie_key<std::uint8_t, 2U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1(std::numeric_limits<std::uint8_t>::max());
    EXPECT_TRUE(key1.begin().is_valid()) << "begin iterator is not valid";
    EXPECT_FALSE(key1.end().is_valid()) << "end iterator is valid";
    EXPECT_NE(key1.begin(), key1.end()) << "begin and end iterators are equal";
    auto iter1 = key1.begin();
    EXPECT_EQ(uint8_key::get_result::unavailable, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    ++iter1;
    EXPECT_EQ(uint8_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    EXPECT_EQ(128U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    ++iter1;
    EXPECT_EQ(uint8_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    EXPECT_EQ(128U + 64U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    ++iter1;
    EXPECT_EQ(uint8_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    EXPECT_EQ(128U + 64U + 32U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    ++iter1;
    EXPECT_EQ(uint8_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    EXPECT_EQ(128U + 64U + 32U + 16U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    ++iter1;
    EXPECT_EQ(uint8_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    EXPECT_EQ(128U + 64U + 32U + 16U + 8U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    ++iter1;
    EXPECT_EQ(uint8_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    EXPECT_EQ(128U + 64U + 32U + 16U + 8U + 4U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    ++iter1;
    EXPECT_EQ(uint8_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    EXPECT_EQ(128U + 64U + 32U + 16U + 8U + 4U + 2U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    ++iter1;
    EXPECT_EQ(uint8_key::get_result::success, std::get<0>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
    EXPECT_EQ(128U + 64U + 32U + 16U + 8U + 4U + 2U + 1U, std::get<1>(key1.get_preceding_prefixes(iter1))) << "prefix does not match predefined key";
}

TEST(uint_trie_key_test, write_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    auto iter1 = key1.begin();
    EXPECT_EQ(uint8_key::write_result::success, key1.write(iter1, 255U)) << "write failed";
    ++iter1;
    EXPECT_EQ(uint8_key::write_result::out_of_bounds, key1.write(iter1, 255U)) << "write succeeded on full key";
    uint8_key key2(255U);
    auto iter2 = key2.begin();
    EXPECT_EQ(uint8_key::write_result::success, key2.write(iter2, 0U)) << "write failed";
    ++iter2;
    EXPECT_EQ(uint8_key::write_result::out_of_bounds, key2.write(iter2, 0U)) << "write succeeded on predefined key";
    uint8_key key3(15U);
    auto iter3 = key3.begin();
    iter3 += 2;
    EXPECT_EQ(uint8_key::write_result::out_of_bounds, key3.write(iter3, 0U)) << "write succeeded with invalid iterator";
    EXPECT_EQ(uint8_key::write_result::out_of_bounds, key3.write(key3.end(), 0U)) << "write succeeded with end iterator";
    EXPECT_FALSE(iter3.is_valid()) << "is_invalid returned false for invalid iterator";
    EXPECT_EQ(key3.end(), iter3) << "invalid and end iterators are not equal";
}

TEST(uint_trie_key_test, align_basic)
{
    typedef tco::uint_trie_key<std::uint8_t, 2U> uint8_key;
    typedef tco::uint_trie_key_tester<std::uint8_t, 2U> uint8_tester;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    auto iter1 = key1.begin();
    EXPECT_EQ(128U, uint8_tester::align(1U, iter1)) << "align failed for 1st iterator position";
    ++iter1;
    EXPECT_EQ(64U, uint8_tester::align(1U, iter1)) << "align failed for 2nd iterator position";
    ++iter1;
    EXPECT_EQ(32U, uint8_tester::align(1U, iter1)) << "align failed for 3rd iterator position";
    ++iter1;
    EXPECT_EQ(16U, uint8_tester::align(1U, iter1)) << "align failed for 4th iterator position";
    ++iter1;
    EXPECT_EQ(8U, uint8_tester::align(1U, iter1)) << "align failed for 5th iterator position";
    ++iter1;
    EXPECT_EQ(4U, uint8_tester::align(1U, iter1)) << "align failed for 6th iterator position";
    ++iter1;
    EXPECT_EQ(2U, uint8_tester::align(1U, iter1)) << "align failed for 7th iterator position";
    ++iter1;
    EXPECT_EQ(1U, uint8_tester::align(1U, iter1)) << "align failed for 8th iterator position";
}

TEST(uint_trie_key_test, clear_prefix_basic)
{
    typedef tco::uint_trie_key<std::uint8_t, 2U> uint8_key;
    typedef tco::uint_trie_key_tester<std::uint8_t, 2U> uint8_tester;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(1U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1(255U);
    auto iter1 = key1.begin();
    EXPECT_EQ(255U - 128U, uint8_tester::clear_prefix(255U, iter1)) << "clear_prefix failed for 1st iterator position";
    ++iter1;
    EXPECT_EQ(255U - 64U, uint8_tester::clear_prefix(255U, iter1)) << "clear_prefix failed for 2nd iterator position";
    ++iter1;
    EXPECT_EQ(255U - 32U, uint8_tester::clear_prefix(255U, iter1)) << "clear_prefix failed for 3rd iterator position";
    ++iter1;
    EXPECT_EQ(255U - 16U, uint8_tester::clear_prefix(255U, iter1)) << "clear_prefix failed for 4th iterator position";
    ++iter1;
    EXPECT_EQ(255U - 8U, uint8_tester::clear_prefix(255U, iter1)) << "clear_prefix failed for 5th iterator position";
    ++iter1;
    EXPECT_EQ(255U - 4U, uint8_tester::clear_prefix(255U, iter1)) << "clear_prefix failed for 6th iterator position";
    ++iter1;
    EXPECT_EQ(255U - 2U, uint8_tester::clear_prefix(255U, iter1)) << "clear_prefix failed for 7th iterator position";
    ++iter1;
    EXPECT_EQ(255U - 1U, uint8_tester::clear_prefix(255U, iter1)) << "clear_prefix failed for 8th iterator position";
}

TEST(uint_trie_key_test, write_basic)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    auto iter1 = key1.begin();
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 8U)) << "write failed";
    ++iter1;
    auto result1 = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result1)) << "get failed after write";
    EXPECT_EQ(32768U, std::get<1>(result1)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 8U)) << "write failed";
    ++iter1;
    auto result2 = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result2)) << "get failed after write";
    EXPECT_EQ(32768U + 2048U, std::get<1>(result2)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 8U)) << "write failed";
    ++iter1;
    auto result3 = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result3)) << "get failed after write";
    EXPECT_EQ(32768U + 2048U + 128U, std::get<1>(result3)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 8U)) << "write failed";
    ++iter1;
    auto result4 = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result4)) << "get failed after write";
    EXPECT_EQ(32768U + 2048U + 128U + 8U, std::get<1>(result4)) << "write failed";
    uint16_key key2;
    auto iter2 = key2.begin();
    EXPECT_EQ(uint16_key::write_result::success, key2.write(iter2, 1U)) << "write failed";
    ++iter2;
    auto result5 = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result5)) << "get failed after write";
    EXPECT_EQ(4096U, std::get<1>(result5)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key2.write(iter2, 1U)) << "write failed";
    ++iter2;
    auto result6 = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result6)) << "get failed after write";
    EXPECT_EQ(4096U + 256U, std::get<1>(result6)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key2.write(iter2, 1U)) << "write failed";
    ++iter2;
    auto result7 = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result7)) << "get failed after write";
    EXPECT_EQ(4096U + 256U + 16U, std::get<1>(result7)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key2.write(iter2, 1U)) << "write failed";
    ++iter2;
    auto result8 = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result8)) << "get failed after write";
    EXPECT_EQ(4096U + 256U + 16U + 1U, std::get<1>(result8)) << "write failed";
}

TEST(uint_trie_key_test, write_jump)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    auto iter1 = key1.begin();
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 8U)) << "write failed";
    iter1 += 2;
    auto result1 = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result1)) << "get failed after write";
    EXPECT_EQ(32768U, std::get<1>(result1)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 8U)) << "write failed";
    iter1 += 2;
    auto result2 = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result2)) << "get failed after write";
    EXPECT_EQ(32768U + 128U, std::get<1>(result2)) << "write failed";
    uint16_key key2;
    auto iter2 = key2.begin();
    EXPECT_EQ(uint16_key::write_result::success, key2.write(iter2, 1U)) << "write failed";
    iter2 += 2;
    auto result5 = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result5)) << "get failed after write";
    EXPECT_EQ(4096U, std::get<1>(result5)) << "write failed";
    EXPECT_EQ(uint16_key::write_result::success, key2.write(iter2, 1U)) << "write failed";
    iter2 += 2;
    auto result6 = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result6)) << "get failed after write";
    EXPECT_EQ(4096U + 16U, std::get<1>(result6)) << "write failed";
}

TEST(uint_trie_key_test, read_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    auto iter1 = key1.begin();
    EXPECT_EQ(uint8_key::read_result::success, std::get<0>(key1.read(iter1))) << "read failed";
    ++iter1;
    EXPECT_EQ(uint8_key::read_result::out_of_bounds, std::get<0>(key1.read(iter1))) << "read succeeded on empty key";
    uint8_key key2(255U);
    auto iter2 = key2.begin();
    EXPECT_EQ(uint8_key::read_result::success, std::get<0>(key2.read(iter2))) << "read failed";
    ++iter2;
    EXPECT_EQ(uint8_key::read_result::out_of_bounds, std::get<0>(key2.read(iter2))) << "read succeeded on empty key";
    uint8_key key3(15U);
    auto iter3 = key3.begin();
    iter3 += 2;
    EXPECT_EQ(uint8_key::read_result::out_of_bounds, std::get<0>(key3.read(iter3))) << "read succeeded with invalid iterator";
    EXPECT_EQ(uint8_key::read_result::out_of_bounds, std::get<0>(key3.read(key3.end()))) << "read succeeded on empty key";
    EXPECT_FALSE(iter3.is_valid()) << "is_invalid returned false for invalid iterator";
    EXPECT_EQ(key3.end(), iter3) << "invalid and end iterators are not equal";
}

TEST(uint_trie_key_test, read_basic)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1(32768U + 2048U + 128U + 8U);
    auto iter1 = key1.begin();
    auto result1a = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result1a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result1a)) << "read failed";
    ++iter1;
    auto result1b = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result1b)) << "get failed after read";
    EXPECT_EQ(32768U, std::get<1>(result1b)) << "read failed";
    auto result2a = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result2a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result2a)) << "read failed";
    ++iter1;
    auto result2b = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result2b)) << "get failed after read";
    EXPECT_EQ(32768U + 2048U, std::get<1>(result2b)) << "read failed";
    auto result3a = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result3a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result3a)) << "read failed";
    ++iter1;
    auto result3b = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result3b)) << "get failed after read";
    EXPECT_EQ(32768U + 2048U + 128U, std::get<1>(result3b)) << "read failed";
    auto result4a = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result4a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result4a)) << "read failed";
    ++iter1;
    auto result4b = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result4b)) << "get failed after read";
    EXPECT_EQ(32768U + 2048U + 128U + 8U, std::get<1>(result4b)) << "read failed";
    uint16_key key2(4096U + 256U + 16U + 1U);
    auto iter2 = key2.begin();
    auto result5a = key2.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result5a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result5a)) << "read failed";
    ++iter2;
    auto result5b = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result5b)) << "get failed after read";
    EXPECT_EQ(4096U, std::get<1>(result5b)) << "read failed";
    auto result6a = key2.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result6a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result6a)) << "read failed";
    ++iter2;
    auto result6b = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result6b)) << "get failed after read";
    EXPECT_EQ(4096U + 256U, std::get<1>(result6b)) << "read failed";
    auto result7a = key2.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result7a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result7a)) << "read failed";
    ++iter2;
    auto result7b = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result7b)) << "get failed after read";
    EXPECT_EQ(4096U + 256U + 16U, std::get<1>(result7b)) << "read failed";
    auto result8a = key2.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result8a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result8a)) << "read failed";
    ++iter2;
    auto result8b = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result8b)) << "get failed after read";
    EXPECT_EQ(4096U + 256U + 16U + 1U, std::get<1>(result8b)) << "read failed";
}

TEST(uint_trie_key_test, read_jump)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1(32768U + 2048U + 128U + 8U);
    auto iter1 = key1.begin();
    auto result1a = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result1a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result1a)) << "read failed";
    iter1 += 2;
    auto result1b = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result1b)) << "get failed after read";
    EXPECT_EQ(32768U + 2048U, std::get<1>(result1b)) << "read failed";
    auto result2a = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result2a)) << "read failed";
    EXPECT_EQ(8U, std::get<1>(result2a)) << "read failed";
    iter1 += 2;
    auto result2b = key1.get_preceding_prefixes(iter1);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result2b)) << "get failed after read";
    EXPECT_EQ(32768U + 2048U + 128U + 8U, std::get<1>(result2b)) << "read failed";
    uint16_key key2(4096U + 256U + 16U + 1U);
    auto iter2 = key2.begin();
    auto result5a = key2.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result5a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result5a)) << "read failed";
    iter2 += 2;
    auto result5b = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result5b)) << "get failed after read";
    EXPECT_EQ(4096U + 256U, std::get<1>(result5b)) << "read failed";
    auto result6a = key2.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result6a)) << "read failed";
    EXPECT_EQ(1U, std::get<1>(result6a)) << "read failed";
    iter2 += 2;
    auto result6b = key2.get_preceding_prefixes(iter2);
    EXPECT_EQ(uint16_key::get_result::success, std::get<0>(result6b)) << "get failed after read";
    EXPECT_EQ(4096U + 256U + 16U + 1U, std::get<1>(result6b)) << "read failed";
}

TEST(uint_trie_key_test, write_read_invalid)
{
    typedef tco::uint_trie_key<std::uint8_t, 256U> uint8_key;
    EXPECT_EQ(8U, uint8_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(8U, uint8_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint8_key key1;
    auto iter1 = key1.begin();
    EXPECT_EQ(uint8_key::write_result::success, key1.write(iter1, 255U)) << "write failed on empty key";
    EXPECT_EQ(uint8_key::read_result::success, std::get<0>(key1.read(iter1))) << "read failed on filled key";
    ++iter1;
    EXPECT_EQ(uint8_key::write_result::out_of_bounds, key1.write(iter1, 0U)) << "write on full key succeeded";
    EXPECT_EQ(uint8_key::read_result::out_of_bounds, std::get<0>(key1.read(iter1))) << "read succeeded on empty key";
}

TEST(uint_trie_key_test, write_read_alternate)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    auto iter1 = key1.begin();
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 1U)) << "write failed on empty key";
    auto result1 = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result1)) << "read failed on written key";
    EXPECT_EQ(1U, std::get<1>(result1)) << "read failed on written key";
    ++iter1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 2U)) << "write failed on empty key";
    auto result2 = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result2)) << "read failed on written key";
    EXPECT_EQ(2U, std::get<1>(result2)) << "read failed on written key";
    ++iter1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 3U)) << "write failed on empty key";
    auto result3 = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result3)) << "read failed on written key";
    EXPECT_EQ(3U, std::get<1>(result3)) << "read failed on written key";
    ++iter1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 4U)) << "write failed on empty key";
    auto result4 = key1.read(iter1);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result4)) << "read failed on written key";
    EXPECT_EQ(4U, std::get<1>(result4)) << "read failed on written key";
    ++iter1;
    EXPECT_EQ(uint16_key::write_result::out_of_bounds, key1.write(iter1, 0U)) << "write on full key succeeded";
    EXPECT_EQ(uint16_key::read_result::out_of_bounds, std::get<0>(key1.read(iter1))) << "read succeeded on empty key";
}

TEST(uint_trie_key_test, write_then_read)
{
    typedef tco::uint_trie_key<std::uint16_t, 16U> uint16_key;
    EXPECT_EQ(16U, uint16_key::key_bit_size()) << "key_bit_size calculation is wrong";
    EXPECT_EQ(4U, uint16_key::radix_bit_size()) << "radix_bit_size calculation is wrong";
    uint16_key key1;
    auto iter1 = key1.begin();
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 4U)) << "write failed";
    ++iter1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 3U)) << "write failed";
    ++iter1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 2U)) << "write failed";
    ++iter1;
    EXPECT_EQ(uint16_key::write_result::success, key1.write(iter1, 1U)) << "write failed";
    ++iter1;
    EXPECT_EQ(uint16_key::write_result::out_of_bounds, key1.write(iter1, 0U)) << "write on full key succeeded";
    auto iter2 = key1.begin();
    auto result1 = key1.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result1)) << "read failed on written key";
    EXPECT_EQ(4U, std::get<1>(result1)) << "read failed on written key";
    ++iter2;
    auto result2 = key1.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result2)) << "read failed on written key";
    EXPECT_EQ(3U, std::get<1>(result2)) << "read failed on written key";
    ++iter2;
    auto result3 = key1.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result3)) << "read failed on written key";
    EXPECT_EQ(2U, std::get<1>(result3)) << "read failed on written key";
    ++iter2;
    auto result4 = key1.read(iter2);
    EXPECT_EQ(uint16_key::read_result::success, std::get<0>(result4)) << "read failed on written key";
    EXPECT_EQ(1U, std::get<1>(result4)) << "read failed on written key";
    ++iter2;
    EXPECT_EQ(uint16_key::read_result::out_of_bounds, std::get<0>(key1.read(iter2))) << "read succeeded on empty key";
}

TEST(uint_trie_key_perf_test, perf_test_read)
{
    typedef tco::uint_trie_key<std::uint16_t, 2U> uint16_key;
    std::random_device device;
    std::uint16_t output = 0U;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	uint16_key key(static_cast<std::uint16_t>(device() >> 16U));
	for (auto iter = key.begin(); iter != key.end(); ++iter)
	{
	    output += std::get<1>(key.read(iter));
	}
    }
    EXPECT_NE(0U, output);
}

TEST(uint_trie_key_perf_test, perf_test_preceding)
{
    typedef tco::uint_trie_key<std::uint16_t, 2U> uint16_key;
    std::random_device device;
    std::uint16_t output = 0U;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	uint16_key key(static_cast<std::uint16_t>(device() >> 16U));
	for (auto iter = key.begin(); iter != key.end(); ++iter)
	{
	    auto result = key.get_preceding_prefixes(iter);
	    if (std::get<0>(result) == uint16_key::get_result::success)
	    {
		output += std::get<1>(result);
	    }
	}
    }
    EXPECT_NE(0U, output);
}

TEST(uint_trie_key_perf_test, perf_test_count)
{
    typedef tco::uint_trie_key<std::uint16_t, 2U> uint16_key;
    std::random_device device;
    std::uint16_t output = 0U;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	uint16_key key(static_cast<std::uint16_t>(device() >> 16U));
	for (auto iter = key.begin(); iter != key.end(); ++iter)
	{
	    output += iter.get_index();
	}
    }
    EXPECT_NE(1U, output);
}

TEST(uint_trie_key_perf_test, perf_test_shift)
{
    typedef tco::uint_trie_key<std::uint16_t, 2U> uint16_key;
    std::random_device device;
    std::uint16_t output = 0U;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	uint16_key key(static_cast<std::uint16_t>(device() >> 16U));
	for (auto iter = key.begin(); iter != key.end();)
	{
	    iter += 2U;
	    ++output;
	}
    }
    EXPECT_NE(1U, output);
}
