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

TEST(emplacing_skiplist_test, empty_list)
{
    typedef tco::emplacing_skiplist<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map list1(allocator1);
}
