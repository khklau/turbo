#include <turbo/container/concurrent_unordered_map.hpp>
#include <turbo/container/concurrent_unordered_map.hxx>
#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hxx>

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
    EXPECT_TRUE(map1.try_emplace(std::make_tuple("steve"), std::make_tuple(45))) << "Emplace failed";
    auto iter1a = map1.find("steve");
    EXPECT_NE(map1.end(), iter1a) << "Could not find just emplaced value";
    EXPECT_EQ((*iter1a)->first, std::string("steve")) << "Find returned the wrong value";
    EXPECT_EQ((*iter1a)->second, 45) << "Find returned the wrong value";
}
