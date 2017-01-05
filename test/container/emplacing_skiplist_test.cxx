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
    typedef typename skiplist_type::floor_id floor_id;
    typedef typename skiplist_type::trace trace;
    emplacing_skiplist_tester(skiplist_type& skiplist)
	:
	    skiplist_(skiplist)
    { }
    inline typename skiplist_type::tower& get_tower()
    {
	return skiplist_.tower_;
    }
    inline typename skiplist_type::store& get_store()
    {
	return skiplist_.store_;
    }
    inline void grow_tower(typename skiplist_type::floor_id new_maximum)
    {
	skiplist_.grow_tower(new_maximum);
    }
    inline typename skiplist_type::floor_region search_floor(
	    const typename skiplist_type::key_type& key,
	    const typename skiplist_type::floor::iterator& iter)
    {
	return skiplist_.search_floor(std::forward<decltype(key)>(key), std::forward<decltype(iter)>(iter));
    }
    inline typename skiplist_type::store_region search_store(
	    const typename skiplist_type::key_type& key,
	    const typename skiplist_type::store::iterator& iter)
    {
	return skiplist_.search_store(std::forward<decltype(key)>(key), std::forward<decltype(iter)>(iter));
    }
    inline std::vector<typename skiplist_type::trace> trace_tower(const typename skiplist_type::key_type& key)
    {
	return skiplist_.trace_tower(std::forward<decltype(key)>(key));
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
    tester1.grow_tower(0U);
    typename uint_tester::floor& floor1 = *(tester1.get_tower().begin());
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
    typename uint_tester::store& store1 = tester1.get_store();
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

TEST(emplacing_skiplist_test, trace_tower_basic)
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
    tester1.grow_tower(1U);
    typename uint_tester::floor& floor0 = *(tester1.get_tower().begin());
    typename uint_tester::floor::iterator room01 = floor0.emplace_back(
	    1U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor::iterator room03 = floor0.emplace_back(
	    3U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor::iterator room05 = floor0.emplace_back(
	    5U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor::iterator room07 = floor0.emplace_back(
	    7U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor::iterator room09 = floor0.emplace_back(
	    9U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor& floor1 = *(tester1.get_tower().rbegin());
    typename uint_tester::floor::iterator room13 = floor1.emplace_back(
	    3U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>(room03.share()));
    typename uint_tester::floor::iterator room17 = floor1.emplace_back(
	    7U,
	    std::weak_ptr<typename uint_tester::store::iterator::node_type>(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>(room07.share()));
    const typename uint_tester::floor::iterator empty;
    std::vector<typename uint_tester::trace> trace0 = tester1.trace_tower(0U);
    EXPECT_EQ(2U, trace0.size()) << "Trace for key less than all rooms in the tower failed";
    EXPECT_EQ(empty, trace0[0].nearest) << "Trace for key less than all rooms in the tower failed";
    EXPECT_EQ(room13, trace0[0].next) << "Trace for key less than all rooms in the tower failed";
    EXPECT_EQ(empty, trace0[1].nearest) << "Trace for key less than all rooms in the tower failed";
    EXPECT_EQ(room01, trace0[1].next) << "Trace for key less than all rooms in the tower failed";
    std::vector<typename uint_tester::trace> trace1 = tester1.trace_tower(1U);
    EXPECT_EQ(2U, trace1.size()) << "Trace for left most lower floor room failed";
    EXPECT_EQ(empty, trace1[0].nearest) << "Trace for left most lower floor room failed";
    EXPECT_EQ(room13, trace1[0].next) << "Trace for left most lower floor room failed";
    EXPECT_EQ(room01, trace1[1].nearest) << "Trace for left most lower floor room failed";
    EXPECT_EQ(room03, trace1[1].next) << "Trace for left most lower floor room failed";
    std::vector<typename uint_tester::trace> trace2 = tester1.trace_tower(2U);
    EXPECT_EQ(2U, trace2.size()) << "Trace for key between left and centre-left rooms on lower floor failed";
    EXPECT_EQ(empty, trace2[0].nearest) << "Trace for key between left and centre-left rooms on lower floor failed";
    EXPECT_EQ(room13, trace2[0].next) << "Trace for key between left and centre-left rooms on lower floor failed";
    EXPECT_EQ(room01, trace2[1].nearest) << "Trace for key between left and centre-left rooms on lower floor failed";
    EXPECT_EQ(room03, trace2[1].next) << "Trace for key between left and centre-left rooms on lower floor failed";
    std::vector<typename uint_tester::trace> trace3 = tester1.trace_tower(3U);
    EXPECT_EQ(1U, trace3.size()) << "Trace for left room on upper floor failed";
    EXPECT_EQ(room13, trace3[0].nearest) << "Trace for left room on upper floor failed";
    EXPECT_EQ(room17, trace3[0].next) << "Trace for left room on upper floor failed";
    std::vector<typename uint_tester::trace> trace4 = tester1.trace_tower(4U);
    EXPECT_EQ(2U, trace4.size()) << "Trace for key between centre-left and centre rooms on lower floor failed";
    EXPECT_EQ(room13, trace4[0].nearest) << "Trace for key between centre-left and centre rooms on lower floor failed";
    EXPECT_EQ(room17, trace4[0].next) << "Trace for key between centre-left and centre rooms on lower floor failed";
    EXPECT_EQ(room03, trace4[1].nearest) << "Trace for key between centre-left and centre rooms on lower floor failed";
    EXPECT_EQ(room05, trace4[1].next) << "Trace for key between centre-left and centre rooms on lower floor failed";
    std::vector<typename uint_tester::trace> trace5 = tester1.trace_tower(5U);
    EXPECT_EQ(2U, trace5.size()) << "Trace for centre room on lower floor failed";
    EXPECT_EQ(room13, trace5[0].nearest) << "Trace for centre room on lower floor failed";
    EXPECT_EQ(room17, trace5[0].next) << "Trace for centre room on lower floor failed";
    EXPECT_EQ(room05, trace5[1].nearest) << "Trace for centre room on lower floor failed";
    EXPECT_EQ(room07, trace5[1].next) << "Trace for centre room on lower floor failed";
    std::vector<typename uint_tester::trace> trace6 = tester1.trace_tower(6U);
    EXPECT_EQ(2U, trace6.size()) << "Trace for key between centre and centre-right rooms on lower floor failed";
    EXPECT_EQ(room13, trace6[0].nearest) << "Trace for key between centre and centre-right rooms on lower floor failed";
    EXPECT_EQ(room17, trace6[0].next) << "Trace for key between centre and centre-right rooms on lower floor failed";
    EXPECT_EQ(room05, trace6[1].nearest) << "Trace for key between centre and centre-right rooms on lower floor failed";
    EXPECT_EQ(room07, trace6[1].next) << "Trace for key between centre and centre-right rooms on lower floor failed";
    std::vector<typename uint_tester::trace> trace7 = tester1.trace_tower(7U);
    EXPECT_EQ(1U, trace7.size()) << "Trace for right room on upper floor failed";
    EXPECT_EQ(room17, trace7[0].nearest) << "Trace for right room on upper floor failed";
    EXPECT_EQ(empty, trace7[0].next) << "Trace for right room on upper floor failed";
    std::vector<typename uint_tester::trace> trace8 = tester1.trace_tower(8U);
    EXPECT_EQ(2U, trace8.size()) << "Trace for key between centre-right and right rooms on lower floor failed";
    EXPECT_EQ(room17, trace8[0].nearest) << "Trace for key between centre-right and right rooms on lower floor failed";
    EXPECT_EQ(empty, trace8[0].next) << "Trace for key between centre-right and right rooms on lower floor failed";
    EXPECT_EQ(room07, trace8[1].nearest) << "Trace for key between centre-right and right rooms on lower floor failed";
    EXPECT_EQ(room09, trace8[1].next) << "Trace for key between centre-right and right rooms on lower floor failed";
    std::vector<typename uint_tester::trace> trace9 = tester1.trace_tower(9U);
    EXPECT_EQ(2U, trace9.size()) << "Trace for right room on lower floor failed";
    EXPECT_EQ(room17, trace9[0].nearest) << "Trace for right room on lower floor failed";
    EXPECT_EQ(empty, trace9[0].next) << "Trace for right room on lower floor failed";
    EXPECT_EQ(room09, trace9[1].nearest) << "Trace for right room on lower floor failed";
    EXPECT_EQ(empty, trace9[1].next) << "Trace for right room on lower floor failed";
    std::vector<typename uint_tester::trace> trace10 = tester1.trace_tower(10U);
    EXPECT_EQ(2U, trace10.size()) << "Trace for key greater than all of the rooms in the tower failed";
    EXPECT_EQ(room17, trace10[0].nearest) << "Trace for key greater than all of the rooms in the tower failed";
    EXPECT_EQ(empty, trace10[0].next) << "Trace for key greater than all of the rooms in the tower failed";
    EXPECT_EQ(room09, trace10[1].nearest) << "Trace for key greater than all of the rooms in the tower failed";
    EXPECT_EQ(empty, trace10[1].next) << "Trace for key greater than all of the rooms in the tower failed";
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
