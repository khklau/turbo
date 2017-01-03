#include <turbo/container/emplacing_skiplist.hpp>
#include <turbo/container/emplacing_skiplist.hxx>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>

namespace turbo {
namespace container {

template <class key_t, class value_t, class allocator_t, class compare_f>
class emplacing_skiplist_tester
{
public:
    typedef emplacing_skiplist<key_t, value_t, allocator_t, compare_f> skiplist_type;
    typedef typename skiplist_type::record record;
    typedef typename skiplist_type::store store;
    typedef typename skiplist_type::store_region store_region;
    typedef typename skiplist_type::room room;
    typedef typename skiplist_type::floor floor;
    typedef typename skiplist_type::floor_region floor_region;
    emplacing_skiplist_tester(skiplist_type& skiplist)
	:
	    skiplist_(skiplist)
    { }
    typename skiplist_type::floor_region search_floor(
	    const typename skiplist_type::key_type& key,
	    const typename skiplist_type::floor::iterator& iter)
    {
	return skiplist_.search_floor(std::forward<decltype(key)>(key), std::forward<decltype(iter)>(iter));
    }
    typename skiplist_type::store_region search_store(
	    const typename skiplist_type::key_type& key,
	    const typename skiplist_type::store::iterator& iter)
    {
	return skiplist_.search_store(std::forward<decltype(key)>(key), std::forward<decltype(iter)>(iter));
    }
private:
    skiplist_type& skiplist_;
};


} // namespace container
} // namespace turbo

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;
namespace tme = turbo::memory;

TEST(emplacing_skiplist_test, empty_skiplist)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U}, {string_map::node_sizes[2], 8U} });
    string_map map1(allocator1);
    EXPECT_EQ(map1.end(), map1.begin()) << "In an empty skiplist begin and end iterators are not equal";
    EXPECT_EQ(0U, map1.size()) << "Size of an empty skiplist is not 0";
}

TEST(emplacing_skiplist_test, search_floor_basic)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::uint32_t, tme::pool> uint_map;
    typedef tco::emplacing_skiplist_tester<std::uint32_t, std::uint32_t, tme::pool> uint_tester;
    tme::pool allocator1(
	    8U,
	    {
		{uint_map::node_sizes[0], std::numeric_limits<std::uint8_t>::max()},
		{uint_map::node_sizes[1], std::numeric_limits<std::uint8_t>::max()},
		{uint_map::node_sizes[2], std::numeric_limits<std::uint8_t>::max()}
	    });
    uint_map map1(allocator1);
    uint_tester tester1(map1);
    typename uint_tester::floor floor1(allocator1);
    typename uint_tester::floor::iterator current1;
    typename uint_tester::floor::iterator next1;
    const typename uint_tester::floor::iterator empty;
    std::tie(current1, next1) = tester1.search_floor(0U, floor1.begin());
    EXPECT_EQ(empty, current1) << "Search of empty floor did not return empty result";
    EXPECT_EQ(empty, next1) << "Search of empty floor did not return empty result";
    typename uint_tester::floor::iterator room1 = floor1.emplace_back(
	    1U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    std::tie(current1, next1) = tester1.search_floor(0U, floor1.begin());
    EXPECT_EQ(empty, current1) << "Search of floor for key less than all rooms failed";
    EXPECT_EQ(room1, next1) << "Search of floor for key less than all rooms failed";
    std::tie(current1, next1) = tester1.search_floor(1U, floor1.begin());
    EXPECT_EQ(room1, current1) << "Search of floor for key used by the only room failed";
    EXPECT_EQ(empty, next1) << "Search of floor for key used by the only room failed";
    std::tie(current1, next1) = tester1.search_floor(2U, floor1.begin());
    EXPECT_EQ(room1, current1) << "Search of empty floor for key greater than all rooms failed";
    EXPECT_EQ(empty, next1) << "Search of empty floor did key greater than all rooms failed";
    typename uint_tester::floor::iterator room2 = floor1.emplace_back(
	    3U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    std::tie(current1, next1) = tester1.search_floor(0U, floor1.begin());
    EXPECT_EQ(empty, current1) << "Search of floor for key less than all rooms failed";
    EXPECT_EQ(room1, next1) << "Search of floor for key less than all rooms failed";
    std::tie(current1, next1) = tester1.search_floor(1U, floor1.begin());
    EXPECT_EQ(room1, current1) << "Search of floor for key used by the first room failed";
    EXPECT_EQ(room2, next1) << "Search of floor for key used by the first room failed";
    std::tie(current1, next1) = tester1.search_floor(2U, floor1.begin());
    EXPECT_EQ(room1, current1) << "Search of floor for key between the rooms failed";
    EXPECT_EQ(room2, next1) << "Search of floor for key between the rooms failed";
    std::tie(current1, next1) = tester1.search_floor(3U, floor1.begin());
    EXPECT_EQ(room2, current1) << "Search of floor for key used by the last room failed";
    EXPECT_EQ(empty, next1) << "Search of floor for key used by the last room failed";
    std::tie(current1, next1) = tester1.search_floor(4U, floor1.begin());
    EXPECT_EQ(room2, current1) << "Search of floor for key greater than all rooms failed";
    EXPECT_EQ(empty, next1) << "Search of floor for key greater than all rooms failed";
}

TEST(emplacing_skiplist_test, search_store_basic)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::uint32_t, tme::pool> uint_map;
    typedef tco::emplacing_skiplist_tester<std::uint32_t, std::uint32_t, tme::pool> uint_tester;
    tme::pool allocator1(
	    8U,
	    {
		{uint_map::node_sizes[0], std::numeric_limits<std::uint8_t>::max()},
		{uint_map::node_sizes[1], std::numeric_limits<std::uint8_t>::max()},
		{uint_map::node_sizes[2], std::numeric_limits<std::uint8_t>::max()}
	    });
    uint_map map1(allocator1);
    uint_tester tester1(map1);
    typename uint_tester::store store1(allocator1);
    typename uint_tester::store::iterator current1;
    typename uint_tester::store::iterator next1;
    const typename uint_tester::store::iterator empty;
    std::tie(current1, next1) = tester1.search_store(0U, store1.begin());
    EXPECT_EQ(empty, current1) << "Search of empty store did not return empty result";
    EXPECT_EQ(empty, next1) << "Search of empty store did not return empty result";
    typename uint_tester::store::iterator record1 = store1.emplace_back(
	    1U,
	    1U);
    std::tie(current1, next1) = tester1.search_store(0U, store1.begin());
    EXPECT_EQ(empty, current1) << "Search of store for key less than all records failed";
    EXPECT_EQ(record1, next1) << "Search of store for key less than all records failed";
    std::tie(current1, next1) = tester1.search_store(1U, store1.begin());
    EXPECT_EQ(record1, current1) << "Search of store for key used by the only record failed";
    EXPECT_EQ(empty, next1) << "Search of store for key used by the only record failed";
    std::tie(current1, next1) = tester1.search_store(2U, store1.begin());
    EXPECT_EQ(record1, current1) << "Search of empty store for key greater than all records failed";
    EXPECT_EQ(empty, next1) << "Search of empty store did key greater than all records failed";
    typename uint_tester::store::iterator record2 = store1.emplace_back(
	    3U,
	    3U);
    std::tie(current1, next1) = tester1.search_store(0U, store1.begin());
    EXPECT_EQ(empty, current1) << "Search of store for key less than all records failed";
    EXPECT_EQ(record1, next1) << "Search of store for key less than all records failed";
    std::tie(current1, next1) = tester1.search_store(1U, store1.begin());
    EXPECT_EQ(record1, current1) << "Search of store for key used by the first record failed";
    EXPECT_EQ(record2, next1) << "Search of store for key used by the first record failed";
    std::tie(current1, next1) = tester1.search_store(2U, store1.begin());
    EXPECT_EQ(record1, current1) << "Search of store for key between the records failed";
    EXPECT_EQ(record2, next1) << "Search of store for key between the records failed";
    std::tie(current1, next1) = tester1.search_store(3U, store1.begin());
    EXPECT_EQ(record2, current1) << "Search of store for key used by the last record failed";
    EXPECT_EQ(empty, next1) << "Search of store for key used by the last record failed";
    std::tie(current1, next1) = tester1.search_store(4U, store1.begin());
    EXPECT_EQ(record2, current1) << "Search of store for key greater than all records failed";
    EXPECT_EQ(empty, next1) << "Search of store for key greater than all records failed";
}

TEST(emplacing_skiplist_test, find_invalid)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U}, {string_map::node_sizes[2], 8U} });
    string_map map1(allocator1);
    EXPECT_EQ(map1.end(), map1.find(99U)) << "Search for non-existant key did not return empty iterator";
}

TEST(emplacing_skiplist_test, emplace_basic)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U}, {string_map::node_sizes[2], 8U} });
    string_map map1(allocator1);
    auto result1 = map1.emplace(6U, "foo");
    EXPECT_TRUE(std::get<1>(result1)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result1)) << "Emplace failed";
    EXPECT_EQ(std::string("foo"), std::get<0>(result1)->value) << "Emplace failed";
    EXPECT_EQ(1U, map1.size()) << "Size of skiplist after 1 emplace is not 1";
    EXPECT_NE(map1.end(), map1.find(6U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), map1.find(6U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), map1.cbegin()->value) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("foo"), map1.crbegin()->value) << "Just emplaced key & value is not ordered";
    auto result2 = map1.emplace(9U, "bar");
    EXPECT_TRUE(std::get<1>(result2)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result2)) << "Emplace failed";
    EXPECT_EQ(std::string("bar"), std::get<0>(result2)->value) << "Emplace failed";
    EXPECT_EQ(2U, map1.size()) << "Size of skiplist after 2 emplace is not 2";
    EXPECT_NE(map1.end(), map1.find(9U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), map1.find(9U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), map1.cbegin()->value) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), map1.crbegin()->value) << "Just emplaced key & value is not ordered";
    auto result3 = map1.emplace(20U, "blah");
    EXPECT_TRUE(std::get<1>(result3)) << "Emplace failed";
    EXPECT_NE(map1.end(), std::get<0>(result3)) << "Emplace failed";
    EXPECT_EQ(std::string("blah"), std::get<0>(result3)->value) << "Emplace failed";
    EXPECT_EQ(3U, map1.size()) << "Size of skiplist after 3 emplace is not 3";
    EXPECT_NE(map1.end(), map1.find(20U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("blah"), map1.find(20U)->value) << "Could not find just emplaced key & value";
    auto iter1 = map1.cbegin();
    EXPECT_EQ(std::string("foo"), iter1->value) << "Just emplaced key & value is not ordered";
    ++iter1;
    EXPECT_EQ(std::string("bar"), iter1->value) << "Just emplaced key & value is not ordered";
    ++iter1;
    EXPECT_EQ(std::string("blah"), iter1->value) << "Just emplaced key & value is not ordered";
    auto iter2 = map1.crbegin();
    EXPECT_EQ(std::string("blah"), iter2->value) << "Just emplaced key & value is not ordered";
    ++iter2;
    EXPECT_EQ(std::string("bar"), iter2->value) << "Just emplaced key & value is not ordered";
    ++iter2;
    EXPECT_EQ(std::string("foo"), iter2->value) << "Just emplaced key & value is not ordered";
}

TEST(emplacing_skiplist_test, emplace_duplicate)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U}, {string_map::node_sizes[2], 8U} });
    string_map map1(allocator1);
    map1.emplace(99U, "bar");
    map1.emplace(60U, "foo");
    auto result1 = map1.emplace(60U, "foo");
    EXPECT_FALSE(std::get<1>(result1)) << "Emplace of duplicate succeeded";
    EXPECT_EQ(map1.begin(), std::get<0>(result1)) << "Emplace of duplicate succeeded";
    EXPECT_EQ(2U, map1.size()) << "Size of skiplist after failed emplace changed";
    EXPECT_EQ(std::string("foo"), map1.find(60U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), map1.cbegin()->value) << "Just emplaced ikey & value is not ordered";
    EXPECT_EQ(std::string("bar"), map1.crbegin()->value) << "Just emplaced key & value is not ordered";
    auto result2 = map1.emplace(99U, "blah");
    EXPECT_FALSE(std::get<1>(result2)) << "Emplace of duplicate succeeded";
    auto iter = map1.begin();
    ++iter;
    EXPECT_EQ(iter, std::get<0>(result2)) << "Emplace of duplicate succeeded";
    EXPECT_EQ(std::string("bar"), std::get<0>(result2)->value) << "Emplace of duplicate succeeded";
    EXPECT_EQ(2U, map1.size()) << "Size of skiplist after failed emplace changed";
    EXPECT_EQ(std::string("bar"), map1.find(99U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), map1.cbegin()->value) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), map1.crbegin()->value) << "Just emplaced key & value is not ordered";
}

TEST(emplacing_skiplist_test, emplace_many)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::uint32_t, tme::pool> uint_map;
    tme::pool allocator1(
	    8U,
	    {
		{uint_map::node_sizes[0], std::numeric_limits<std::uint16_t>::max()},
		{uint_map::node_sizes[1], std::numeric_limits<std::uint16_t>::max()},
		{uint_map::node_sizes[2], std::numeric_limits<std::uint16_t>::max()}
	    });
    uint_map map1(allocator1);
    std::random_device device;
    for (std::uint32_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	std::uint32_t value = device() >> 16U;
	map1.emplace(value, value);
	//EXPECT_EQ(value, map1.find(value)->value) << "Could not find just emplaced key & value";
    }
}
