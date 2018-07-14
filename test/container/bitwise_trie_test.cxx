#include <turbo/container/bitwise_trie.hpp>
#include <turbo/container/bitwise_trie.hh>
#include <chrono>
#include <functional>
#include <map>
#include <random>
#include <gtest/gtest.h>
#include <turbo/memory/slab_allocator.hpp>
#include <turbo/memory/slab_allocator.hh>

namespace turbo {
namespace container {

template <class key_t, class value_t, class allocator_t>
class bitwise_trie_tester
{
public:
    typedef bitwise_trie<key_t, value_t, allocator_t> trie_type;
    typedef typename trie_type::leaf leaf;
    typedef typename trie_type::branch branch;
    typedef typename trie_type::branch_ptr branch_ptr;
    typedef typename trie_type::trie_key trie_key;
    typedef typename trie_type::leading_zero_index leading_zero_index;
    bitwise_trie_tester(trie_type& trie)
	:
	    trie_(trie)
    { }
    inline const typename trie_type::branch_ptr& get_root() const
    {
	return trie_.root_;
    }
    inline const typename trie_type::leading_zero_index& get_index() const
    {
	return trie_.index_;
    }
    inline typename trie_type::leading_zero_index& mutate_index()
    {
	return trie_.index_;
    }
    inline typename trie_type::branch* min()
    {
	return trie_.min();
    }
    inline typename trie_type::branch* max()
    {
	return trie_.max();
    }
    static inline typename trie_type::key_type get_prefix(typename trie_type::key_type key)
    {
	return trie_type::get_prefix(key);
    }
private:
    trie_type& trie_;
};


} // namespace container
} // namespace turbo

namespace tco = turbo::container;
namespace tme = turbo::memory;

TEST(leading_zero_index_test, empty_index)
{
    typedef tco::bitwise_trie_tester<std::uint16_t, std::string, tme::concurrent_sized_slab> trie_tester;
    trie_tester::branch_ptr root1;
    trie_tester::leading_zero_index index1(root1);
    trie_tester::trie_key key1(32768U);
    auto result1 = index1.search(key1);
    EXPECT_EQ(&root1, std::get<0>(result1)) << "Search on empty index did not return the root";
    EXPECT_EQ(key1.begin(), std::get<1>(result1)) << "Search on empty index did not return the root";
    trie_tester::trie_key key2(2048U);
    auto result2 = index1.search(key2);
    EXPECT_EQ(&root1, std::get<0>(result2)) << "Search on empty index did not return the root";
    EXPECT_EQ(key2.begin(), std::get<1>(result2)) << "Search on empty index did not return the root";
    trie_tester::trie_key key3(128U);
    auto result3 = index1.search(key3);
    EXPECT_EQ(&root1, std::get<0>(result3)) << "Search on empty index did not return the root";
    EXPECT_EQ(key3.begin(), std::get<1>(result3)) << "Search on empty index did not return the root";
    trie_tester::trie_key key4(8U);
    auto result4 = index1.search(key4);
    EXPECT_EQ(&root1, std::get<0>(result4)) << "Search on empty index did not return the root";
    EXPECT_EQ(key4.begin(), std::get<1>(result4)) << "Search on empty index did not return the root";
}

TEST(leading_zero_index_test, insert_invalid)
{
    typedef tco::bitwise_trie_tester<std::uint16_t, std::string, tme::concurrent_sized_slab> trie_tester;
    trie_tester::branch root1;
    trie_tester::branch_ptr root_ptr1(&root1);
    trie_tester::leading_zero_index index1(root_ptr1);
    trie_tester::trie_key key1(0U);
    trie_tester::trie_key::iterator input_iter1 = key1.begin();
    ++input_iter1;
    ++input_iter1;
    ++input_iter1;
    ++input_iter1;
    trie_tester::branch branch1;
    index1.insert(&branch1, key1, input_iter1);
    auto result1 = index1.const_search(key1);
    EXPECT_EQ(&root1, std::get<0>(result1)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 16 leading zeroes";
    EXPECT_EQ(key1.begin(), std::get<1>(result1)) << "The leading_zero_index returned wrong iterator for a key with 16 leading zeroes";
}

TEST(leading_zero_index_test, insert_basic)
{
    typedef tco::bitwise_trie_tester<std::uint16_t, std::string, tme::concurrent_sized_slab> trie_tester;
    trie_tester::branch root1;
    trie_tester::branch_ptr root_ptr1(&root1);
    trie_tester::leading_zero_index index1(root_ptr1);
    trie_tester::branch branch101;
    trie_tester::branch branch102;
    trie_tester::branch branch103;
    trie_tester::branch branch104;
    trie_tester::branch branch105;
    trie_tester::branch branch106;
    trie_tester::branch branch107;
    trie_tester::branch branch108;
    trie_tester::branch branch109;
    trie_tester::branch branch110;
    trie_tester::branch branch111;
    trie_tester::branch branch112;
    trie_tester::branch branch113;
    trie_tester::branch branch114;
    trie_tester::branch branch115;
    trie_tester::trie_key key1(0U);
    trie_tester::trie_key::iterator input_iter1 = key1.begin();
    index1.insert(&root1, key1, input_iter1);
    ++input_iter1;
    index1.insert(&branch101, trie_tester::trie_key(1U << 14U), input_iter1);
    index1.insert(&branch102, trie_tester::trie_key(1U << 13U), input_iter1);
    index1.insert(&branch103, trie_tester::trie_key(1U << 12U), input_iter1);
    index1.insert(&branch104, trie_tester::trie_key(1U << 11U), input_iter1);
    ++input_iter1;
    index1.insert(&branch105, trie_tester::trie_key(1U << 10U), input_iter1);
    index1.insert(&branch106, trie_tester::trie_key(1U << 9U), input_iter1);
    index1.insert(&branch107, trie_tester::trie_key(1U << 8U), input_iter1);
    index1.insert(&branch108, trie_tester::trie_key(1U << 7U), input_iter1);
    ++input_iter1;
    index1.insert(&branch109, trie_tester::trie_key(1U << 6U), input_iter1);
    index1.insert(&branch110, trie_tester::trie_key(1U << 5U), input_iter1);
    index1.insert(&branch111, trie_tester::trie_key(1U << 4U), input_iter1);
    index1.insert(&branch112, trie_tester::trie_key(1U << 3U), input_iter1);
    ++input_iter1;
    index1.insert(&branch113, trie_tester::trie_key(1U << 2U), input_iter1);
    index1.insert(&branch114, trie_tester::trie_key(1U << 1U), input_iter1);
    index1.insert(&branch115, trie_tester::trie_key(1U << 0U), input_iter1);
    trie_tester::trie_key::iterator expected_iter1 = key1.begin();
    auto result100 = index1.const_search(trie_tester::trie_key(1U << 15U));
    EXPECT_EQ(&root1, std::get<0>(result100)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 0 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result100)) << "The leading_zero_index returned wrong iterator for a key with 0 leading zeroes";
    auto result101 = index1.const_search(trie_tester::trie_key(1U << 14U));
    EXPECT_EQ(&root1, std::get<0>(result101)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 1 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result101)) << "The leading_zero_index returned wrong iterator for a key with 1 leading zeroes";
    auto result102 = index1.const_search(trie_tester::trie_key(1U << 13U));
    EXPECT_EQ(&root1, std::get<0>(result102)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 2 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result102)) << "The leading_zero_index returned wrong iterator for a key with 2 leading zeroes";
    auto result103 = index1.const_search(trie_tester::trie_key(1U << 12U));
    EXPECT_EQ(&root1, std::get<0>(result103)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 3 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result103)) << "The leading_zero_index returned wrong iterator for a key with 3 leading zeroes";
    ++expected_iter1;
    auto result104 = index1.const_search(trie_tester::trie_key(1U << 11U));
    EXPECT_EQ(&branch104, std::get<0>(result104)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 4 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result104)) << "The leading_zero_index returned wrong iterator for a key with 4 leading zeroes";
    auto result105 = index1.const_search(trie_tester::trie_key(1U << 10U));
    EXPECT_EQ(&branch104, std::get<0>(result105)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 5 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result105)) << "The leading_zero_index returned wrong iterator for a key with 5 leading zeroes";
    auto result106 = index1.const_search(trie_tester::trie_key(1U << 9U));
    EXPECT_EQ(&branch104, std::get<0>(result106)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 6 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result106)) << "The leading_zero_index returned wrong iterator for a key with 6 leading zeroes";
    auto result107 = index1.const_search(trie_tester::trie_key(1U << 8U));
    EXPECT_EQ(&branch104, std::get<0>(result107)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 7 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result107)) << "The leading_zero_index returned wrong iterator for a key with 7 leading zeroes";
    ++expected_iter1;
    auto result108 = index1.const_search(trie_tester::trie_key(1U << 7U));
    EXPECT_EQ(&branch108, std::get<0>(result108)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 8 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result108)) << "The leading_zero_index returned wrong iterator for a key with 8 leading zeroes";
    auto result109 = index1.const_search(trie_tester::trie_key(1U << 6U));
    EXPECT_EQ(&branch108, std::get<0>(result109)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 9 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result109)) << "The leading_zero_index returned wrong iterator for a key with 9 leading zeroes";
    auto result110 = index1.const_search(trie_tester::trie_key(1U << 5U));
    EXPECT_EQ(&branch108, std::get<0>(result110)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 10 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result110)) << "The leading_zero_index returned wrong iterator for a key with 10 leading zeroes";
    auto result111 = index1.const_search(trie_tester::trie_key(1U << 4U));
    EXPECT_EQ(&branch108, std::get<0>(result111)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 11 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result111)) << "The leading_zero_index returned wrong iterator for a key with 11 leading zeroes";
    ++expected_iter1;
    auto result112 = index1.const_search(trie_tester::trie_key(1U << 3U));
    EXPECT_EQ(&branch112, std::get<0>(result112)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 12 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result112)) << "The leading_zero_index returned wrong iterator for a key with 12 leading zeroes";
    auto result113 = index1.const_search(trie_tester::trie_key(1U << 2U));
    EXPECT_EQ(&branch112, std::get<0>(result113)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 13 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result113)) << "The leading_zero_index returned wrong iterator for a key with 13 leading zeroes";
    auto result114 = index1.const_search(trie_tester::trie_key(1U << 1U));
    EXPECT_EQ(&branch112, std::get<0>(result114)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 14 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result114)) << "The leading_zero_index returned wrong iterator for a key with 14 leading zeroes";
    auto result115 = index1.const_search(trie_tester::trie_key(1U << 0U));
    EXPECT_EQ(&branch112, std::get<0>(result115)->get_ptr()) << "The leading_zero_index returned wrong branch for a key with 15 leading zeroes";
    EXPECT_EQ(expected_iter1, std::get<1>(result115)) << "The leading_zero_index returned wrong iterator for a key with 15 leading zeroes";
}

TEST(leading_zero_index_test, remove_invalid)
{
    typedef tco::bitwise_trie_tester<std::uint16_t, std::string, tme::concurrent_sized_slab> trie_tester;
    trie_tester::branch root1;
    trie_tester::branch_ptr root_ptr1(&root1);
    trie_tester::leading_zero_index index1(root_ptr1);
    trie_tester::trie_key key1(0U);
    trie_tester::trie_key::iterator iter1 = key1.begin();
    EXPECT_FALSE(index1.is_defined(key1, iter1));
    index1.remove(key1, iter1);
    EXPECT_FALSE(index1.is_defined(key1, iter1));
    ++iter1;
    EXPECT_FALSE(index1.is_defined(key1, iter1));
    index1.remove(key1, iter1);
    EXPECT_FALSE(index1.is_defined(key1, iter1));
}

TEST(leading_zero_index_test, remove_basic)
{
    typedef tco::bitwise_trie_tester<std::uint8_t, std::string, tme::concurrent_sized_slab> trie_tester;
    trie_tester::branch root1;
    trie_tester::branch_ptr root_ptr1(&root1);
    trie_tester::leading_zero_index index1(root_ptr1);
    trie_tester::branch branch11;
    trie_tester::branch branch12;
    trie_tester::branch branch13;
    trie_tester::branch branch14;
    trie_tester::branch branch15;
    trie_tester::branch branch16;
    trie_tester::branch branch17;
    trie_tester::trie_key key10(1U << 7U);
    trie_tester::trie_key key11(1U << 6U);
    trie_tester::trie_key key12(1U << 5U);
    trie_tester::trie_key key13(1U << 4U);
    trie_tester::trie_key key14(1U << 3U);
    trie_tester::trie_key key15(1U << 2U);
    trie_tester::trie_key key16(1U << 1U);
    trie_tester::trie_key key17(1U << 0U);
    trie_tester::trie_key::iterator iter0 = key10.begin();
    trie_tester::trie_key::iterator iter1 = iter0 + 1U;
    trie_tester::trie_key::iterator iter2 = iter0 + 2U;
    EXPECT_FALSE(index1.is_defined(key10, iter0));
    index1.insert(&root1, key10, iter0);
    EXPECT_TRUE(index1.is_defined(key10, iter0));
    EXPECT_TRUE(index1.is_defined(key11, iter1));
    EXPECT_TRUE(index1.is_defined(key12, iter1));
    EXPECT_TRUE(index1.is_defined(key13, iter1));
    EXPECT_FALSE(index1.is_defined(key14, iter1));
    index1.insert(&branch14, key14, iter1);
    EXPECT_TRUE(index1.is_defined(key14, iter1));
    EXPECT_TRUE(index1.is_defined(key15, iter2));
    EXPECT_TRUE(index1.is_defined(key16, iter2));
    EXPECT_TRUE(index1.is_defined(key17, iter2));
    index1.remove(key10, iter0);
    EXPECT_FALSE(index1.is_defined(key10, iter0)) << "Failed to remove index entry for key with 0 leading zeros";
    EXPECT_FALSE(index1.is_defined(key11, iter1)) << "Failed to remove index entry for key with 1 leading zeros";
    EXPECT_FALSE(index1.is_defined(key12, iter1)) << "Failed to remove index entry for key with 2 leading zeros";
    EXPECT_FALSE(index1.is_defined(key13, iter1)) << "Failed to remove index entry for key with 3 leading zeros";
    index1.remove(key14, iter1);
    EXPECT_FALSE(index1.is_defined(key14, iter1)) << "Failed to remove index entry for key with 4 leading zeros";
    EXPECT_FALSE(index1.is_defined(key15, iter2)) << "Failed to remove index entry for key with 5 leading zeros";
    EXPECT_FALSE(index1.is_defined(key16, iter2)) << "Failed to remove index entry for key with 6 leading zeros";
    EXPECT_FALSE(index1.is_defined(key17, iter2)) << "Failed to remove index entry for key with 7 leading zeros";
}

TEST(bitwise_trie_test, empty_trie)
{
    typedef tco::bitwise_trie<std::uint32_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    EXPECT_EQ(0U, map1.size()) << "Size of an empty trie is not 0";
    EXPECT_EQ(map1.cend(), map1.cbegin()) << "The cbegin and cend iterators of an empty trie are not equal";
    EXPECT_EQ(map1.crend(), map1.crbegin()) << "The crbegin and crend iterators of an empty trie are not equal";
    auto iter1 = map1.cbegin();
    ++iter1;
    EXPECT_EQ(map1.cend(), iter1) << "Finding the successor of the begin iterator from an empty trie succeeded";
    auto iter2 = map1.crbegin();
    ++iter2;
    EXPECT_EQ(map1.crend(), iter2) << "Finding the successor of the begin iterator from an empty trie succeeded";
    EXPECT_EQ(map1.cend(), map1.find(0U)) << "Find on empty trie succedded";
    EXPECT_EQ(map1.cend(), map1.find(128U)) << "Find on empty trie succedded";
    EXPECT_EQ(map1.cend(), map1.find(255U)) << "Find on empty trie succedded";
    EXPECT_EQ(map1.cend(), map1.find_less_equal(0U)) << "Find on empty trie succedded";
    EXPECT_EQ(map1.cend(), map1.find_less_equal(255U)) << "Find on empty trie succedded";
    EXPECT_EQ(0U, map1.erase(64U)) << "erase on empty trie succeeded";
}

TEST(bitwise_trie_test, emplace_invalid)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    auto result1 = map1.emplace(64U, "bar");
    EXPECT_TRUE(std::get<1>(result1)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result1)) << "Emplace failed";
    EXPECT_EQ(std::string("bar"), *std::get<0>(result1)) << "Emplace failed";
    EXPECT_EQ(1U, map1.size()) << "Size of trie after 1 emplace is not 1";
    auto result2 = map1.emplace(64U, "foo");
    EXPECT_FALSE(std::get<1>(result2)) << "Emplace of duplicate succeeded";
    EXPECT_NE(map1.end(), std::get<0>(result2)) << "Emplace of duplicate did not return an iterator to the existing value";
    EXPECT_EQ(std::string("bar"), *std::get<0>(result2)) << "Emplace of duplicate overwrote the existing value";
    EXPECT_EQ(1U, map1.size()) << "Emplace of duplicate increased the size of the trie";
}

TEST(bitwise_trie_test, emplace_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    auto result1 = map1.emplace(64U, "bar");
    EXPECT_TRUE(std::get<1>(result1)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result1)) << "Emplace failed";
    EXPECT_EQ(std::string("bar"), *std::get<0>(result1)) << "Emplace failed";
    EXPECT_EQ(1U, map1.size()) << "Size of trie after 1 emplace is not 1";
    EXPECT_EQ(std::string("bar"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
    auto result2 = map1.emplace(32U, "foo");
    EXPECT_TRUE(std::get<1>(result2)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result2)) << "Emplace failed";
    EXPECT_EQ(std::string("foo"), *std::get<0>(result2)) << "Emplace failed";
    EXPECT_EQ(2U, map1.size()) << "Size of trie after 2 emplace is not 2";
    EXPECT_EQ(std::string("foo"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(32U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
    auto result3 = map1.emplace(128U, "blah");
    EXPECT_TRUE(std::get<1>(result3)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result3)) << "Emplace failed";
    EXPECT_EQ(std::string("blah"), *std::get<0>(result3)) << "Emplace failed";
    EXPECT_EQ(3U, map1.size()) << "Size of trie after 3 emplace is not 3";
    EXPECT_EQ(std::string("foo"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(32U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("blah"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(128U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
}

TEST(bitwise_trie_test, begin_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(255U, "foo");
    auto iter1a = map1.cbegin();
    EXPECT_NE(map1.cend(), iter1a) << "Could not first value";
    EXPECT_EQ(std::string("foo"), *iter1a) << "Could not first value";
    auto iter1b = map1.crbegin();
    EXPECT_NE(map1.crend(), iter1b) << "Could not first value";
    EXPECT_EQ(std::string("foo"), *iter1b) << "Could not first value";
    string_map map2(allocator1);
    map2.emplace(0U, "foo");
    auto iter2a = map2.cbegin();
    EXPECT_NE(map2.cend(), iter2a) << "Could not first value";
    EXPECT_EQ(std::string("foo"), *iter2a) << "Could not first value";
    auto iter2b = map2.crbegin();
    EXPECT_NE(map2.crend(), iter2b) << "Could not first value";
    EXPECT_EQ(std::string("foo"), *iter2b) << "Could not first value";
}

TEST(bitwise_trie_test, find_invalid)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(32U, "foo");
    map1.emplace(64U, "bar");
    map1.emplace(128U, "blah");
    EXPECT_EQ(map1.cend(), map1.find(0U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map1.cend(), map1.find(31U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map1.cend(), map1.find(33U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map1.cend(), map1.find(63U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map1.cend(), map1.find(65U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map1.cend(), map1.find(127U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map1.cend(), map1.find(129U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map1.cend(), map1.find(255U)) << "Find on non-existent key succeeded";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    EXPECT_EQ(map2.cend(), map2.find(0U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map2.cend(), map2.find(125U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map2.cend(), map2.find(129U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map2.cend(), map2.find(255U)) << "Find on non-existent key succeeded";
    string_map map3(allocator1);
    map3.emplace(0U, "foo");
    map3.emplace(255U, "bar");
    EXPECT_EQ(map3.cend(), map3.find(1U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map3.cend(), map3.find(254U)) << "Find on non-existent key succeeded";
}

TEST(bitwise_trie_test, find_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(64U, "bar");
    map1.emplace(32U, "foo");
    map1.emplace(128U, "blah");
    EXPECT_NE(map1.cend(), map1.find(128U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("blah"), *map1.find(128U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find(64U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map1.find(64U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find(32U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map1.find(32U)) << "Could not find just emplaced key & value";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    EXPECT_NE(map1.cend(), map2.find(126U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map2.find(126U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map2.find(127U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map2.find(127U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map2.find(128U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("blah"), *map2.find(128U)) << "Could not find just emplaced key & value";
}

TEST(bitwise_trie_test, forward_successor_invalid)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(0U, "foo");
    map1.emplace(128U, "bar");
    map1.emplace(255U, "blah");
    auto iter1 = map1.cbegin();
    ++iter1;
    ++iter1;
    ++iter1;
    EXPECT_EQ(map1.cend(), iter1) << "Successor of last vaue is not the end iterator";
    ++iter1;
    EXPECT_EQ(map1.cend(), iter1) << "Successor of last vaue is not the end iterator";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    auto iter2 = map2.cbegin();
    ++iter2;
    ++iter2;
    ++iter2;
    EXPECT_EQ(map2.cend(), iter2) << "Successor of last vaue is not the end iterator";
    ++iter2;
    EXPECT_EQ(map2.cend(), iter2) << "Successor of last vaue is not the end iterator";
}

TEST(bitwise_trie_test, forward_successor_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(0U, "foo");
    map1.emplace(128U, "bar");
    map1.emplace(255U, "blah");
    auto iter1 = map1.cbegin();
    ++iter1;
    EXPECT_NE(map1.cend(), iter1) << "Could not find successor";
    EXPECT_EQ(std::string("bar"), *iter1) << "Could not find successor";
    ++iter1;
    EXPECT_NE(map1.cend(), iter1) << "Could not find successor";
    EXPECT_EQ(std::string("blah"), *iter1) << "Could not find successor";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    auto iter2 = map2.cbegin();
    ++iter2;
    EXPECT_NE(map2.cend(), iter2) << "Could not find successor";
    EXPECT_EQ(std::string("bar"), *iter2) << "Could not find successor";
    ++iter2;
    EXPECT_NE(map2.cend(), iter2) << "Could not find successor";
    EXPECT_EQ(std::string("blah"), *iter2) << "Could not find successor";
}

TEST(bitwise_trie_test, forward_predecessor_invalid)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(0U, "foo");
    map1.emplace(128U, "bar");
    map1.emplace(255U, "blah");
    auto iter1 = map1.cbegin();
    ++iter1;
    ++iter1;
    --iter1;
    --iter1;
    --iter1;
    EXPECT_EQ(map1.cend(), iter1) << "Predecessor of first vaue is not the end iterator";
    --iter1;
    EXPECT_EQ(map1.cend(), iter1) << "Predecessor of first vaue is not the end iterator";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    auto iter2 = map2.cbegin();
    ++iter2;
    ++iter2;
    --iter2;
    --iter2;
    --iter2;
    EXPECT_EQ(map2.cend(), iter2) << "Predecessor of first vaue is not the end iterator";
    --iter2;
    EXPECT_EQ(map2.cend(), iter2) << "Predecessor of first vaue is not the end iterator";
}

TEST(bitwise_trie_test, forward_predecessor_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(0U, "foo");
    map1.emplace(128U, "bar");
    map1.emplace(255U, "blah");
    auto iter1 = map1.cbegin();
    ++iter1;
    ++iter1;
    EXPECT_EQ(std::string("blah"), *iter1) << "Could not find predecessor";
    --iter1;
    EXPECT_NE(map1.cend(), iter1) << "Could not find predecessor";
    EXPECT_EQ(std::string("bar"), *iter1) << "Could not find predecessor";
    --iter1;
    EXPECT_NE(map1.cend(), iter1) << "Could not find predecessor";
    EXPECT_EQ(std::string("foo"), *iter1) << "Could not find predecessor";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    auto iter2 = map2.cbegin();
    ++iter2;
    ++iter2;
    EXPECT_EQ(std::string("blah"), *iter2) << "Could not find predecessor";
    --iter2;
    EXPECT_NE(map2.cend(), iter2) << "Could not find predecessor";
    EXPECT_EQ(std::string("bar"), *iter2) << "Could not find predecessor";
    --iter2;
    EXPECT_NE(map2.cend(), iter2) << "Could not find predecessor";
    EXPECT_EQ(std::string("foo"), *iter2) << "Could not find predecessor";
}

TEST(bitwise_trie_test, reverse_successor_invalid)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(0U, "foo");
    map1.emplace(128U, "bar");
    map1.emplace(255U, "blah");
    auto iter1 = map1.crbegin();
    ++iter1;
    ++iter1;
    ++iter1;
    EXPECT_EQ(map1.crend(), iter1) << "Successor of first vaue is not the end iterator";
    ++iter1;
    EXPECT_EQ(map1.crend(), iter1) << "Successor of first vaue is not the end iterator";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    auto iter2 = map2.crbegin();
    ++iter2;
    ++iter2;
    ++iter2;
    EXPECT_EQ(map2.crend(), iter2) << "Successor of first vaue is not the end iterator";
    ++iter2;
    EXPECT_EQ(map2.crend(), iter2) << "Successor of first vaue is not the end iterator";
}

TEST(bitwise_trie_test, reverse_successor_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(0U, "foo");
    map1.emplace(128U, "bar");
    map1.emplace(255U, "blah");
    auto iter1 = map1.crbegin();
    ++iter1;
    EXPECT_NE(map1.crend(), iter1) << "Could not find successor";
    EXPECT_EQ(std::string("bar"), *iter1) << "Could not find successor";
    ++iter1;
    EXPECT_NE(map1.crend(), iter1) << "Could not find successor";
    EXPECT_EQ(std::string("foo"), *iter1) << "Could not find successor";
    ++iter1;
    EXPECT_EQ(map1.crend(), iter1) << "Successor of last vaue is not the end iterator";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    auto iter2 = map2.crbegin();
    ++iter2;
    EXPECT_NE(map2.crend(), iter2) << "Could not find successor";
    EXPECT_EQ(std::string("bar"), *iter2) << "Could not find successor";
    ++iter2;
    EXPECT_NE(map2.crend(), iter2) << "Could not find successor";
    EXPECT_EQ(std::string("foo"), *iter2) << "Could not find successor";
    ++iter2;
    EXPECT_EQ(map2.crend(), iter2) << "Successor of last vaue is not the end iterator";
}

TEST(bitwise_trie_test, reverse_predecessor_invalid)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(0U, "foo");
    map1.emplace(128U, "bar");
    map1.emplace(255U, "blah");
    auto iter1 = map1.crbegin();
    ++iter1;
    ++iter1;
    --iter1;
    --iter1;
    --iter1;
    EXPECT_EQ(map1.crend(), iter1) << "Predecessor of last vaue is not the end iterator";
    --iter1;
    EXPECT_EQ(map1.crend(), iter1) << "Predecessor of last vaue is not the end iterator";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    auto iter2 = map2.crbegin();
    ++iter2;
    ++iter2;
    --iter2;
    --iter2;
    --iter2;
    EXPECT_EQ(map2.crend(), iter2) << "Predecessor of last vaue is not the end iterator";
    --iter2;
    EXPECT_EQ(map2.crend(), iter2) << "Predecessor of last vaue is not the end iterator";
}

TEST(bitwise_trie_test, reverse_predecessor_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(0U, "foo");
    map1.emplace(128U, "bar");
    map1.emplace(255U, "blah");
    auto iter1 = map1.crbegin();
    ++iter1;
    ++iter1;
    EXPECT_EQ(std::string("foo"), *iter1) << "Could not find predecessor";
    --iter1;
    EXPECT_NE(map1.crend(), iter1) << "Could not find predecessor";
    EXPECT_EQ(std::string("bar"), *iter1) << "Could not find predecessor";
    --iter1;
    EXPECT_NE(map1.crend(), iter1) << "Could not find predecessor";
    EXPECT_EQ(std::string("blah"), *iter1) << "Could not find predecessor";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    map2.emplace(128U, "blah");
    auto iter2 = map2.crbegin();
    ++iter2;
    ++iter2;
    EXPECT_EQ(std::string("foo"), *iter2) << "Could not find predecessor";
    --iter2;
    EXPECT_NE(map2.crend(), iter2) << "Could not find predecessor";
    EXPECT_EQ(std::string("bar"), *iter2) << "Could not find predecessor";
    --iter2;
    EXPECT_NE(map2.crend(), iter2) << "Could not find predecessor";
    EXPECT_EQ(std::string("blah"), *iter2) << "Could not find predecessor";
}

TEST(bitwise_trie_test, find_less_equal_invalid)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(1U, "foo");
    map1.emplace(32U, "bar");
    EXPECT_EQ(map1.cend(), map1.find_less_equal(0U)) << "Find on non-existent key succeeded";
    string_map map2(allocator1);
    map2.emplace(128U, "foo");
    map2.emplace(129U, "bar");
    EXPECT_EQ(map2.cend(), map2.find_less_equal(0U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map2.cend(), map2.find_less_equal(127U)) << "Find on non-existent key succeeded";
    string_map map3(allocator1);
    map3.emplace(255U, "bar");
    EXPECT_EQ(map3.cend(), map3.find_less_equal(0U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map3.cend(), map3.find_less_equal(127U)) << "Find on non-existent key succeeded";
    EXPECT_EQ(map3.cend(), map3.find_less_equal(254U)) << "Find on non-existent key succeeded";
}

TEST(bitwise_trie_test, find_less_equal_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(64U, "bar");
    map1.emplace(32U, "foo");
    map1.emplace(128U, "blah");
    EXPECT_NE(map1.cend(), map1.find_less_equal(255U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("blah"), *map1.find_less_equal(255U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find_less_equal(129U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("blah"), *map1.find_less_equal(129U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find_less_equal(128U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("blah"), *map1.find_less_equal(128U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find_less_equal(127U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map1.find_less_equal(127U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find_less_equal(65U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map1.find_less_equal(65U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find_less_equal(64U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map1.find_less_equal(64U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find_less_equal(63U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map1.find_less_equal(63U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find_less_equal(33U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map1.find_less_equal(33U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map1.cend(), map1.find_less_equal(32U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map1.find_less_equal(32U)) << "Could not find just emplaced key & value";
    string_map map2(allocator1);
    map2.emplace(126U, "foo");
    map2.emplace(127U, "bar");
    EXPECT_NE(map2.cend(), map2.find_less_equal(126U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map2.find_less_equal(126U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map2.cend(), map2.find_less_equal(127U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map2.find_less_equal(127U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map2.cend(), map2.find_less_equal(128U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map2.find_less_equal(128U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map2.cend(), map2.find_less_equal(255U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map2.find_less_equal(255U)) << "Could not find just emplaced key & value";
    string_map map3(allocator1);
    map3.emplace(254U, "foo");
    EXPECT_NE(map3.cend(), map3.find_less_equal(255U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map3.find_less_equal(255U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map3.cend(), map3.find_less_equal(254U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map3.find_less_equal(254U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(map3.cend(), map3.find_less_equal(253U)) << "Find returned a key & value that should not exist";
    EXPECT_EQ(map3.cend(), map3.find_less_equal(128U)) << "Find returned a key & value that should not exist";
    EXPECT_EQ(map3.cend(), map3.find_less_equal(127U)) << "Find returned a key & value that should not exist";
    EXPECT_EQ(map3.cend(), map3.find_less_equal(1U)) << "Find returned a key & value that should not exist";
    EXPECT_EQ(map3.cend(), map3.find_less_equal(0U)) << "Find returned a key & value that should not exist";
    string_map map4(allocator1);
    map4.emplace(0U, "foo");
    EXPECT_NE(map4.cend(), map4.find_less_equal(1U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map4.find_less_equal(1U)) << "Could not find just emplaced key & value";
    EXPECT_NE(map4.cend(), map4.find_less_equal(0U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map4.find_less_equal(0U)) << "Could not find just emplaced key & value";
}

TEST(bitwise_trie_test, erase_invalid)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_map;
    typedef tco::bitwise_trie_tester<std::uint8_t, std::string, tme::concurrent_sized_slab> map_tester;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    map1.emplace(1U, "foo");
    map1.emplace(32U, "bar");
    EXPECT_EQ(0U, map1.erase(0U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map1.erase(2U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map1.erase(31U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map1.erase(33U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map1.erase(255U)) << "erase on non-existent key succeeded";
    string_map map2(allocator1);
    map2.emplace(128U, "foo");
    map2.emplace(129U, "bar");
    EXPECT_EQ(0U, map2.erase(0U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map2.erase(127U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map2.erase(130U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map2.erase(255U)) << "erase on non-existent key succeeded";
    string_map map3(allocator1);
    map3.emplace(255U, "bar");
    EXPECT_EQ(0U, map3.erase(0U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map3.erase(127U)) << "erase on non-existent key succeeded";
    EXPECT_EQ(0U, map3.erase(254U)) << "erase on non-existent key succeeded";
    string_map map4(allocator1);
    map_tester tester4(map4);
    map4.emplace(0U, "foo");
    map4.emplace(128U, "bar");
    map4.emplace(255U, "blah");
    EXPECT_EQ(3U, map4.size()) << "Size of trie after emplace is wrong";
    EXPECT_FALSE(tester4.get_root().is_empty()) << "Root of trie after emplace is empty";
    EXPECT_EQ(1U, map4.erase(128U)) << "erase on valid key failed";
    EXPECT_EQ(2U, map4.size()) << "Size of trie after erase is wrong";
    EXPECT_EQ(1U, map4.erase(255U)) << "erase on valid key failed";
    EXPECT_EQ(1U, map4.size()) << "Size of trie after erase is wrong";
    EXPECT_EQ(1U, map4.erase(0U)) << "erase on valid key failed";
    EXPECT_EQ(0U, map4.size()) << "Size of trie after erase is wrong";
    EXPECT_TRUE(tester4.get_root().is_empty()) << "Root of trie after erase of all values is not empty";
    EXPECT_EQ(0U, map4.erase(128U)) << "erase on already erased key succeeded";
    EXPECT_EQ(0U, map4.size()) << "Size of trie changed after unsuccessful erase is wrong";
    EXPECT_EQ(0U, map4.erase(255U)) << "erase on already erased key succeeded";
    EXPECT_EQ(0U, map4.size()) << "Size of trie changed after unsuccessful erase is wrong";
    EXPECT_EQ(0U, map4.erase(0U)) << "erase on already erased key succeeded";
    EXPECT_EQ(0U, map4.size()) << "Size of trie changed after unsuccessful erase is wrong";
    EXPECT_TRUE(tester4.get_root().is_empty()) << "Root of trie after erase of all values is not empty";
}

TEST(bitwise_trie_test, erase_basic)
{
    typedef tco::bitwise_trie<std::uint64_t, std::string, tme::concurrent_sized_slab> string_map;
    typedef tco::bitwise_trie_tester<std::uint64_t, std::string, tme::concurrent_sized_slab> map_tester;
    tme::concurrent_sized_slab allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });

    string_map map1(allocator1);
    map_tester tester1(map1);
    map1.emplace(128U, "foo");
    EXPECT_EQ(1U, map1.size()) << "Size of trie after emplace is wrong";
    EXPECT_FALSE(tester1.get_root().is_empty()) << "Root of trie after emplace is empty";
    EXPECT_EQ(1U, map1.erase(128U)) << "erase on valid key failed";
    EXPECT_EQ(map1.cend(), map1.find(128U)) << "value that was just erased can still be found";
    EXPECT_EQ(0U, map1.size()) << "Size of trie after erase is wrong";
    EXPECT_TRUE(tester1.get_root().is_empty()) << "Root of trie after erase of all values is not empty";

    string_map map2(allocator1);
    map_tester tester2(map2);
    map2.emplace(0U, "foo");
    map2.emplace(1U, "bar");
    EXPECT_EQ(2U, map2.size()) << "Size of trie after emplace is wrong";
    EXPECT_FALSE(tester2.get_root().is_empty()) << "Root of trie after emplace is empty";
    EXPECT_EQ(1U, map2.erase(0U)) << "erase on valid key failed";
    EXPECT_EQ(map2.cend(), map2.find(0U)) << "value that was just erased can still be found";
    EXPECT_EQ(1U, map2.size()) << "Size of trie after erase is wrong";
    EXPECT_FALSE(tester2.get_root().is_empty()) << "Root of trie when some values still remain after erase is empty";
    EXPECT_EQ(1U, map2.erase(1U)) << "erase on valid key failed";
    EXPECT_EQ(map2.cend(), map2.find(1U)) << "value that was just erased can still be found";
    EXPECT_EQ(0U, map2.size()) << "Size of trie after erase is wrong";
    EXPECT_TRUE(tester2.get_root().is_empty()) << "Root of trie after erase of all values is not empty";

    string_map map3(allocator1);
    map_tester tester3(map3);
    map3.emplace(128U, "blah");
    map3.emplace(126U, "foo");
    map3.emplace(127U, "bar");
    EXPECT_EQ(3U, map3.size()) << "Size of trie after emplace is wrong";
    EXPECT_FALSE(tester3.get_root().is_empty()) << "Root of trie after emplace is empty";
    EXPECT_EQ(1U, map3.erase(127U)) << "erase on valid key failed";
    EXPECT_EQ(map3.cend(), map3.find(127U)) << "value that was just erased can still be found";
    EXPECT_EQ(2U, map3.size()) << "Size of trie after erase is wrong";
    EXPECT_FALSE(tester3.get_root().is_empty()) << "Root of trie when some values still remain after erase is empty";
    EXPECT_EQ(1U, map3.erase(126U)) << "erase on valid key failed";
    EXPECT_EQ(map3.cend(), map3.find(126U)) << "value that was just erased can still be found";
    EXPECT_EQ(1U, map3.size()) << "Size of trie after erase is wrong";
    EXPECT_FALSE(tester3.get_root().is_empty()) << "Root of trie when some values still remain after erase is empty";
    EXPECT_EQ(1U, map3.erase(128U)) << "erase on valid key failed";
    EXPECT_EQ(map3.cend(), map3.find(128U)) << "value that was just erased can still be found";
    EXPECT_EQ(0U, map3.size()) << "Size of trie after erase is wrong";
    EXPECT_TRUE(tester3.get_root().is_empty()) << "Root of trie after erase of all values is not empty";
}

TEST(bitwise_trie_test, emplace_erase_reuse)
{
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::concurrent_sized_slab> uint64_map;
    typedef tco::bitwise_trie_tester<std::uint64_t, std::uint64_t, tme::concurrent_sized_slab> map_tester;
    tme::concurrent_sized_slab allocator1(8U, { {uint64_map::node_sizes[0], 8U}, {uint64_map::node_sizes[1], 15U} });

    uint64_map map1(allocator1);
    map_tester tester1(map1);
    map1.emplace(1U, 1U);
    map1.emplace(3U, 3U);
    map1.emplace(5U, 5U);
    map1.emplace(7U, 7U);
    map1.emplace(11U, 11U);
    map1.emplace(13U, 13U);
    map1.emplace(17U, 17U);
    map1.emplace(19U, 19U);
    EXPECT_EQ(8U, map1.size()) << "Size of trie after emplace is wrong";
    EXPECT_FALSE(tester1.get_root().is_empty()) << "Root of trie after emplace is empty";
    map1.erase(1U);
    map1.erase(3U);
    map1.erase(5U);
    map1.erase(7U);
    map1.erase(11U);
    map1.erase(13U);
    map1.erase(17U);
    map1.erase(19U);
    EXPECT_EQ(0U, map1.size()) << "Size of trie after erase is wrong";
    EXPECT_TRUE(tester1.get_root().is_empty()) << "Root of trie after erase of all values is not empty";
    map1.emplace(1U, 101U);
    map1.emplace(3U, 103U);
    map1.emplace(5U, 105U);
    map1.emplace(7U, 107U);
    map1.emplace(11U, 111U);
    map1.emplace(13U, 113U);
    map1.emplace(17U, 117U);
    map1.emplace(19U, 119U);
    EXPECT_EQ(8U, map1.size()) << "Size of trie after emplace is wrong";
    EXPECT_FALSE(tester1.get_root().is_empty()) << "Root of trie after emplace is empty";
    EXPECT_EQ(101U, *map1.find(1U)) << "replacement value that was just emplacex could not be found";
    EXPECT_EQ(103U, *map1.find(3U)) << "replacement value that was just emplacex could not be found";
    EXPECT_EQ(105U, *map1.find(5U)) << "replacement value that was just emplacex could not be found";
    EXPECT_EQ(107U, *map1.find(7U)) << "replacement value that was just emplacex could not be found";
    EXPECT_EQ(111U, *map1.find(11U)) << "replacement value that was just emplacex could not be found";
    EXPECT_EQ(113U, *map1.find(13U)) << "replacement value that was just emplacex could not be found";
    EXPECT_EQ(117U, *map1.find(17U)) << "replacement value that was just emplacex could not be found";
    EXPECT_EQ(119U, *map1.find(19U)) << "replacement value that was just emplacex could not be found";
    map1.erase(1U);
    map1.erase(3U);
    map1.erase(5U);
    map1.erase(7U);
    map1.erase(11U);
    map1.erase(13U);
    map1.erase(17U);
    map1.erase(19U);
    EXPECT_EQ(0U, map1.size()) << "Size of trie after erase is wrong";
    EXPECT_TRUE(tester1.get_root().is_empty()) << "Root of trie after erase of all values is not empty";
}

TEST(bitwise_trie_test, copy_construct)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::concurrent_sized_slab> string_trie;
    tme::concurrent_sized_slab allocator1(8U, { {string_trie::node_sizes[0], 8U}, {string_trie::node_sizes[1], 8U} });
    tme::concurrent_sized_slab allocator2(8U, { {string_trie::node_sizes[0], 8U}, {string_trie::node_sizes[1], 8U} });
    string_trie map1(allocator1);
    map1.emplace(64U, "bar");
    map1.emplace(32U, "foo");
    map1.emplace(128U, "blah");
    EXPECT_EQ(3U, map1.size()) << "Size of trie after 3 emplace is not 3";
    string_trie map2(map1, &allocator2);
    EXPECT_FALSE(allocator1 == allocator2) << "Memory slabs should not be equal because they contain points to different values";
    EXPECT_TRUE(map1 == map2) << "Copy constructed bitwise trie is not equal to the original";
}

class bitwise_trie_emplace_perf_test : public ::testing::Test
{
public:
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::concurrent_sized_slab> uint_trie;
    typedef std::map<std::uint64_t, std::uint64_t> uint_map;
    bitwise_trie_emplace_perf_test()
	:
	    allocator1(8U, { {uint_trie::node_sizes[0], 1ULL << 16U}, {uint_trie::node_sizes[1], (1ULL << 17U) - 1U} }),
	    trie(allocator1),
	    map(),
	    values(1U << 16U)
    {
	std::random_device device;
	uint_map accepted;
	for (std::uint64_t counter = accepted.size(); counter <= std::numeric_limits<std::uint16_t>::max(); counter = accepted.size())
	{
	    std::uint64_t value = device();
	    if (accepted.emplace(value, value).second)
	    {
		values[counter] = value;
	    }
	}
    }
protected:
    tme::concurrent_sized_slab allocator1;
    uint_trie trie;
    uint_map map;
    std::vector<std::uint64_t> values;
};

TEST_F(bitwise_trie_emplace_perf_test, perf_test_emplace_overhead)
{
    EXPECT_TRUE(true);
}

TEST_F(bitwise_trie_emplace_perf_test, perf_test_trie_emplace)
{
    auto end = trie.end();
    for (auto value: values)
    {
	auto result = trie.emplace(value, value);
	EXPECT_TRUE(std::get<1>(result)) << "Failed to emplace " << value;
	EXPECT_NE(end, std::get<0>(result));
	EXPECT_EQ(value, *(std::get<0>(result)));
    }
}

TEST_F(bitwise_trie_emplace_perf_test, perf_test_map_emplace)
{
    auto end = map.end();
    for (auto value: values)
    {
	trie.emplace(value, value);
	auto result = map.emplace(value, value);
	EXPECT_TRUE(result.second);
	EXPECT_NE(end, result.first);
	EXPECT_EQ(value, (result.first)->first);
    }
}

class bitwise_trie_find_perf_test : public ::testing::Test
{
public:
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::concurrent_sized_slab> uint_trie;
    typedef std::map<std::uint64_t, std::uint64_t, std::greater_equal<std::uint64_t>> uint_map;
    bitwise_trie_find_perf_test()
	:
	    allocator(8U, { {uint_trie::node_sizes[0], 1U << 16U}, {uint_trie::node_sizes[1], (1U << 17U) - 1U} }),
	    trie(allocator),
	    map(),
	    values(1U << 16U)
    {
	std::random_device device;
	for (std::uint64_t counter = map.size(); counter <= std::numeric_limits<std::uint16_t>::max(); counter = map.size())
	{
	    std::uint64_t value = device();
	    if (std::get<1>(trie.emplace(value, value)) && map.emplace(value, value).second)
	    {
		values[counter] = value;
	    }
	}
    }
protected:
    tme::concurrent_sized_slab allocator;
    uint_trie trie;
    uint_map map;
    std::vector<std::uint64_t> values;
};

TEST_F(bitwise_trie_find_perf_test, perf_test_find_overhead)
{
    EXPECT_TRUE(true);
}

TEST_F(bitwise_trie_find_perf_test, perf_test_trie_find)
{
    for (auto value: values)
    {
	trie.find(value);
    }
}

TEST_F(bitwise_trie_find_perf_test, perf_test_map_find)
{
    for (auto value: values)
    {
	map.find(value);
    }
}

class bitwise_trie_find_less_equal_perf_test : public ::testing::Test
{
public:
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::concurrent_sized_slab> uint_trie;
    typedef std::map<std::uint64_t, std::uint64_t, std::greater_equal<std::uint64_t>> uint_map;
    bitwise_trie_find_less_equal_perf_test()
	:
	    allocator(8U, { {uint_trie::node_sizes[0], 1U << 16U}, {uint_trie::node_sizes[1], (1U << 17U) - 1U} }),
	    trie(allocator),
	    map(),
	    input(1U << 16U),
	    expected(1U << 16U)
    {
	std::uint64_t previous = 0U;
	for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
	{
	    if (counter == 0U)
	    {
		trie.emplace(0U, 0U);
		map.emplace(0U, 0U);
		expected[counter] = 0U;
		input[counter] = 1U;
		previous = 1U;
	    }
	    else
	    {
		std::uint64_t value = previous + 1U;
		trie.emplace(value, value);
		map.emplace(value, value);
		expected[counter] = value;
		input[counter] = value + counter;
		previous = value + counter;
	    }
	}
    }
protected:
    tme::concurrent_sized_slab allocator;
    uint_trie trie;
    uint_map map;
    std::vector<std::uint64_t> input;
    std::vector<std::uint64_t> expected;
};

TEST_F(bitwise_trie_find_less_equal_perf_test, perf_test_find_less_equal_overhead)
{
    EXPECT_TRUE(true);
}

TEST_F(bitwise_trie_find_less_equal_perf_test, perf_test_trie_find_less_equal)
{
    uint_trie::const_iterator end = trie.cend();
    for (std::size_t iter = 0U; iter < input.size() && iter < expected.size(); ++iter)
    {
	uint_trie::const_iterator result = trie.find_less_equal(input[iter]);
	ASSERT_NE(end, result) << "Failed find_less_equal for key " << input[iter] << " @ " << iter;
	EXPECT_EQ(expected[iter], *result) << "Failed find_less_equal for key " << input[iter] << " @ " << iter;
    }
}

TEST_F(bitwise_trie_find_less_equal_perf_test, perf_test_map_find_less_equal)
{
    uint_map::const_iterator end = map.cend();
    for (std::size_t iter = 0U; iter < input.size() && iter < expected.size(); ++iter)
    {
	uint_map::const_iterator result = map.upper_bound(input[iter]);
	ASSERT_NE(end, result) << "Failed upper_bound for key " << input[iter] << " @ " << iter;
	EXPECT_EQ(expected[iter], result->first) << "Failed upper_bound for key " << input[iter] << " @ " << iter;
    }
}

class bitwise_trie_erase_perf_test : public ::testing::Test
{
public:
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::concurrent_sized_slab> uint_trie;
    typedef std::map<std::uint64_t, std::uint64_t> uint_map;
    bitwise_trie_erase_perf_test()
	:
	    allocator(8U, { {uint_trie::node_sizes[0], 1U << 16U}, {uint_trie::node_sizes[1], (1U << 17U) - 1U} }),
	    trie(allocator),
	    map(),
	    input(1U << 16U)
    {
	std::random_device device;
	for (std::uint64_t counter = map.size(); map.size() <= std::numeric_limits<std::uint16_t>::max(); counter = map.size())
	{
	    std::uint64_t value = device();
	    trie.emplace(value, value);
	    map.emplace(value, value);
	    input[counter] = value;
	}
    }
protected:
    tme::concurrent_sized_slab allocator;
    uint_trie trie;
    uint_map map;
    std::vector<std::uint64_t> input;
};

TEST_F(bitwise_trie_erase_perf_test, perf_test_erase_overhead)
{
    EXPECT_TRUE(true);
}

TEST_F(bitwise_trie_erase_perf_test, perf_test_trie_erase)
{
    for (std::uint64_t key: input)
    {
	EXPECT_EQ(1U, trie.erase(key)) << "Failed erase for key " << key;
    }
}

TEST_F(bitwise_trie_erase_perf_test, perf_test_map_erase)
{
    for (std::uint64_t key: input)
    {
	EXPECT_EQ(1U, map.erase(key)) << "Failed erase for key " << key;
    }
}
