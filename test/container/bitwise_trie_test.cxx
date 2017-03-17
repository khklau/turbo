#include <turbo/container/bitwise_trie.hpp>
#include <turbo/container/bitwise_trie.hxx>
#include <chrono>
#include <functional>
#include <map>
#include <random>
#include <gtest/gtest.h>
#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>

namespace turbo {
namespace container {

template <class key_t, class value_t, class allocator_t>
class bitwise_trie_tester
{
public:
    typedef bitwise_trie<key_t, value_t, allocator_t> trie_type;
    typedef typename trie_type::leaf leaf;
    typedef typename trie_type::branch branch;
    bitwise_trie_tester(trie_type& trie)
	:
	    trie_(trie)
    { }
    inline typename trie_type::branch* get_root()
    {
	return trie_.root_;
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

TEST(bitwise_trie_test, empty_trie)
{
    typedef tco::bitwise_trie<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
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

class bitwise_trie_emplace_perf_test : public ::testing::Test
{
public:
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::pool> uint_trie;
    bitwise_trie_emplace_perf_test()
	:
	    allocator1(8U, { {uint_trie::node_sizes[0], 1U << 16U}, {uint_trie::node_sizes[1], (1U << 17U) - 1U} })
    { }
protected:
    tme::pool allocator1;
};

TEST_F(bitwise_trie_emplace_perf_test, perf_test_emplace_overhead)
{
    EXPECT_TRUE(true);
}

TEST_F(bitwise_trie_emplace_perf_test, perf_test_trie_emplace)
{
    uint_trie map1(allocator1);
    auto end = map1.end();
    std::random_device device;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	std::uint64_t value = device() >> 16U;
	EXPECT_NE(end, std::get<0>(map1.emplace(value, value)));
    }
}

TEST_F(bitwise_trie_emplace_perf_test, perf_test_map_emplace)
{
    std::map<std::uint64_t, std::uint64_t> map1;
    auto end = map1.end();
    std::random_device device;
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	std::uint64_t value = device() >> 16U;
	EXPECT_NE(end, map1.emplace(value, value).first);
    }
}

class bitwise_trie_find_perf_test : public ::testing::Test
{
public:
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::pool> uint_trie;
    typedef std::map<std::uint64_t, std::uint64_t, std::greater_equal<std::uint64_t>> uint_map;
    bitwise_trie_find_perf_test()
	:
	    allocator(8U, { {uint_trie::node_sizes[0], 1U << 16U}, {uint_trie::node_sizes[1], (1U << 17U) - 1U} }),
	    trie(allocator),
	    map(),
	    values(1U << 15U)
    {
	std::random_device device;
	while (values.size() < std::numeric_limits<std::uint16_t>::max())
	{
	    std::uint64_t value = device() >> 16U;
	    if (std::get<1>(trie.emplace(value, value)) && map.emplace(value, value).second)
	    {
		values.emplace_back(value);
	    }
	}
    }
protected:
    tme::pool allocator;
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
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::pool> uint_trie;
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
	for (std::uint64_t counter = 0U; counter < std::numeric_limits<std::uint16_t>::max(); ++counter)
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
    tme::pool allocator;
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
