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

TEST(untyped_allocator_test, malloc_basic)
{
    tci::untyped_allocator allocator1(2U, { {sizeof(record), 2U}, {sizeof(ipv4_address), 2U} });
    record* record1 = static_cast<record*>(allocator1.malloc(sizeof(record)));
    EXPECT_TRUE(record1 != nullptr) << "Unexpected malloc failure";
}

TEST(untyped_allocator_test, pool_copy_assignment_same_length)
{
    tci::untyped_allocator allocator1(2U, { {sizeof(std::uint64_t), 2U}, {sizeof(std::uint16_t), 2U} });
    tci::untyped_allocator allocator2(2U, { {sizeof(std::uint64_t), 2U}, {sizeof(std::uint16_t), 2U} });
    //allocator2 = allocator1;
    //EXPECT_TRUE(allocator1 == allocator2) << "Copy assigned pool is not equal to the original";
}
