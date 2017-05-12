#include <turbo/cinterop/untyped_allocator.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <utility>

namespace turbo {
namespace cinterop {

class untyped_allocator_tester
{
public:
private:
};

} // namespace cinterop
} // namespace turbo

namespace tci = turbo::cinterop;
namespace tme = turbo::memory;

struct record
{
    record();
    record(uint16_t f, uint32_t s, uint64_t t);
    record(const record& other);
    record& operator=(const record& other);
    bool operator==(const record& other) const;
    bool operator<(const record& other) const;
    uint16_t first;
    uint32_t second;
    uint64_t third;
};

record::record()
    :
	first(0U),
	second(0U),
	third(0U)
{ }

record::record(uint16_t f, uint32_t s, uint64_t t)
    :
	first(f),
	second(s),
	third(t)
{ }

record::record(const record& other)
    :
	first(other.first),
	second(other.second),
	third(other.third)
{ }

record& record::operator=(const record& other)
{
    if (this != &other)
    {
	first = other.first;
	second = other.second;
	third = other.third;
    }
    return *this;
}

bool record::operator==(const record& other) const
{
    return first == other.first && second == other.second && third == other.third;
}

bool record::operator<(const record& other) const
{
    if (first != other.first)
    {
	return first < other.first;
    }
    else if (second != other.second)
    {
	return second < other.second;
    }
    else
    {
	return third < other.third;
    }
}

typedef std::array<uint8_t, 4> ipv4_address;

TEST(untyped_allocator_test, malloc_invalid)
{
    tci::untyped_allocator allocator1(2U, { {32U, 2U}, {64U, 2U} });
    EXPECT_EQ(nullptr, static_cast<record*>(allocator1.malloc(128U))) << "Malloc succeeded for an unsupported size";
    EXPECT_EQ(nullptr, static_cast<record*>(allocator1.malloc(0U))) << "Malloc succeeded for an unsupported size";
}

TEST(untyped_allocator_test, malloc_basic)
{
    tci::untyped_allocator allocator1(2U, { {sizeof(record), 2U}, {sizeof(ipv4_address), 2U} });
    record* record1 = static_cast<record*>(allocator1.malloc(sizeof(record)));
    EXPECT_TRUE(record1 != nullptr) << "Unexpected malloc failure";
    ipv4_address* address1 = static_cast<ipv4_address*>(allocator1.malloc(sizeof(ipv4_address)));
    EXPECT_TRUE(address1 != nullptr) << "Unexpected malloc failure";
}

TEST(untyped_allocator_test, free_invalid)
{
    tci::untyped_allocator allocator1(1U, { {sizeof(record), 1U}, {sizeof(ipv4_address), 1U} });
    record record1;
    EXPECT_NO_THROW(allocator1.free(&record1)) << "Free should just ignore invalid addresses";
}

TEST(untyped_allocator_test, free_basic)
{
    tci::untyped_allocator allocator1(1U, { {sizeof(record), 1U}, {sizeof(ipv4_address), 1U} });
    record* record1 = static_cast<record*>(allocator1.malloc(sizeof(record)));
    EXPECT_NE(nullptr, record1) << "Unexpected malloc failure";
    allocator1.free(record1);
    record* record2 = static_cast<record*>(allocator1.malloc(sizeof(record)));
    EXPECT_EQ(record1, record2) << "Allocator did not recycle the available memory slot";
}

TEST(untyped_allocator_test, pool_copy_construction_basic)
{
    tci::untyped_allocator allocator1(2U, { {sizeof(record), 2U}, {sizeof(ipv4_address), 2U} });
    record* record1 = static_cast<record*>(allocator1.malloc(sizeof(record)));
    EXPECT_NE(nullptr, record1) << "Unexpected malloc failure";
    new (record1) record(5U, 8741U, 20873U);
    ipv4_address* address1 = static_cast<ipv4_address*>(allocator1.malloc(sizeof(ipv4_address)));
    EXPECT_NE(nullptr, address1) << "Unexpected malloc failure";
    (*address1)[0] = 192U;
    (*address1)[1] = 168U;
    (*address1)[2] = 1U;
    (*address1)[3] = 254U;
    tci::untyped_allocator allocator2(allocator1);
    record* record2 = static_cast<record*>(allocator2.malloc(sizeof(record)));
    EXPECT_NE(nullptr, record2) << "Unexpected malloc failure";
    new (record2) record(6U, 8593U, 192582U);
    ipv4_address* address2 = static_cast<ipv4_address*>(allocator2.malloc(sizeof(ipv4_address)));
    EXPECT_NE(nullptr, address2) << "Unexpected malloc failure";
    (*address2)[0] = 255U;
    (*address2)[1] = 255U;
    (*address2)[2] = 255U;
    (*address2)[3] = 255U;
    record* record3 = static_cast<record*>(allocator2.malloc(sizeof(record)));
    EXPECT_NE(nullptr, record3) << "Unexpected malloc failure";
    ipv4_address* address3 = static_cast<ipv4_address*>(allocator2.malloc(sizeof(ipv4_address)));
    EXPECT_NE(nullptr, address3) << "Unexpected malloc failure";
    EXPECT_EQ(record(5U, 8741U, 20873U), *record1) << "Allocation from original pool was affected by allocation from copy constructed pool";
    ipv4_address expected1 {192U, 168U, 1U, 254U };
    EXPECT_TRUE(std::equal(expected1.cbegin(), expected1.cend(), address1->cbegin())) << "Allocation from original pool was affected by allocation from copy constructed pool";
}
