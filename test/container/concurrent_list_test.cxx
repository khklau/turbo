#include <turbo/container/concurrent_list.hpp>
#include <turbo/container/concurrent_list.hxx>
#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hxx>
#include <turbo/memory/slab_allocator.hpp>
#include <turbo/memory/slab_allocator.hxx>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;
namespace tme = turbo::memory;

TEST(concurrent_list_test, create_node_basic)
{
    typedef tco::concurrent_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(4U, { {string_list::node_size(), 4U} });
    string_list list1(allocator1);
    auto ptr1 = std::move(list1.create_node("foobar"));
    EXPECT_EQ(std::string("foobar"), ptr1->value) << "Node construction failed";
}
