#include <turbo/container/bitwise_trie.hpp>
#include <turbo/container/bitwise_trie.hxx>
#include <gtest/gtest.h>
#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>

namespace tco = turbo::container;
namespace tme = turbo::memory;

TEST(bitwise_trie_test, empty_trie)
{
    typedef tco::bitwise_trie<std::uint32_t, std::string, tme::pool> string_map;
    tme::pool allocator1(8U, { {string_map::node_sizes[0], 8U}, {string_map::node_sizes[1], 8U} });
    string_map map1(allocator1);
    EXPECT_EQ(0U, map1.size()) << "Size of an empty skiplist is not 0";
}