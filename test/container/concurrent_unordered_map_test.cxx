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
    turbo::memory::cstdlib_typed_allocator allocator1;
    typedef tco::concurrent_unordered_map<std::string, std::uint8_t> person_age_map;
    person_age_map map1(allocator1);
    auto iter = map1.begin();
    auto citer = map1.cbegin();
    ++iter;
    ++citer;
}
