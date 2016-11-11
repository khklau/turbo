#include <turbo/container/emplacing_list.hpp>
#include <turbo/container/emplacing_list.hxx>
#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;
namespace tme = turbo::memory;

TEST(emplacing_list_test, emplace_front_basic)
{
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("foobar");
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is empty emplace_front failed";
}
