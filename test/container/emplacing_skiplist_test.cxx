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
    EXPECT_EQ(0U, list1.size()) << "Size of an empty skiplist is not 0";
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
    EXPECT_EQ(1U, list1.size()) << "Size of skiplist after 1 emplace is not 1";
    EXPECT_NE(list1.end(), list1.find(6U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), list1.find(6U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), list1.cbegin()->value) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("foo"), list1.crbegin()->value) << "Just emplaced key & value is not ordered";
    auto result2 = list1.emplace(9U, "bar");
    EXPECT_TRUE(std::get<1>(result2)) << "Emplace failed";
    EXPECT_NE(list1.end(), std::get<0>(result2)) << "Emplace failed";
    EXPECT_EQ(std::string("bar"), std::get<0>(result2)->value) << "Emplace failed";
    EXPECT_EQ(2U, list1.size()) << "Size of skiplist after 2 emplace is not 2";
    EXPECT_NE(list1.end(), list1.find(9U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("bar"), list1.find(9U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), list1.cbegin()->value) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), list1.crbegin()->value) << "Just emplaced key & value is not ordered";
    auto result3 = list1.emplace(20U, "blah");
    EXPECT_TRUE(std::get<1>(result3)) << "Emplace failed";
    EXPECT_NE(list1.end(), std::get<0>(result3)) << "Emplace failed";
    EXPECT_EQ(std::string("blah"), std::get<0>(result3)->value) << "Emplace failed";
    EXPECT_EQ(3U, list1.size()) << "Size of skiplist after 3 emplace is not 3";
    EXPECT_NE(list1.end(), list1.find(20U)) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("blah"), list1.find(20U)->value) << "Could not find just emplaced key & value";
    auto iter1 = list1.cbegin();
    EXPECT_EQ(std::string("foo"), iter1->value) << "Just emplaced key & value is not ordered";
    ++iter1;
    EXPECT_EQ(std::string("bar"), iter1->value) << "Just emplaced key & value is not ordered";
    ++iter1;
    EXPECT_EQ(std::string("blah"), iter1->value) << "Just emplaced key & value is not ordered";
    auto iter2 = list1.crbegin();
    EXPECT_EQ(std::string("blah"), iter2->value) << "Just emplaced key & value is not ordered";
    ++iter2;
    EXPECT_EQ(std::string("bar"), iter2->value) << "Just emplaced key & value is not ordered";
    ++iter2;
    EXPECT_EQ(std::string("foo"), iter2->value) << "Just emplaced key & value is not ordered";
}

TEST(emplacing_skiplist_test, emplace_duplicate)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map list1(allocator1);
    list1.emplace(99U, "bar");
    list1.emplace(60U, "foo");
    auto result1 = list1.emplace(60U, "foo");
    EXPECT_FALSE(std::get<1>(result1)) << "Emplace of duplicate succeeded";
    EXPECT_EQ(list1.begin(), std::get<0>(result1)) << "Emplace of duplicate succeeded";
    EXPECT_EQ(2U, list1.size()) << "Size of skiplist after failed emplace changed";
    EXPECT_EQ(std::string("foo"), list1.find(60U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), list1.cbegin()->value) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), list1.crbegin()->value) << "Just emplaced key & value is not ordered";
    auto result2 = list1.emplace(99U, "blah");
    EXPECT_FALSE(std::get<1>(result2)) << "Emplace of duplicate succeeded";
    auto iter = list1.begin();
    ++iter;
    EXPECT_EQ(iter, std::get<0>(result2)) << "Emplace of duplicate succeeded";
    EXPECT_EQ(std::string("bar"), std::get<0>(result2)->value) << "Emplace of duplicate succeeded";
    EXPECT_EQ(2U, list1.size()) << "Size of skiplist after failed emplace changed";
    EXPECT_EQ(std::string("bar"), list1.find(99U)->value) << "Could not find just emplaced key & value";
    EXPECT_EQ(std::string("foo"), list1.cbegin()->value) << "Just emplaced key & value is not ordered";
    EXPECT_EQ(std::string("bar"), list1.crbegin()->value) << "Just emplaced key & value is not ordered";
}
