#include <turbo/container/concurrent_unordered_map.hpp>
#include <turbo/container/concurrent_unordered_map.hh>
#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hh>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;

TEST(concurrent_unordered_map_test, iterator_basic)
{
    turbo::memory::concurrent_sized_slab allocator1(16U, { {sizeof(std::pair<std::string, std::uint8_t>), 32U} });
    typedef tco::concurrent_unordered_map<std::string, std::uint8_t> person_age_map;
    person_age_map map1(allocator1);
    auto iter = map1.begin();
    auto citer = map1.cbegin();
    ++iter;
    ++citer;
}

TEST(concurrent_unordered_map_test, emplace_basic)
{
    turbo::memory::concurrent_sized_slab allocator1(16U, { {sizeof(std::pair<std::string, std::uint8_t>), 32U} });
    typedef tco::concurrent_unordered_map<std::string, std::uint8_t> person_age_map;
    person_age_map map1(allocator1);
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("steve"), std::make_tuple(45)))
	    << "Emplace failed";
    auto iter1a = map1.find("steve");
    EXPECT_NE(map1.end(), iter1a) << "Could not find just emplaced value";
    EXPECT_EQ((*iter1a)->first, std::string("steve")) << "Find returned the wrong value";
    EXPECT_EQ((*iter1a)->second, 45) << "Find returned the wrong value";
    EXPECT_EQ(person_age_map::emplace_result::key_exists, map1.try_emplace(std::make_tuple("steve"), std::make_tuple(10)))
	    << "Duplicate key detection failed";
}

TEST(concurrent_unordered_map_test, erase_basic)
{
    turbo::memory::concurrent_sized_slab allocator1(16U, { {sizeof(std::pair<std::string, std::uint8_t>), 32U} });
    typedef tco::concurrent_unordered_map<std::string, std::uint8_t> person_age_map;
    person_age_map map1(allocator1, 2U);
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("a"), std::make_tuple(1)))
	    << "Emplace failed";
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("b"), std::make_tuple(2)))
	    << "Emplace failed";
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("c"), std::make_tuple(3)))
	    << "Emplace failed";
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("d"), std::make_tuple(4)))
	    << "Emplace failed";
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("e"), std::make_tuple(5)))
	    << "Emplace failed";
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("f"), std::make_tuple(6)))
	    << "Emplace failed";
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("g"), std::make_tuple(7)))
	    << "Emplace failed";
    EXPECT_EQ(person_age_map::emplace_result::success, map1.try_emplace(std::make_tuple("h"), std::make_tuple(8)))
	    << "Emplace failed";
    EXPECT_NE(map1.end(), map1.find("c")) << "Could not find just emplaced value";
    EXPECT_EQ(person_age_map::erase_result::success, map1.erase("c"))
	    << "Erase failed";
    EXPECT_NE(map1.end(), map1.find("a")) << "Could not find key a";
    EXPECT_NE(map1.end(), map1.find("b")) << "Could not find key b";
    EXPECT_NE(map1.end(), map1.find("d")) << "Could not find key d";
    EXPECT_NE(map1.end(), map1.find("e")) << "Could not find key e";
    EXPECT_NE(map1.end(), map1.find("f")) << "Could not find key f";
    EXPECT_NE(map1.end(), map1.find("g")) << "Could not find key g";
    EXPECT_NE(map1.end(), map1.find("h")) << "Could not find key h";
    EXPECT_EQ(person_age_map::erase_result::key_not_found, map1.erase("c"))
	    << "Non-existing key detection failed";
}
