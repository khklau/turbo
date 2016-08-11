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

TEST(block_test, invalid_construction)
{
    ASSERT_THROW(tme::block(0U, 3U, alignof(std::uint64_t)), tme::invalid_size_error);
    ASSERT_THROW(tme::block(sizeof(std::uint8_t), 2U, alignof(std::uint64_t)), tme::invalid_alignment_error);
}

TEST(block_test, allocate_basic)
{
    tme::block block1(sizeof(std::uint64_t), 3U, alignof(std::uint64_t));
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";

    tme::block block2(sizeof(std::uint64_t), 0U, alignof(std::uint64_t));
    EXPECT_EQ(nullptr, block2.allocate()) << "Empty block still allocated";
}

TEST(block_test, free_basic)
{
    tme::block block1(sizeof(std::uint16_t), 3U, alignof(std::uint64_t));
    void* result1 = block1.allocate();
    void* result2 = block1.allocate();
    void* result3 = block1.allocate();
    ASSERT_NO_THROW(block1.free(result1)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result2)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result3)) << "Free failed";
}

TEST(block_test, recycle_basic)
{
    tme::block block1(sizeof(std::uint32_t), 3U, alignof(std::uint64_t));
    void* result1 = block1.allocate();
    void* result2 = block1.allocate();
    EXPECT_NE(nullptr, result1) << "Allocation failed";
    EXPECT_NE(nullptr, result2) << "Allocation failed";
    ASSERT_NO_THROW(block1.free(result1)) << "Free failed";
    void* result3 = block1.allocate();
    void* result4 = block1.allocate();
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
    ASSERT_NO_THROW(block1.free(result2)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result3)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result4)) << "Free failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
}

struct record
{
    record();
    record(uint16_t f, uint32_t s, uint64_t t, void* o);
    record(const record& other);
    bool operator==(const record& other) const;
    uint16_t first;
    uint32_t second;
    uint64_t third;
    void* fourth;
};

record::record()
    :
	first(0U),
	second(0U),
	third(0U),
	fourth(nullptr)
{ }

record::record(uint16_t f, uint32_t s, uint64_t t, void* o)
    :
	first(f),
	second(s),
	third(t),
	fourth(o)
{ }

record::record(const record& other)
    :
	first(other.first),
	second(other.second),
	third(other.third),
	fourth(other.fourth)
{ }

bool record::operator==(const record& other) const
{
    return first == other.first && second == other.second && third == other.third && fourth == other.fourth;
}

TEST(block_test, recycle_struct)
{
    tme::block block1(sizeof(record), 3U, alignof(record));
    void* result1 = block1.allocate();
    void* result2 = block1.allocate();
    EXPECT_NE(nullptr, result1) << "Allocation failed";
    EXPECT_NE(nullptr, result2) << "Allocation failed";
    ASSERT_NO_THROW(block1.free(result1)) << "Free failed";
    void* result3 = block1.allocate();
    void* result4 = block1.allocate();
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
    ASSERT_NO_THROW(block1.free(result2)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result3)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result4)) << "Free failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
}

typedef std::array<char, 64> char_64_array;

TEST(block_test, recycle_array)
{
    tme::block block1(sizeof(char_64_array), 3U, alignof(char_64_array));
    void* result1 = block1.allocate();
    void* result2 = block1.allocate();
    EXPECT_NE(nullptr, result1) << "Allocation failed";
    EXPECT_NE(nullptr, result2) << "Allocation failed";
    ASSERT_NO_THROW(block1.free(result1)) << "Free failed";
    void* result3 = block1.allocate();
    void* result4 = block1.allocate();
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
    ASSERT_NO_THROW(block1.free(result2)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result3)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result4)) << "Free failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
}
