#include <turbo/container/emplacing_skiplist.hpp>
#include <turbo/container/emplacing_skiplist.hxx>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
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
    template <class key_arg_t, class... value_args_t>
    inline std::tuple<typename skiplist_type::iterator, bool> emplace(std::int64_t chosen_height, const key_arg_t& key_arg, value_args_t&&... value_args)
    {
	return skiplist_.emplace(chosen_height, std::forward<decltype(key_arg)>(key_arg), std::forward<value_args_t>(value_args)...);
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

TEST(emplacing_skiplist_test, find_basic)
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
    typename uint_tester::store::iterator record1 = tester1.get_store().emplace_back(1U, 1U);
    typename uint_tester::store::iterator record3 = tester1.get_store().emplace_back(3U, 3U);
    typename uint_tester::store::iterator record5 = tester1.get_store().emplace_back(5U, 5U);
    typename uint_tester::store::iterator record6 = tester1.get_store().emplace_back(6U, 6U);
    typename uint_tester::store::iterator record7 = tester1.get_store().emplace_back(7U, 7U);
    typename uint_tester::store::iterator record9 = tester1.get_store().emplace_back(9U, 9U);
    typename uint_tester::floor& floor0 = *(tester1.get_tower().begin());
    typename uint_tester::floor::iterator room01 = floor0.emplace_back(
	    1U,
	    record1.share(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor::iterator room03 = floor0.emplace_back(
	    3U,
	    record3.share(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor::iterator room05 = floor0.emplace_back(
	    5U,
	    record5.share(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor::iterator room07 = floor0.emplace_back(
	    7U,
	    record7.share(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor::iterator room09 = floor0.emplace_back(
	    9U,
	    record9.share(),
	    std::weak_ptr<typename uint_tester::floor::iterator::node_type>());
    typename uint_tester::floor& floor1 = *(tester1.get_tower().rbegin());
    typename uint_tester::floor::iterator room13 = floor1.emplace_back(
	    3U,
	    record3.share(),
	    room03.share());
    typename uint_tester::floor::iterator room17 = floor1.emplace_back(
	    7U,
	    record7.share(),
	    room07.share());
    const typename uint_tester::floor::iterator empty;
    EXPECT_EQ(record1, map1.find(1U)) << "Search for existing key 1 failed";
    EXPECT_EQ(tester1.get_store().end(), map1.find(2U)) << "Search for non-existant key 2 failed";
    EXPECT_EQ(record3, map1.find(3U)) << "Search for existing key 3 failed";
    EXPECT_EQ(tester1.get_store().end(), map1.find(4U)) << "Search for non-existant key 4 failed";
    EXPECT_EQ(record5, map1.find(5U)) << "Search for existing key 5 failed";
    EXPECT_EQ(record6, map1.find(6U)) << "Search for existing key 6 failed";
    EXPECT_EQ(record7, map1.find(7U)) << "Search for existing key 7 failed";
    EXPECT_EQ(tester1.get_store().end(), map1.find(8U)) << "Search for non-existant key 8 failed";
    EXPECT_EQ(record9, map1.find(9U)) << "Search for existing key 9 failed";
    typename uint_tester::store::iterator record10 = tester1.get_store().emplace_back(10U, 10U);
    EXPECT_EQ(record10, map1.find(10U)) << "Search for existing key 10 failed";
}

TEST(emplacing_skiplist_test, emplace_tower_validation)
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
    typename uint_tester::store::iterator nearest_record;
    typename uint_tester::store::iterator next_record;
    typename uint_tester::floor::iterator nearest_room;
    typename uint_tester::floor::iterator next_room;
    typename uint_tester::store::iterator record4;
    bool result4 = false;
    std::tie(record4, result4) = tester1.emplace(-1, 4U, 4U);
    std::tie(nearest_record, next_record) = tester1.search_store(4U, tester1.get_store().begin());
    EXPECT_EQ(true, result4) << "Emplace to empty skiplist when height is -1 failed";
    EXPECT_EQ(record4, nearest_record) << "Emplace to empty skiplist when height is -1 failed";
    EXPECT_EQ(tester1.get_store().end(), next_record) << "Emplace to empty skiplist when height is -1 failed";
    typename uint_tester::store::iterator record5;
    bool result5 = false;
    std::tie(record5, result5) = tester1.emplace(0, 5U, 5U);
    std::tie(nearest_record, next_record) = tester1.search_store(5U, tester1.get_store().begin());
    EXPECT_EQ(true, result5) << "Emplace to skiplist with empty tower when height is 0 failed";
    EXPECT_EQ(record5, nearest_record) << "Emplace to skiplist with empty tower when height is 0 failed";
    EXPECT_EQ(tester1.get_store().end(), next_record) << "Emplace to skiplist with empty tower when height is 0 failed";
    typename uint_tester::floor& floor0 = *(tester1.get_tower().begin());
    std::tie(nearest_room, next_room) = tester1.search_floor(5U, floor0.begin());
    EXPECT_NE(floor0.end(), nearest_room) << "Emplace to skiplist with empty tower when height is 0 failed";
    EXPECT_EQ(5U, nearest_room->key) << "Emplace to skiplist with empty tower when height is 0 failed";
    EXPECT_EQ(floor0.end(), next_room) << "Emplace to skiplist with empty tower when height is 0 failed";
    typename uint_tester::store::iterator record3;
    bool result3 = false;
    std::tie(record3, result3) = tester1.emplace(1, 3U, 3U);
    std::tie(nearest_record, next_record) = tester1.search_store(3U, tester1.get_store().begin());
    EXPECT_EQ(true, result3) << "Emplace to skiplist with empty top floor when height is 1 failed";
    EXPECT_EQ(record3, nearest_record) << "Emplace to skiplist with empty top floor when height is 1 failed";
    EXPECT_EQ(record4, next_record) << "Emplace to skiplist with empty top floor when height is 1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(3U, floor0.begin());
    EXPECT_NE(floor0.end(), nearest_room) << "Emplace to skiplist with empty top floor when height is 1 failed";
    EXPECT_EQ(3U, nearest_room->key) << "Emplace to skiplist with empty top floor when height is 1 failed";
    EXPECT_NE(floor0.end(), next_room) << "Emplace to skiplist with empty top floor when height is 1 failed";
    EXPECT_EQ(5U, next_room->key) << "Emplace to skiplist with empty top floor when height is 1 failed";
    typename uint_tester::floor& floor1 = *(tester1.get_tower().rbegin());
    std::tie(nearest_room, next_room) = tester1.search_floor(3U, floor1.begin());
    EXPECT_NE(floor1.end(), nearest_room) << "Emplace to skiplist with empty top floor when height is 1 failed";
    EXPECT_EQ(3U, nearest_room->key) << "Emplace to skiplist with empty top floor when height is 1 failed";
    EXPECT_EQ(floor1.end(), next_room) << "Emplace to skiplist with empty top floor when height is 1 failed";
    typename uint_tester::store::iterator record8;
    bool result8 = false;
    std::tie(record8, result8) = tester1.emplace(-1, 8U, 8U);
    std::tie(nearest_record, next_record) = tester1.search_store(8U, tester1.get_store().begin());
    EXPECT_EQ(true, result8) << "Emplace to end of skiplist when height is -1 failed";
    EXPECT_EQ(record8, nearest_record) << "Emplace to end of skiplist when height is -1 failed";
    EXPECT_EQ(tester1.get_store().end(), next_record) << "Emplace to end of skiplist when height is -1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(8U, floor0.begin());
    EXPECT_NE(floor0.end(), nearest_room) << "Emplace to end of skiplist when height is -1 failed";
    EXPECT_NE(8U, nearest_room->key) << "Emplace to end of skiplist when height is -1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(8U, floor1.begin());
    EXPECT_NE(floor1.end(), nearest_room) << "Emplace to end of skiplist when height is -1 failed";
    EXPECT_NE(8U, nearest_room->key) << "Emplace to end of skiplist when height is -1 failed";
    typename uint_tester::store::iterator record9;
    bool result9 = false;
    std::tie(record9, result9) = tester1.emplace(0, 9U, 9U);
    std::tie(nearest_record, next_record) = tester1.search_store(9U, tester1.get_store().begin());
    EXPECT_EQ(true, result9) << "Emplace to back of bottom floor when height is 0 failed";
    EXPECT_EQ(record9, nearest_record) << "Emplace to back of bottom floor when height is 0 failed";
    EXPECT_EQ(tester1.get_store().end(), next_record) << "Emplace to back of bottom floor when height is 0 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(9U, floor0.begin());
    EXPECT_NE(floor0.end(), nearest_room) << "Emplace to back of bottom floor when height is 0 failed";
    EXPECT_EQ(9U, nearest_room->key) << "Emplace to back of bottom floor when height is 0 failed";
    EXPECT_EQ(floor0.end(), next_room) << "Emplace to back of bottom floor when height is 0 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(9U, floor1.begin());
    EXPECT_NE(floor1.end(), nearest_room) << "Emplace to back of bottom floor when height is 0 failed";
    EXPECT_NE(9U, nearest_room->key) << "Emplace to back of bottom floor when height is 0 failed";
    typename uint_tester::store::iterator record7;
    bool result7 = false;
    std::tie(record7, result7) = tester1.emplace(1, 7U, 7U);
    std::tie(nearest_record, next_record) = tester1.search_store(7U, tester1.get_store().begin());
    EXPECT_EQ(true, result7) << "Emplace to back of top floor when height is 1 failed";
    EXPECT_EQ(record7, nearest_record) << "Emplace to back of top floor when height is 1 failed";
    EXPECT_EQ(record8, next_record) << "Emplace to back of top floor when height is 1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(7U, floor0.begin());
    EXPECT_NE(floor0.end(), nearest_room) << "Emplace to back of top floor when height is 1 failed";
    EXPECT_EQ(7U, nearest_room->key) << "Emplace to back of top floor when height is 1 failed";
    EXPECT_NE(floor0.end(), next_room) << "Emplace to back of top floor when height is 1 failed";
    EXPECT_EQ(9U, next_room->key) << "Emplace to back of top floor when height is 1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(7U, floor1.begin());
    EXPECT_NE(floor1.end(), nearest_room) << "Emplace to back of top floor when height is 1 failed";
    EXPECT_EQ(7U, nearest_room->key) << "Emplace to back of top floor when height is 1 failed";
    EXPECT_EQ(floor1.end(), next_room) << "Emplace to back of top floor when height is 1 failed";
    typename uint_tester::store::iterator record2;
    bool result2 = false;
    std::tie(record2, result2) = tester1.emplace(-1, 2U, 2U);
    std::tie(nearest_record, next_record) = tester1.search_store(2U, tester1.get_store().begin());
    EXPECT_EQ(true, result2) << "Emplace to front of non-empty store when height is -1 failed";
    EXPECT_EQ(record2, nearest_record) << "Emplace to front of non-empty store when height is -1 failed";
    EXPECT_EQ(record3, next_record) << "Emplace to front of non-empty store when height is -1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(2U, floor0.begin());
    EXPECT_EQ(floor0.end(), nearest_room) << "Emplace to front of non-empty store when height is -1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(2U, floor1.begin());
    EXPECT_EQ(floor1.end(), nearest_room) << "Emplace to front of non-empty store when height is -1 failed";
    typename uint_tester::store::iterator record1;
    bool result1 = false;
    std::tie(record1, result1) = tester1.emplace(0, 1U, 1U);
    std::tie(nearest_record, next_record) = tester1.search_store(1U, tester1.get_store().begin());
    EXPECT_EQ(true, result1) << "Emplace to front of bottom floor when height is 0 failed";
    EXPECT_EQ(record1, nearest_record) << "Emplace to front of bottom floor when height is 0 failed";
    EXPECT_EQ(record2, next_record) << "Emplace to front of bottom floor when height is 0 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(1U, floor0.begin());
    EXPECT_NE(floor0.end(), nearest_room) << "Emplace to front of bottom floor when height is 0 failed";
    EXPECT_EQ(1U, nearest_room->key) << "Emplace to front of bottom floor when height is 0 failed";
    EXPECT_NE(floor0.end(), next_room) << "Emplace to front of bottom floor when height is 0 failed";
    EXPECT_EQ(3U, next_room->key) << "Emplace to front of bottom floor when height is 0 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(1U, floor1.begin());
    EXPECT_EQ(floor1.end(), nearest_room) << "Emplace to front of bottom floor when height is 0 failed";
    typename uint_tester::store::iterator record6;
    bool result6 = false;
    std::tie(record6, result6) = tester1.emplace(-1, 6U, 6U);
    std::tie(nearest_record, next_record) = tester1.search_store(6U, tester1.get_store().begin());
    EXPECT_EQ(true, result6) << "Emplace to middle of non-empty store when height is -1 failed";
    EXPECT_EQ(record6, nearest_record) << "Emplace to middle of non-empty store when height is -1 failed";
    EXPECT_EQ(record7, next_record) << "Emplace to middle of non-empty store when height is -1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(6U, floor0.begin());
    EXPECT_NE(floor0.end(), nearest_room) << "Emplace to middle of non-empty store when height is -1 failed";
    EXPECT_NE(6U, nearest_room->key) << "Emplace to middle of non-empty store when height is -1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(6U, floor1.begin());
    EXPECT_NE(floor1.end(), nearest_room) << "Emplace to middle of non-empty store when height is -1 failed";
    EXPECT_NE(6U, nearest_room->key) << "Emplace to middle of non-empty store when height is -1 failed";
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

TEST(emplacing_skiplist_test, erase_tower_validation)
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
    const typename uint_tester::floor::iterator empty_room;
    typename uint_tester::floor::iterator nearest_room;
    typename uint_tester::floor::iterator next_room;
    typename uint_tester::store::iterator nearest_record;
    typename uint_tester::store::iterator next_record;
    typename uint_map::iterator record1 = std::get<0U>(tester1.emplace(0, 1U, 1U));
    typename uint_map::iterator record2 = std::get<0U>(tester1.emplace(-1, 2U, 2U));
    typename uint_map::iterator record3 = std::get<0U>(tester1.emplace(1, 3U, 3U));
    typename uint_map::iterator record4 = std::get<0U>(tester1.emplace(-1, 4U, 4U));
    typename uint_map::iterator record5 = std::get<0U>(tester1.emplace(0, 5U, 5U));
    typename uint_map::iterator record6 = std::get<0U>(tester1.emplace(-1, 6U, 6U));
    typename uint_map::iterator record7 = std::get<0U>(tester1.emplace(1, 7U, 7U));
    typename uint_map::iterator record8 = std::get<0U>(tester1.emplace(-1, 8U, 8U));
    typename uint_map::iterator record9 = std::get<0U>(tester1.emplace(0, 9U, 9U));
    typename uint_tester::floor& floor0 = *(tester1.get_tower().begin());
    typename uint_tester::floor& floor1 = *(tester1.get_tower().rbegin());
    // Erase 6
    EXPECT_EQ(record7, map1.erase(6U)) << "Erase of key 6 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(6U, floor1.begin());
    EXPECT_NE(empty_room, nearest_room) << "Erase of key 6 failed";
    EXPECT_NE(empty_room, next_room) << "Erase of key 6 failed";
    EXPECT_EQ(3U, nearest_room->key) << "Erase of key 6 failed";
    EXPECT_EQ(7U, next_room->key) << "Erase of key 6 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(6U, floor0.begin());
    EXPECT_NE(empty_room, nearest_room) << "Erase of key 6 failed";
    EXPECT_NE(empty_room, next_room) << "Erase of key 6 failed";
    EXPECT_EQ(5U, nearest_room->key) << "Erase of key 6 failed";
    EXPECT_EQ(7U, next_room->key) << "Erase of key 6 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(6U, tester1.get_store().begin());
    EXPECT_NE(tester1.get_store().end(), nearest_record) << "Erase of key 6 failed";
    EXPECT_NE(tester1.get_store().end(), next_record) << "Erase of key 6 failed";
    EXPECT_EQ(5U, nearest_record->key) << "Erase of key 6 failed";
    EXPECT_EQ(7U, next_record->key) << "Erase of key 6 failed";
    // Erase 1
    EXPECT_EQ(record2, map1.erase(1U)) << "Erase of key 1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(1U, floor1.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 1 failed";
    EXPECT_NE(empty_room, next_room) << "Erase of key 1 failed";
    EXPECT_EQ(3U, next_room->key) << "Erase of key 1 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(1U, floor0.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 1 failed";
    EXPECT_NE(empty_room, next_room) << "Erase of key 1 failed";
    EXPECT_EQ(3U, next_room->key) << "Erase of key 1 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(1U, tester1.get_store().begin());
    EXPECT_EQ(tester1.get_store().end(), nearest_record) << "Erase of key 1 failed";
    EXPECT_NE(tester1.get_store().end(), next_record) << "Erase of key 1 failed";
    EXPECT_EQ(2U, next_record->key) << "Erase of key 1 failed";
    // Erase 2
    EXPECT_EQ(record3, map1.erase(2U)) << "Erase of key 2 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(2U, floor1.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 2 failed";
    EXPECT_NE(empty_room, next_room) << "Erase of key 2 failed";
    EXPECT_EQ(3U, next_room->key) << "Erase of key 2 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(2U, floor0.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 2 failed";
    EXPECT_NE(empty_room, next_room) << "Erase of key 2 failed";
    EXPECT_EQ(3U, next_room->key) << "Erase of key 2 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(2U, tester1.get_store().begin());
    EXPECT_EQ(tester1.get_store().end(), nearest_record) << "Erase of key 2 failed";
    EXPECT_NE(tester1.get_store().end(), next_record) << "Erase of key 2 failed";
    EXPECT_EQ(3U, next_record->key) << "Erase of key 2 failed";
    // Erase 7
    EXPECT_EQ(record8, map1.erase(7U)) << "Erase of key 7 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(7U, floor1.begin());
    EXPECT_NE(empty_room, nearest_room) << "Erase of key 7 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 7 failed";
    EXPECT_EQ(3U, nearest_room->key) << "Erase of key 7 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(7U, floor0.begin());
    EXPECT_NE(empty_room, nearest_room) << "Erase of key 7 failed";
    EXPECT_NE(empty_room, next_room) << "Erase of key 7 failed";
    EXPECT_EQ(5U, nearest_room->key) << "Erase of key 7 failed";
    EXPECT_EQ(9U, next_room->key) << "Erase of key 7 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(7U, tester1.get_store().begin());
    EXPECT_NE(tester1.get_store().end(), nearest_record) << "Erase of key 7 failed";
    EXPECT_NE(tester1.get_store().end(), next_record) << "Erase of key 7 failed";
    EXPECT_EQ(5U, nearest_record->key) << "Erase of key 7 failed";
    EXPECT_EQ(8U, next_record->key) << "Erase of key 7 failed";
    // Erase 9
    EXPECT_EQ(tester1.get_store().end(), map1.erase(9U)) << "Erase of key 9 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(9U, floor1.begin());
    EXPECT_NE(empty_room, nearest_room) << "Erase of key 9 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 9 failed";
    EXPECT_EQ(3U, nearest_room->key) << "Erase of key 9 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(9U, floor0.begin());
    EXPECT_NE(empty_room, nearest_room) << "Erase of key 9 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 9 failed";
    EXPECT_EQ(5U, nearest_room->key) << "Erase of key 9 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(9U, tester1.get_store().begin());
    EXPECT_NE(tester1.get_store().end(), nearest_record) << "Erase of key 9 failed";
    EXPECT_EQ(tester1.get_store().end(), next_record) << "Erase of key 9 failed";
    EXPECT_EQ(8U, nearest_record->key) << "Erase of key 9 failed";
    // Erase 8
    EXPECT_EQ(tester1.get_store().end(), map1.erase(8U)) << "Erase of key 8 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(8U, floor1.begin());
    EXPECT_NE(empty_room, nearest_room) << "Erase of key 8 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 8 failed";
    EXPECT_EQ(3U, nearest_room->key) << "Erase of key 8 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(8U, floor0.begin());
    EXPECT_NE(empty_room, nearest_room) << "Erase of key 8 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 8 failed";
    EXPECT_EQ(5U, nearest_room->key) << "Erase of key 8 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(8U, tester1.get_store().begin());
    EXPECT_NE(tester1.get_store().end(), nearest_record) << "Erase of key 8 failed";
    EXPECT_EQ(tester1.get_store().end(), next_record) << "Erase of key 8 failed";
    EXPECT_EQ(5U, nearest_record->key) << "Erase of key 8 failed";
    // Erase 3
    EXPECT_EQ(record4, map1.erase(3U)) << "Erase of key 3 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(3U, floor1.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 3 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 3 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(3U, floor0.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 3 failed";
    EXPECT_NE(empty_room, next_room) << "Erase of key 3 failed";
    EXPECT_EQ(5U, next_room->key) << "Erase of key 3 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(3U, tester1.get_store().begin());
    EXPECT_EQ(tester1.get_store().end(), nearest_record) << "Erase of key 3 failed";
    EXPECT_NE(tester1.get_store().end(), next_record) << "Erase of key 3 failed";
    EXPECT_EQ(4U, next_record->key) << "Erase of key 3 failed";
    // Erase 5
    EXPECT_EQ(tester1.get_store().end(), map1.erase(5U)) << "Erase of key 5 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(5U, floor1.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 5 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 5 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(5U, floor0.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 5 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 5 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(5U, tester1.get_store().begin());
    EXPECT_NE(tester1.get_store().end(), nearest_record) << "Erase of key 5 failed";
    EXPECT_EQ(tester1.get_store().end(), next_record) << "Erase of key 5 failed";
    EXPECT_EQ(4U, nearest_record->key) << "Erase of key 5 failed";
    // Erase 4
    EXPECT_EQ(tester1.get_store().end(), map1.erase(4U)) << "Erase of key 4 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(4U, floor1.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 4 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 4 failed";
    std::tie(nearest_room, next_room) = tester1.search_floor(4U, floor0.begin());
    EXPECT_EQ(empty_room, nearest_room) << "Erase of key 4 failed";
    EXPECT_EQ(empty_room, next_room) << "Erase of key 4 failed";
    std::tie(nearest_record, next_record) = tester1.search_store(4U, tester1.get_store().begin());
    EXPECT_EQ(tester1.get_store().end(), nearest_record) << "Erase of key 4 failed";
    EXPECT_EQ(tester1.get_store().end(), next_record) << "Erase of key 4 failed";
}

TEST(emplacing_skiplist_test, erase_basic)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U}, {string_map::node_sizes[2], 8U} });
    string_map map1(allocator1);
    string_map::iterator iter;
    string_map::iterator record6 = std::get<0U>(map1.emplace(6U, "foo"));
    string_map::iterator record9 = std::get<0U>(map1.emplace(9U, "bar"));
    string_map::iterator record20 = std::get<0U>(map1.emplace(20U, "blah"));
    string_map::iterator record45 = std::get<0U>(map1.emplace(45U, "woot"));
    EXPECT_EQ(record20, map1.erase(9U)) << "Erase of key 9 failed";
    iter = map1.find(6U);
    ++iter;
    EXPECT_NE(map1.end(), iter) << "Erase of key 9 failed";
    EXPECT_EQ(iter->key, 20U) << "Erase of key 9 failed";
    EXPECT_EQ(record45, map1.erase(20U)) << "Erase of key 20 failed";
    iter = map1.find(6U);
    ++iter;
    EXPECT_NE(map1.end(), iter) << "Erase of key 20 failed";
    EXPECT_EQ(iter->key, 45U) << "Erase of key 20 failed";
    EXPECT_EQ(record45, map1.erase(6U)) << "Erase of key 6 failed";
    iter = map1.begin();
    EXPECT_NE(map1.end(), iter) << "Erase of key 6 failed";
    EXPECT_EQ(iter->key, 45U) << "Erase of key 6 failed";
    EXPECT_EQ(map1.end(), map1.erase(45U)) << "Erase of key 45 failed";
    iter = map1.find(45U);
    EXPECT_EQ(map1.end(), iter) << "Erase of key 45 failed";
}

TEST(emplacing_skiplist_test, erase_invalid)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U}, {string_map::node_sizes[2], 8U} });
    string_map map1(allocator1);
    string_map::iterator iter;
    string_map::iterator record6 = std::get<0U>(map1.emplace(6U, "foo"));
    string_map::iterator record9 = std::get<0U>(map1.emplace(9U, "bar"));
    string_map::iterator record20 = std::get<0U>(map1.emplace(20U, "blah"));
    string_map::iterator record45 = std::get<0U>(map1.emplace(45U, "woot"));
    EXPECT_EQ(record20, map1.erase(9U)) << "Erase of key 9 failed";
    iter = map1.find(6U);
    ++iter;
    EXPECT_NE(map1.end(), iter) << "Erase of key 9 failed";
    EXPECT_EQ(iter->key, 20U) << "Erase of key 9 failed";
    EXPECT_EQ(record20, map1.erase(9U)) << "Erase of key 9 failed";
    iter = map1.find(6U);
    ++iter;
    EXPECT_NE(map1.end(), iter) << "Erase of key 9 failed";
    EXPECT_EQ(iter->key, 20U) << "Erase of key 9 failed";
}

TEST(emplacing_skiplist_test, perf_test_skiplist_emplace)
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
    }
}

TEST(emplacing_skiplist_test, perf_test_map_emplace)
{
    typedef std::map<std::uint32_t, std::uint32_t> uint_map;
    uint_map map1;
    std::random_device device;
    for (std::uint32_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max(); ++counter)
    {
	std::uint32_t value = device() >> 16U;
	map1.emplace(value, value);
    }
}
