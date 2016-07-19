#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>
#include <gtest/gtest.h>
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <thread>
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
}
