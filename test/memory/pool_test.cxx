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

namespace tme = turbo::memory;
namespace tar = turbo::algorithm::recovery;

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

TEST(pool_test, make_mixed_basic)
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
    }
}
