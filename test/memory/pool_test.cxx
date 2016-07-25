#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <turbo/algorithm/recovery.hpp>

namespace tar = turbo::algorithm::recovery;
namespace tco = turbo::container;
namespace tme = turbo::memory;

TEST(pool_test, make_unique_basic)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U, 1U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Unique pool string didn't initialise";
    }
}

TEST(pool_test, make_unique_full)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(2U, 1U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::pool_full, result3.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result3.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_unique_recycle)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U, 1U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Unique pool string didn't initialise";
    }
    {
	auto result4 = pool1.make_unique<std::string>("foo");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("foo"), *result4.second) << "Recycled unique pool string didn't initialise";
	auto result5 = pool1.make_unique<std::string>("bar");
	EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("bar"), *result5.second) << "Recycled unique pool string didn't initialise";
	auto result6 = pool1.make_unique<std::string>("blah");
	EXPECT_EQ(tme::make_result::success, result6.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("blah"), *result6.second) << "Recycled unique pool string didn't initialise";
	auto result7 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result7.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result7.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_unique_moved)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U, 1U);
    {
	tme::pool_unique_ptr<std::string> moved1;
	tme::pool_unique_ptr<std::string> moved2;
	tme::pool_unique_ptr<std::string> moved3;
	{
	    auto result1 = pool1.make_unique<std::string>("abc123");
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	    auto result2 = pool1.make_unique<std::string>("xyz789");
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	    auto result3 = pool1.make_unique<std::string>("lmn456");
	    EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("lmn456"), *result3.second) << "Unique pool string didn't initialise";
	    moved1 = std::move(result1.second);
	    moved2 = std::move(result2.second);
	    moved3 = std::move(result3.second);
	}
    }
    {
	auto result4 = pool1.make_unique<std::string>("foo");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("foo"), *result4.second) << "Recycled unique pool string didn't initialise";
	auto result5 = pool1.make_unique<std::string>("bar");
	EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("bar"), *result5.second) << "Recycled unique pool string didn't initialise";
	auto result6 = pool1.make_unique<std::string>("blah");
	EXPECT_EQ(tme::make_result::success, result6.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("blah"), *result6.second) << "Recycled unique pool string didn't initialise";
	auto result7 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result7.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result7.second.get()) << "Pointer returned from full pool is not null";
	tme::pool_unique_ptr<std::string> moved4(std::move(result4.second));
	tme::pool_unique_ptr<std::string> moved5(std::move(result5.second));
	tme::pool_unique_ptr<std::string> moved6(std::move(result6.second));
    }
}

constexpr std::size_t double_string()
{
    return sizeof(std::string) * 2;
}

TEST(pool_test, make_unique_large_block)
{
    typedef tme::block_pool<double_string()> string_pool;
    string_pool pool1(3U, 1U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Unique pool string didn't initialise";
    }
    {
	auto result4 = pool1.make_unique<std::string>("foo");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("foo"), *result4.second) << "Recycled unique pool string didn't initialise";
	auto result5 = pool1.make_unique<std::string>("bar");
	EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("bar"), *result5.second) << "Recycled unique pool string didn't initialise";
	auto result6 = pool1.make_unique<std::string>("blah");
	EXPECT_EQ(tme::make_result::success, result6.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("blah"), *result6.second) << "Recycled unique pool string didn't initialise";
	auto result7 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result7.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result7.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_unique_array)
{
    typedef std::array<std::uint32_t, 8> uint_array;
    typedef tme::block_pool<sizeof(uint_array)> array_pool;
    array_pool pool1(2U, 1U);
    {
	auto result1 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool array failed";
	std::fill(result1.second->begin(), result1.second->end(), 0U);
	auto result2 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool array failed";
	std::fill(result2.second->begin(), result2.second->end(), std::numeric_limits<uint_array::value_type>::max());
	for (const std::uint32_t& value: *(result1.second))
	{
	    EXPECT_EQ(0U, value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result1.second->cbegin()));
	}
	for (const std::uint32_t& value: *(result2.second))
	{
	    EXPECT_EQ(std::numeric_limits<uint_array::value_type>::max(), value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result2.second->cbegin()));
	}
    }
    {
	auto result3 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool array failed";
	std::fill(result3.second->begin(), result3.second->end(), std::numeric_limits<uint_array::value_type>::max());
	auto result4 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled unique pool array failed";
	std::fill(result4.second->begin(), result4.second->end(), 0U);
	for (const std::uint32_t& value: *(result3.second))
	{
	    EXPECT_EQ(std::numeric_limits<uint_array::value_type>::max(), value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result3.second->cbegin()));
	}
	for (const std::uint32_t& value: *(result4.second))
	{
	    EXPECT_EQ(0U, value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result4.second->cbegin()));
	}
	auto result5 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::pool_full, result5.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result5.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_shared_basic)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U, 1U);
    {
	auto result1 = pool1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	auto result2 = pool1.make_shared<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	auto result3 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared pool string didn't initialise";
    }
}

TEST(pool_test, make_shared_full)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(2U, 1U);
    {
	auto result1 = pool1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	auto result2 = pool1.make_shared<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	auto result3 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::pool_full, result3.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result3.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_shared_recycle)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U, 1U);
    {
	auto result1 = pool1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	auto result2 = pool1.make_shared<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	auto result3 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared pool string didn't initialise";
    }
    {
	auto result4 = pool1.make_shared<std::string>("foo");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled shared pool string failed";
	EXPECT_EQ(std::string("foo"), *result4.second) << "Recycled shared pool string didn't initialise";
	auto result5 = pool1.make_shared<std::string>("bar");
	EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled shared pool string failed";
	EXPECT_EQ(std::string("bar"), *result5.second) << "Recycled shared pool string didn't initialise";
	auto result6 = pool1.make_shared<std::string>("blah");
	EXPECT_EQ(tme::make_result::success, result6.first) << "Make reycled shared pool string failed";
	EXPECT_EQ(std::string("blah"), *result6.second) << "Recycled shared pool string didn't initialise";
	auto result7 = pool1.make_shared<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result7.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result7.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_shared_copied)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U, 1U);
    {
	std::shared_ptr<std::string> copy1;
	std::shared_ptr<std::string> copy2;
	std::shared_ptr<std::string> copy3;
	{
	    auto result1 = pool1.make_shared<std::string>("abc123");
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	    auto result2 = pool1.make_shared<std::string>("xyz789");
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	    auto result3 = pool1.make_shared<std::string>("lmn456");
	    EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared pool string didn't initialise";
	    copy1 = result1.second;
	    copy2 = result2.second;
	    copy3 = result3.second;
	}
	auto result4 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result4.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result4.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result5 = pool1.make_shared<std::string>("!@#");
    EXPECT_EQ(tme::make_result::success, result5.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("!@#"), *result5.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy4 = result5.second;
    auto result6 = pool1.make_shared<std::string>("$%^");
    EXPECT_EQ(tme::make_result::success, result6.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("$%^"), *result6.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy5 = result6.second;
    auto result7 = pool1.make_shared<std::string>("&*(");
    EXPECT_EQ(tme::make_result::success, result7.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("&*("), *result7.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy6 = result7.second;
}

TEST(pool_test, make_shared_large_block)
{
    typedef tme::block_pool<double_string()> string_pool;
    string_pool pool1(3U, 1U);
    {
	std::shared_ptr<std::string> copy1;
	std::shared_ptr<std::string> copy2;
	std::shared_ptr<std::string> copy3;
	{
	    auto result1 = pool1.make_shared<std::string>("abc123");
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	    auto result2 = pool1.make_shared<std::string>("xyz789");
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	    auto result3 = pool1.make_shared<std::string>("lmn456");
	    EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared pool string didn't initialise";
	    copy1 = result1.second;
	    copy2 = result2.second;
	    copy3 = result3.second;
	}
	auto result4 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result4.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result4.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result5 = pool1.make_shared<std::string>("!@#");
    EXPECT_EQ(tme::make_result::success, result5.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("!@#"), *result5.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy4 = result5.second;
    auto result6 = pool1.make_shared<std::string>("$%^");
    EXPECT_EQ(tme::make_result::success, result6.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("$%^"), *result6.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy5 = result6.second;
    auto result7 = pool1.make_shared<std::string>("&*(");
    EXPECT_EQ(tme::make_result::success, result7.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("&*("), *result7.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy6 = result7.second;
}

TEST(pool_test, make_shared_array)
{
    typedef std::array<std::uint32_t, 8> uint_array;
    typedef tme::block_pool<sizeof(uint_array)> array_pool;
    array_pool pool1(2U, 1U);
    {
	std::shared_ptr<uint_array> copy1;
	std::shared_ptr<uint_array> copy2;
	{
	    auto result1 = pool1.make_shared<uint_array>();
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool array failed";
	    std::fill(result1.second->begin(), result1.second->end(), 15U);
	    auto result2 = pool1.make_shared<uint_array>();
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool array failed";
	    std::fill(result2.second->begin(), result2.second->end(), 256U);
	    copy1 = result1.second;
	    copy2 = result2.second;
	}
	for (std::uint32_t& value: *copy1)
	{
	    value *= 2;
	}
	for (std::uint32_t& value: *copy2)
	{
	    value *= 2;
	}
	for (const std::uint32_t& value: *copy1)
	{
	    EXPECT_EQ(15U * 2, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(copy1->cbegin()));
	}
	for (const std::uint32_t& value: *copy2)
	{
	    EXPECT_EQ(256U * 2, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(copy2->cbegin()));
	}
	auto result3 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::pool_full, result3.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result3.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result4 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::success, result4.first) << "Make shared pool array failed";
    std::fill(result4.second->begin(), result4.second->end(), 256U);
    std::shared_ptr<uint_array> copy3 = result4.second;
    auto result5 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled shared pool array failed";
    std::fill(result5.second->begin(), result5.second->end(), 15U);
    std::shared_ptr<uint_array> copy4 = result5.second;
    for (const std::uint32_t& value: *(result4.second))
    {
	EXPECT_EQ(256U, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(result4.second->cbegin()));
    }
    for (const std::uint32_t& value: *(result5.second))
    {
	EXPECT_EQ(15U, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(result5.second->cbegin()));
    }
    auto result6 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::pool_full, result6.first) << "Full pool is still allocating";
    EXPECT_EQ(nullptr, result6.second.get()) << "Pointer returned from full pool is not null";
}

TEST(pool_test, make_mixed_full)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(4U, 1U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_shared<std::string>("!@#");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("!@#"), *result2.second) << "Shared pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result3.second) << "Unique pool string didn't initialise";
	auto result4 = pool1.make_shared<std::string>("$%^");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("$%^"), *result4.second) << "Shared pool string didn't initialise";
	auto result5 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::pool_full, result5.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result5.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_mixed_transferred)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(4U, 1U);
    {
	tme::pool_unique_ptr<std::string> move1;
	std::shared_ptr<std::string> copy2;
	tme::pool_unique_ptr<std::string> move3;
	std::shared_ptr<std::string> copy4;
	{
	    auto result1 = pool1.make_unique<std::string>("abc123");
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	    auto result2 = pool1.make_shared<std::string>("!@#");
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("!@#"), *result2.second) << "Shared pool string didn't initialise";
	    auto result3 = pool1.make_unique<std::string>("xyz789");
	    EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("xyz789"), *result3.second) << "Unique pool string didn't initialise";
	    auto result4 = pool1.make_shared<std::string>("$%^");
	    EXPECT_EQ(tme::make_result::success, result4.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("$%^"), *result4.second) << "Shared pool string didn't initialise";
	    move1 = std::move(result1.second);
	    copy2 = result2.second;
	    move3 = std::move(result3.second);
	    copy4 = result4.second;
	}
	auto result5 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::pool_full, result5.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result5.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result6 = pool1.make_unique<std::string>("xyz789");
    EXPECT_EQ(tme::make_result::success, result6.first) << "Make unique pool string failed";
    EXPECT_EQ(std::string("xyz789"), *result6.second) << "Unique pool string didn't initialise";
    auto result7 = pool1.make_shared<std::string>("$%^");
    EXPECT_EQ(tme::make_result::success, result7.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("$%^"), *result7.second) << "Shared pool string didn't initialise";
}

TEST(pool_test, make_mixed_array)
{
    typedef std::array<std::uint32_t, 8> uint_array;
    typedef tme::block_pool<sizeof(uint_array)> array_pool;
    array_pool pool1(2U, 1U);
    {
	std::shared_ptr<uint_array> copy1;
	tme::pool_unique_ptr<uint_array> move2;
	{
	    auto result1 = pool1.make_shared<uint_array>();
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool array failed";
	    std::fill(result1.second->begin(), result1.second->end(), 15U);
	    auto result2 = pool1.make_unique<uint_array>();
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool array failed";
	    std::fill(result2.second->begin(), result2.second->end(), 256U);
	    copy1 = result1.second;
	    move2 = std::move(result2.second);
	}
	for (std::uint32_t& value: *copy1)
	{
	    value *= 2;
	}
	for (std::uint32_t& value: *move2)
	{
	    value *= 2;
	}
	for (const std::uint32_t& value: *copy1)
	{
	    EXPECT_EQ(15U * 2, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(copy1->cbegin()));
	}
	for (const std::uint32_t& value: *move2)
	{
	    EXPECT_EQ(256U * 2, value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(move2->cbegin()));
	}
	auto result3 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::pool_full, result3.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result3.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result4 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::success, result4.first) << "Make recyced shared pool array failed";
    std::fill(result4.second->begin(), result4.second->end(), 256U);
    std::shared_ptr<uint_array> copy3 = result4.second;
    auto result5 = pool1.make_unique<uint_array>();
    EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled unique pool array failed";
    std::fill(result5.second->begin(), result5.second->end(), 15U);
    for (const std::uint32_t& value: *(result4.second))
    {
	EXPECT_EQ(256U, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(result4.second->cbegin()));
    }
    for (const std::uint32_t& value: *(result5.second))
    {
	EXPECT_EQ(15U, value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result5.second->cbegin()));
    }
    auto result6 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::pool_full, result6.first) << "Full pool is still allocating";
    EXPECT_EQ(nullptr, result6.second.get()) << "Pointer returned from full pool is not null";
}

struct record
{
    record();
    record(uint16_t f, uint32_t s, uint64_t t);
    record(const record& other);
    bool operator==(const record& other) const;
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

bool record::operator==(const record& other) const
{
    return first == other.first && second == other.second && third == other.third;
}

template <std::size_t limit>
class produce_task
{
public:
    typedef tme::pool_unique_ptr<record> unique_value;
    typedef tco::mpmc_ring_queue<unique_value> queue;
    typedef tme::block_pool<sizeof(record)> pool_type;
    produce_task(typename queue::producer& producer, pool_type& pool, std::array<record, limit>& input);
    ~produce_task() noexcept;
    void run();
    void produce();
private:
    typename queue::producer& producer_;
    pool_type& pool_;
    std::array<record, limit>& input_;
    std::thread* thread_;
};

template <std::size_t limit>
produce_task<limit>::produce_task(typename queue::producer& producer, pool_type& pool, std::array<record, limit>& input)
    :
	producer_(producer),
	pool_(pool),
	input_(input),
	thread_(nullptr)
{ }

template <std::size_t limit>
produce_task<limit>::~produce_task() noexcept
{
    try
    {
	if (thread_)
	{
	    thread_->join();
	    delete thread_;
	    thread_ = nullptr;
	}
    }
    catch(...)
    {
	// do nothing
    }
}

template <std::size_t limit>
void produce_task<limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&produce_task::produce, this));
	thread_ = new std::thread(entry);
    }
}

template <std::size_t limit>
void produce_task<limit>::produce()
{
    for (auto iter = input_.begin(); iter != input_.end();)
    {
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    auto result = pool_.make_unique<record>(*iter);
	    if (result.first == tme::make_result::success)
	    {
		tar::retry_with_random_backoff([&] () -> tar::try_state
		{
		    if (producer_.try_enqueue_move(std::move(result.second)) == queue::producer::result::success)
		    {
			++iter;
			return tar::try_state::done;
		    }
		    else
		    {
			return tar::try_state::retry;
		    }
		});
		return tar::try_state::done;
	    }
	    else
	    {
		return tar::try_state::retry;
	    }
	    return tar::try_state::done;
	});
    }
}

template <std::size_t limit>
class consume_task
{
public:
    typedef tme::pool_unique_ptr<record> unique_value;
    typedef tco::mpmc_ring_queue<unique_value> queue;
    consume_task(typename queue::consumer& consumer, std::array<record, limit>& output);
    ~consume_task() noexcept;
    void run();
    void consume();
private:
    typename queue::consumer& consumer_;
    std::array<record, limit>& output_;
    std::thread* thread_;
};

template <std::size_t limit>
consume_task<limit>::consume_task(typename queue::consumer& consumer, std::array<record, limit>& output)
    :
	consumer_(consumer),
	output_(output),
	thread_(nullptr)
{ }

template <std::size_t limit>
consume_task<limit>::~consume_task() noexcept
{
    try
    {
	if (thread_)
	{
	    thread_->join();
	    delete thread_;
	    thread_ = nullptr;
	}
    }
    catch(...)
    {
	// do nothing
    }
}

template <std::size_t limit>
void consume_task<limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&consume_task::consume, this));
	thread_ = new std::thread(entry);
    }
}

template <std::size_t limit>
void consume_task<limit>::consume()
{
    for (auto iter = output_.begin(); iter != output_.end();)
    {
	unique_value tmp;
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    if (consumer_.try_dequeue_move(tmp) == queue::consumer::result::success)
	    {
		*iter = *tmp;
		++iter;
		return tar::try_state::done;
	    }
	    else
	    {
		return tar::try_state::retry;
	    }
	});
    }
}

TEST(pool_test, messasge_passing)
{
    typedef tme::pool_unique_ptr<record> unique_record;
    typedef tco::mpmc_ring_queue<unique_record> unique_record_queue;
    typedef tme::block_pool<sizeof(record)> record_pool;
    unique_record_queue queue1(64U, 4U);
    record_pool pool1(8192U, 4U);
    std::unique_ptr<std::array<record, 8192U>> expected_input(new std::array<record, 8192U>());
    std::unique_ptr<std::array<record, 2048U>> input1(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> input2(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> input3(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> input4(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> output1(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> output2(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> output3(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> output4(new std::array<record, 2048U>());
    for (uint64_t counter1 = 0U; counter1 < input1->max_size(); ++counter1)
    {
	uint16_t base1 = 3U + (counter1 * 5U) + 0U;
	record tmp{base1, base1 * 3U, base1 * 9UL};
	(*input1)[counter1] = tmp;
	(*expected_input)[counter1 + 0U] = tmp;
    }
    for (uint64_t counter2 = 0U; counter2 < input2->max_size(); ++counter2)
    {
	uint16_t base2 = 3U + (counter2 * 5U) + 10240U;
	record tmp{base2, base2 * 3U, base2 * 9UL};
	(*input2)[counter2] = tmp;
	(*expected_input)[counter2 + 2048U] = tmp;
    }
    for (uint64_t counter3 = 0U; counter3 < input3->max_size(); ++counter3)
    {
	uint16_t base3 = 3U + (counter3 * 5U) + 20480;
	record tmp{base3, base3 * 3U, base3 * 9UL};
	(*input3)[counter3] = tmp;
	(*expected_input)[counter3 + 4096U] = tmp;
    }
    for (uint64_t counter4 = 0U; counter4 < input4->max_size(); ++counter4)
    {
	uint16_t base4 = 3U + (counter4 * 5U) + 30720U;
	record tmp{base4, base4 * 3U, base4 * 9UL};
	(*input4)[counter4] = tmp;
	(*expected_input)[counter4 + 6144U] = tmp;
    }
    {
	produce_task<2048U> producer1(queue1.get_producer(), pool1, *input1);
	produce_task<2048U> producer2(queue1.get_producer(), pool1, *input2);
	produce_task<2048U> producer3(queue1.get_producer(), pool1, *input3);
	produce_task<2048U> producer4(queue1.get_producer(), pool1, *input4);
	consume_task<2048U> consumer1(queue1.get_consumer(), *output1);
	consume_task<2048U> consumer2(queue1.get_consumer(), *output2);
	consume_task<2048U> consumer3(queue1.get_consumer(), *output3);
	consume_task<2048U> consumer4(queue1.get_consumer(), *output4);
	producer1.run();
	consumer2.run();
	producer2.run();
	consumer3.run();
	producer3.run();
	consumer4.run();
	producer4.run();
	consumer1.run();
    }
    std::unique_ptr<std::array<record, 8192U>> actual_output(new std::array<record, 8192U>());
    {
	auto actual_iter = actual_output->begin();
	for (auto out_iter = output1->cbegin(); actual_iter != actual_output->end() && out_iter != output1->cend(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output2->cbegin(); actual_iter != actual_output->end() && out_iter != output2->cend(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output3->cbegin(); actual_iter != actual_output->end() && out_iter != output3->cend(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output4->cbegin(); actual_iter != actual_output->end() && out_iter != output4->cend(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
    }
    std::stable_sort(actual_output->begin(), actual_output->end(), [] (const record& left, const record& right) -> bool
    {
	return left.first < right.first;
    });
    auto expected_iter = expected_input->cbegin();
    auto actual_iter = actual_output->cbegin();
    for (; expected_iter != expected_input->cend() && actual_iter != actual_output->cend(); ++expected_iter, ++actual_iter)
    {
	EXPECT_EQ(*expected_iter, *actual_iter) << "Mismatching record consumed " <<
		"- expected {" << expected_iter->first << ", " << expected_iter->second << ", " << expected_iter->third << "} " <<
		"- actual {" << actual_iter->first << ", " << actual_iter->second << ", " << actual_iter->third << "}";
    }
}
