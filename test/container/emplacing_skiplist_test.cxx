#include <turbo/container/emplacing_skiplist.hpp>
#include <turbo/container/emplacing_skiplist.hxx>
#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;
namespace tme = turbo::memory;

TEST(emplacing_skiplist_test, empty_skiplist)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map list1(allocator1);
    EXPECT_EQ(list1.end(), list1.begin()) << "In an empty skiplist begin and end iterators are not equal";
}

TEST(emplacing_skiplist_test, find_invalid)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map list1(allocator1);
    EXPECT_EQ(list1.end(), list1.find(99U)) << "Search for non-existant key did not return empty iterator";
}

TEST(emplacing_skiplist_test, emplace_basic)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map list1(allocator1);
    auto result1 = list1.emplace(6U, "foo");
    EXPECT_TRUE(std::get<1>(result1)) << "Emplace failed";
    EXPECT_NE(list1.end(), std::get<0>(result1)) << "Emplace failed";
    EXPECT_EQ(std::string("foo"), std::get<0>(result1)->value) << "Emplace failed";
    EXPECT_NE(list1.end(), list1.find(6U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), list1.find(6U)->value) << "Could not find just emplaced key & value";
}
