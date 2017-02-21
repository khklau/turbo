#include <turbo/container/bitwise_trie.hpp>
#include <turbo/container/bitwise_trie.hxx>
#include <chrono>
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

/*
TEST(bitwise_trie_test, empty_trie)
{
    typedef tco::bitwise_trie<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    EXPECT_EQ(0U, map1.size()) << "Size of an empty trie is not 0";
    EXPECT_EQ(map1.cend(), map1.cbegin()) << "The cbegin and cend iterators of an empty trie are not equal";
    EXPECT_EQ(map1.crend(), map1.crbegin()) << "The crbegin and crend iterators of an empty trie are not equal";
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
    //EXPECT_NE(map1.end(), map1.find(64U)) << "Could not find just emplaced key & value";
    //EXPECT_EQ(std::string("bar"), *map1.find(64U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
    auto result2 = map1.emplace(32U, "foo");
    EXPECT_TRUE(std::get<1>(result2)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result2)) << "Emplace failed";
    EXPECT_EQ(std::string("foo"), *std::get<0>(result2)) << "Emplace failed";
    EXPECT_EQ(2U, map1.size()) << "Size of trie after 2 emplace is not 2";
    //EXPECT_NE(map1.end(), map1.find(32U)) << "Could not find just emplaced key & value";
    //EXPECT_EQ(std::string("foo"), map1.find(32U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(32U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
    auto result3 = map1.emplace(128U, "blah");
    EXPECT_TRUE(std::get<1>(result3)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result3)) << "Emplace failed";
    EXPECT_EQ(std::string("blah"), *std::get<0>(result3)) << "Emplace failed";
    EXPECT_EQ(3U, map1.size()) << "Size of trie after 3 emplace is not 3";
    //EXPECT_NE(map1.end(), map1.find(128U)) << "Could not find just emplaced key & value";
    //EXPECT_EQ(std::string("blah"), map1.find(128U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(32U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("blah"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(128U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
    //auto iter1 = map1.cbegin();
    //EXPECT_EQ(std::string("foo"), iter1->value) << "Just emplaced key & value is not ordered";
    //++iter1;
    //EXPECT_EQ(std::string("bar"), iter1->value) << "Just emplaced key & value is not ordered";
    //++iter1;
    //EXPECT_EQ(std::string("blah"), iter1->value) << "Just emplaced key & value is not ordered";
    //auto iter2 = map1.crbegin();
    //EXPECT_EQ(std::string("blah"), iter2->value) << "Just emplaced key & value is not ordered";
    //++iter2;
    //EXPECT_EQ(std::string("bar"), iter2->value) << "Just emplaced key & value is not ordered";
    //++iter2;
    //EXPECT_EQ(std::string("foo"), iter2->value) << "Just emplaced key & value is not ordered";
}

TEST(bitwise_trie_test, emplace2_basic)
{
    typedef tco::bitwise_trie<std::uint8_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    auto result1 = map1.emplace2(64U, "bar");
    EXPECT_TRUE(std::get<1>(result1)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result1)) << "Emplace failed";
    EXPECT_EQ(std::string("bar"), *std::get<0>(result1)) << "Emplace failed";
    EXPECT_EQ(1U, map1.size()) << "Size of trie after 1 emplace is not 1";
    //EXPECT_NE(map1.end(), map1.find(64U)) << "Could not find just emplaced key & value";
    //EXPECT_EQ(std::string("bar"), *map1.find(64U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
    auto result2 = map1.emplace2(32U, "foo");
    EXPECT_TRUE(std::get<1>(result2)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result2)) << "Emplace failed";
    EXPECT_EQ(std::string("foo"), *std::get<0>(result2)) << "Emplace failed";
    EXPECT_EQ(2U, map1.size()) << "Size of trie after 2 emplace is not 2";
    //EXPECT_NE(map1.end(), map1.find(32U)) << "Could not find just emplaced key & value";
    //EXPECT_EQ(std::string("foo"), map1.find(32U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(32U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(64U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
    auto result3 = map1.emplace2(128U, "blah");
    EXPECT_TRUE(std::get<1>(result3)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result3)) << "Emplace failed";
    EXPECT_EQ(std::string("blah"), *std::get<0>(result3)) << "Emplace failed";
    EXPECT_EQ(3U, map1.size()) << "Size of trie after 3 emplace is not 3";
    //EXPECT_NE(map1.end(), map1.find(128U)) << "Could not find just emplaced key & value";
    //EXPECT_EQ(std::string("blah"), map1.find(128U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), *map1.cbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(32U, map1.cbegin().get_key()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("blah"), *map1.crbegin()) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(128U, map1.crbegin().get_key()) << "Just emplaced key & value is not ordered";
    //auto iter1 = map1.cbegin();
    //EXPECT_EQ(std::string("foo"), iter1->value) << "Just emplaced key & value is not ordered";
    //++iter1;
    //EXPECT_EQ(std::string("bar"), iter1->value) << "Just emplaced key & value is not ordered";
    //++iter1;
    //EXPECT_EQ(std::string("blah"), iter1->value) << "Just emplaced key & value is not ordered";
    //auto iter2 = map1.crbegin();
    //EXPECT_EQ(std::string("blah"), iter2->value) << "Just emplaced key & value is not ordered";
    //++iter2;
    //EXPECT_EQ(std::string("bar"), iter2->value) << "Just emplaced key & value is not ordered";
    //++iter2;
    //EXPECT_EQ(std::string("foo"), iter2->value) << "Just emplaced key & value is not ordered";
}
*/

class bitwise_trie_perf_test : public ::testing::Test
{
public:
    typedef tco::bitwise_trie<std::uint64_t, std::uint64_t, tme::pool> uint_map;
    bitwise_trie_perf_test()
	:
	    allocator1(8U, { {uint_map::node_sizes[0], (1U << 16U) - 1U + (1U << 15U), 4U} })
    { }
protected:
    tme::pool allocator1;
};

TEST_F(bitwise_trie_perf_test, perf_test_trie_emplace)
{
    uint_map map1(allocator1);
    //std::random_device device;
    std::mt19937 device(std::chrono::system_clock::now().time_since_epoch().count());
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	std::uint64_t value = device() >> 16U;
	map1.emplace(value, value);
    }
}

TEST_F(bitwise_trie_perf_test, perf_test_trie_emplace2)
{
    uint_map map1(allocator1);
    //std::random_device device;
    std::mt19937 device(std::chrono::system_clock::now().time_since_epoch().count());
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	std::uint64_t value = device() >> 16U;
	map1.emplace2(value, value);
    }
}

TEST_F(bitwise_trie_perf_test, perf_test_map_emplace)
{
    std::map<std::uint64_t, std::uint64_t> map1;
    //std::random_device device;
    std::mt19937 device(std::chrono::system_clock::now().time_since_epoch().count());
    for (std::uint64_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	std::uint64_t value = device() >> 16U;
	map1.emplace(value, value);
    }
}
