#include <turbo/memory/block.hpp>
#include <cstdint>
#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>

namespace tar = turbo::algorithm::recovery;
namespace tco = turbo::container;
namespace tme = turbo::memory;

TEST(block_test, allocate_basic)
{
    tme::block block1(sizeof(std::uint64_t), 3U, alignof(std::uint64_t));
    {
	EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
	EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
	EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
	EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
    }
}
