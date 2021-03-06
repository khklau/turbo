#include <turbo/container/emplacing_list.hpp>
#include <turbo/container/emplacing_list.hh>
#include <cstdint>
#include <algorithm>
#include <list>
#include <random>
#include <string>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hh>
#include <turbo/memory/slab_allocator.hpp>
#include <turbo/memory/slab_allocator.hh>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;
namespace tme = turbo::memory;

TEST(emplacing_list_test, empty_list)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    EXPECT_EQ(list1.end(), list1.begin()) << "When list is empty begin and end are not equal";
    EXPECT_EQ(list1.rend(), list1.rbegin()) << "When list is empty rbegin and rend are not equal";
    EXPECT_EQ(0U, list1.size()) << "Size of empty list is not 0";
}

TEST(emplacing_list_test, emplace_front_basic)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    EXPECT_EQ(std::string("foobar"), *(list1.emplace_front("foobar"))) << "Returned iterator does not point to the emplaced value";
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is empty emplace_front failed";
    EXPECT_EQ(std::string("foobar"), *(list1.rbegin())) << "When list is empty emplace_front failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after emplacing to an empty list is not 1";
    EXPECT_EQ(std::string("blah"), *(list1.emplace_front("blah"))) << "Returned iterator does not point to the emplaced value";
    EXPECT_EQ(std::string("blah"), *(list1.begin())) << "When list is not empty emplace_front failed";
    EXPECT_EQ(std::string("foobar"), *(list1.rbegin())) << "When list is not empty emplace_front failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after emplacing to a list of 1 element is not 2";
}

TEST(emplacing_list_test, emplace_back_basic)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    EXPECT_EQ(std::string("foobar"), *(list1.emplace_back("foobar"))) << "Returned iterator does not point to the emplaced value";
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is empty emplace_back failed";
    EXPECT_EQ(std::string("foobar"), *(list1.rbegin())) << "When list is empty emplace_back failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after emplacing to an empty list is not 1";
    EXPECT_EQ(std::string("blah"), *(list1.emplace_back("blah"))) << "Returned iterator does not point to the emplaced value";
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(std::string("blah"), *(list1.rbegin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after emplacing to a list of 1 element is not 2";
}

TEST(emplacing_list_test, emplace_mixed_basic)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("BBB");
    EXPECT_EQ(std::string("BBB"), *(list1.begin())) << "When list is empty emplace_back failed";
    EXPECT_EQ(std::string("BBB"), *(list1.rbegin())) << "When list is empty emplace_back failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after emplacing to an empty list is not 1";
    list1.emplace_back("CCC");
    EXPECT_EQ(std::string("BBB"), *(list1.begin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(std::string("CCC"), *(list1.rbegin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after emplacing to a list of 1 element is not 2";
    list1.emplace_front("AAA");
    EXPECT_EQ(std::string("AAA"), *(list1.begin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(std::string("CCC"), *(list1.rbegin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(3U, list1.size()) << "Size of list after emplacing to a list of 2 element is not 3";
    list1.emplace_back("DDD");
    EXPECT_EQ(std::string("AAA"), *(list1.begin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(std::string("DDD"), *(list1.rbegin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(4U, list1.size()) << "Size of list after emplacing to a list of 3 element is not 4";
}

TEST(emplacing_list_test, forward_iterate_invalid)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("789");
    list1.emplace_front("456");
    list1.emplace_front("123");
    {
	auto iter = list1.end();
	++iter;
	EXPECT_EQ(list1.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.begin();
	++iter;
	++iter;
	++iter;
	EXPECT_EQ(list1.end(), iter) << "Iterator at position end is not equal to end iterator";
	++iter;
	EXPECT_EQ(list1.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.begin();
	--iter;
	EXPECT_EQ(list1.end(), iter) << "Iterator at position end is not equal to end iterator";
	--iter;
	EXPECT_EQ(list1.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.cend();
	++iter;
	EXPECT_EQ(list1.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.cbegin();
	++iter;
	++iter;
	++iter;
	EXPECT_EQ(list1.cend(), iter) << "Iterator at position end is not equal to end iterator";
	++iter;
	EXPECT_EQ(list1.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.cbegin();
	--iter;
	EXPECT_EQ(list1.cend(), iter) << "Iterator at position end is not equal to end iterator";
	--iter;
	EXPECT_EQ(list1.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    tme::concurrent_sized_slab allocator2(8U, { {string_list::allocation_size(), 8U} });
    string_list list2(allocator2);
    list2.emplace_back("123");
    list2.emplace_back("456");
    list2.emplace_back("789");
    {
	auto iter = list2.end();
	++iter;
	EXPECT_EQ(list2.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.begin();
	++iter;
	++iter;
	++iter;
	EXPECT_EQ(list2.end(), iter) << "Iterator at position end is not equal to end iterator";
	++iter;
	EXPECT_EQ(list2.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.begin();
	--iter;
	EXPECT_EQ(list2.end(), iter) << "Iterator at position end is not equal to end iterator";
	--iter;
	EXPECT_EQ(list2.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.cend();
	++iter;
	EXPECT_EQ(list2.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.cbegin();
	++iter;
	++iter;
	++iter;
	EXPECT_EQ(list2.cend(), iter) << "Iterator at position end is not equal to end iterator";
	++iter;
	EXPECT_EQ(list2.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.cbegin();
	--iter;
	EXPECT_EQ(list2.cend(), iter) << "Iterator at position end is not equal to end iterator";
	--iter;
	EXPECT_EQ(list2.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
}

TEST(emplacing_list_test, forward_iterate_basic)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("789");
    list1.emplace_front("456");
    list1.emplace_front("123");
    {
	auto iter = list1.begin();
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	++iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	++iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	++iter;
	EXPECT_EQ(list1.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.begin();
	++iter;
	++iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	--iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	--iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	--iter;
	EXPECT_EQ(list1.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.cbegin();
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	++iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	++iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	++iter;
	EXPECT_EQ(list1.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.cbegin();
	++iter;
	++iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	--iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	--iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	--iter;
	EXPECT_EQ(list1.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    tme::concurrent_sized_slab allocator2(8U, { {string_list::allocation_size(), 8U} });
    string_list list2(allocator2);
    list2.emplace_back("123");
    list2.emplace_back("456");
    list2.emplace_back("789");
    {
	auto iter = list2.begin();
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	++iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	++iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	++iter;
	EXPECT_EQ(list2.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.begin();
	++iter;
	++iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	--iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	--iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	--iter;
	EXPECT_EQ(list2.end(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.cbegin();
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	++iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	++iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	++iter;
	EXPECT_EQ(list2.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.cbegin();
	++iter;
	++iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	--iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	--iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	--iter;
	EXPECT_EQ(list2.cend(), iter) << "Iterator at position end is not equal to end iterator";
    }
}

TEST(emplacing_list_test, reverse_iterate_invalid)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("789");
    list1.emplace_front("456");
    list1.emplace_front("123");
    {
	auto iter = list1.rend();
	++iter;
	EXPECT_EQ(list1.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.rbegin();
	++iter;
	++iter;
	++iter;
	EXPECT_EQ(list1.rend(), iter) << "Iterator at position end is not equal to end iterator";
	++iter;
	EXPECT_EQ(list1.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.rbegin();
	--iter;
	EXPECT_EQ(list1.rend(), iter) << "Iterator at position end is not equal to end iterator";
	--iter;
	EXPECT_EQ(list1.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.crend();
	++iter;
	EXPECT_EQ(list1.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.crbegin();
	++iter;
	++iter;
	++iter;
	EXPECT_EQ(list1.crend(), iter) << "Iterator at position end is not equal to end iterator";
	++iter;
	EXPECT_EQ(list1.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.crbegin();
	--iter;
	EXPECT_EQ(list1.crend(), iter) << "Iterator at position end is not equal to end iterator";
	--iter;
	EXPECT_EQ(list1.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    tme::concurrent_sized_slab allocator2(8U, { {string_list::allocation_size(), 8U} });
    string_list list2(allocator2);
    list2.emplace_back("123");
    list2.emplace_back("456");
    list2.emplace_back("789");
    {
	auto iter = list2.rend();
	++iter;
	EXPECT_EQ(list2.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.rbegin();
	++iter;
	++iter;
	++iter;
	EXPECT_EQ(list2.rend(), iter) << "Iterator at position end is not equal to end iterator";
	++iter;
	EXPECT_EQ(list2.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.rbegin();
	--iter;
	EXPECT_EQ(list2.rend(), iter) << "Iterator at position end is not equal to end iterator";
	--iter;
	EXPECT_EQ(list2.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.crend();
	++iter;
	EXPECT_EQ(list2.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.crbegin();
	++iter;
	++iter;
	++iter;
	EXPECT_EQ(list2.crend(), iter) << "Iterator at position end is not equal to end iterator";
	++iter;
	EXPECT_EQ(list2.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.crbegin();
	--iter;
	EXPECT_EQ(list2.crend(), iter) << "Iterator at position end is not equal to end iterator";
	--iter;
	EXPECT_EQ(list2.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
}

TEST(emplacing_list_test, reverse_iterate_basic)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("789");
    list1.emplace_front("456");
    list1.emplace_front("123");
    {
	auto iter = list1.rbegin();
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	++iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	++iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	++iter;
	EXPECT_EQ(list1.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.rbegin();
	++iter;
	++iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	--iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	--iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	--iter;
	EXPECT_EQ(list1.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.crbegin();
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	++iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	++iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	++iter;
	EXPECT_EQ(list1.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list1.crbegin();
	++iter;
	++iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	--iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	--iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	--iter;
	EXPECT_EQ(list1.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    tme::concurrent_sized_slab allocator2(8U, { {string_list::allocation_size(), 8U} });
    string_list list2(allocator2);
    list2.emplace_back("123");
    list2.emplace_back("456");
    list2.emplace_back("789");
    {
	auto iter = list2.rbegin();
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	++iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	++iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	++iter;
	EXPECT_EQ(list2.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.rbegin();
	++iter;
	++iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	--iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	--iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	--iter;
	EXPECT_EQ(list2.rend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.crbegin();
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	++iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	++iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	++iter;
	EXPECT_EQ(list2.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
    {
	auto iter = list2.crbegin();
	++iter;
	++iter;
	EXPECT_EQ(std::string("123"), *iter) << "Iterator at position front is invalid";
	--iter;
	EXPECT_EQ(std::string("456"), *iter) << "Iterator at position middle is invalid";
	--iter;
	EXPECT_EQ(std::string("789"), *iter) << "Iterator at position back is invalid";
	--iter;
	EXPECT_EQ(list2.crend(), iter) << "Iterator at position end is not equal to end iterator";
    }
}

TEST(emplacing_list_test, emplace_invalid)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    EXPECT_EQ(std::string("foobar"), *(list1.emplace(string_list::iterator(), "foobar"))) << "When list is empty emplace failed";
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is empty emplace failed";
    EXPECT_EQ(std::string("foobar"), *(list1.rbegin())) << "When list is empty emplace failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after emplacing to an empty list is not 1";
}

TEST(emplacing_list_test, emplace_empty)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    EXPECT_EQ(std::string("foobar"), *(list1.emplace(list1.begin(), "foobar"))) << "When list is empty emplace failed";
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is empty emplace failed";
    EXPECT_EQ(std::string("foobar"), *(list1.rbegin())) << "When list is empty emplace failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after emplacing to an empty list is not 1";
    tme::concurrent_sized_slab allocator2(8U, { {string_list::allocation_size(), 8U} });
    string_list list2(allocator2);
    EXPECT_EQ(std::string("foobar"), *(list2.emplace(list2.end(), "foobar"))) << "When list is empty emplace failed";
    EXPECT_EQ(std::string("foobar"), *(list2.begin())) << "When list is empty emplace failed";
    EXPECT_EQ(std::string("foobar"), *(list2.rbegin())) << "When list is empty emplace failed";
    EXPECT_EQ(1U, list2.size()) << "Size of list after emplacing to an empty list is not 1";
}

TEST(emplacing_list_test, emplace_front_non_empty)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_back("789");
    EXPECT_EQ(std::string("456"), *(list1.emplace(list1.begin(), "456"))) << "When list is not empty emplace failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after emplacing to a list of 1 is not 2";
    EXPECT_EQ(std::string("456"), *(list1.begin())) << "When list is not empty emplace failed";
    EXPECT_EQ(std::string("789"), *(list1.rbegin())) << "When list is not empty emplace failed";
    auto iter1a = list1.cbegin();
    EXPECT_EQ(std::string("456"), *iter1a) << "Emplace on non-empty list produced invalid links";
    ++iter1a;
    EXPECT_EQ(std::string("789"), *iter1a) << "Emplace on non-empty list produced invalid links";
    auto iter1b = list1.crbegin();
    EXPECT_EQ(std::string("789"), *iter1b) << "Emplace on non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("456"), *iter1b) << "Emplace on non-empty list produced invalid links";
    EXPECT_EQ(std::string("123"), *(list1.emplace(list1.begin(), "123"))) << "When list is not empty emplace failed";
    EXPECT_EQ(3U, list1.size()) << "Size of list after emplacing to a list of 2 is not 3";
    EXPECT_EQ(std::string("123"), *(list1.begin())) << "When list is not empty emplace failed";
    EXPECT_EQ(std::string("789"), *(list1.rbegin())) << "When list is not empty emplace failed";
    auto iter1c = list1.cbegin();
    EXPECT_EQ(std::string("123"), *iter1c) << "Emplace on non-empty list produced invalid links";
    ++iter1c;
    EXPECT_EQ(std::string("456"), *iter1c) << "Emplace on non-empty list produced invalid links";
    ++iter1c;
    EXPECT_EQ(std::string("789"), *iter1c) << "Emplace on non-empty list produced invalid links";
    auto iter1d = list1.crbegin();
    EXPECT_EQ(std::string("789"), *iter1d) << "Emplace on non-empty list produced invalid links";
    ++iter1d;
    EXPECT_EQ(std::string("456"), *iter1d) << "Emplace on non-empty list produced invalid links";
    ++iter1d;
    EXPECT_EQ(std::string("123"), *iter1d) << "Emplace on non-empty list produced invalid links";
}

TEST(emplacing_list_test, emplace_back_non_empty)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator2(8U, { {string_list::allocation_size(), 8U} });
    string_list list2(allocator2);
    list2.emplace_back("123");
    EXPECT_EQ(std::string("456"), *(list2.emplace(list2.end(), "456"))) << "When list is not empty emplace failed";
    EXPECT_EQ(2U, list2.size()) << "Size of list after emplacing to a list of 1 is not 2";
    EXPECT_EQ(std::string("123"), *(list2.begin())) << "When list is not empty emplace failed";
    EXPECT_EQ(std::string("456"), *(list2.rbegin())) << "When list is not empty emplace failed";
    auto iter2a = list2.cbegin();
    EXPECT_EQ(std::string("123"), *iter2a) << "Emplace on non-empty list produced invalid links";
    ++iter2a;
    EXPECT_EQ(std::string("456"), *iter2a) << "Emplace on non-empty list produced invalid links";
    auto iter2b = list2.crbegin();
    EXPECT_EQ(std::string("456"), *iter2b) << "Emplace on non-empty list produced invalid links";
    ++iter2b;
    EXPECT_EQ(std::string("123"), *iter2b) << "Emplace on non-empty list produced invalid links";
    EXPECT_EQ(std::string("789"), *(list2.emplace(list2.end(), "789"))) << "When list is not empty emplace failed";
    EXPECT_EQ(3U, list2.size()) << "Size of list after emplacing to a list of 2 is not 3";
    EXPECT_EQ(std::string("123"), *(list2.begin())) << "When list is not empty emplace failed";
    EXPECT_EQ(std::string("789"), *(list2.rbegin())) << "When list is not empty emplace failed";
    auto iter2c = list2.cbegin();
    EXPECT_EQ(std::string("123"), *iter2c) << "Emplace on non-empty list produced invalid links";
    ++iter2c;
    EXPECT_EQ(std::string("456"), *iter2c) << "Emplace on non-empty list produced invalid links";
    ++iter2c;
    EXPECT_EQ(std::string("789"), *iter2c) << "Emplace on non-empty list produced invalid links";
    auto iter2d = list2.crbegin();
    EXPECT_EQ(std::string("789"), *iter2d) << "Emplace on non-empty list produced invalid links";
    ++iter2d;
    EXPECT_EQ(std::string("456"), *iter2d) << "Emplace on non-empty list produced invalid links";
    ++iter2d;
    EXPECT_EQ(std::string("123"), *iter2d) << "Emplace on non-empty list produced invalid links";
}

TEST(emplacing_list_test, single_emplace_middle)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("123");
    list1.emplace_back("789");
    auto iter1a = list1.begin();
    ++iter1a;
    EXPECT_EQ(std::string("456"), *(list1.emplace(iter1a, "456"))) << "Emplace in the middle of a non-empty list produced incorrect return iterator";
    auto iter1b = list1.rbegin();
    EXPECT_EQ(std::string("789"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("456"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("123"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
}

TEST(emplacing_list_test, multi_same_emplace_middle)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("aaa");
    list1.emplace_back("ddd");
    auto iter1a = list1.begin();
    ++iter1a;
    EXPECT_EQ(std::string("bbb"), *(list1.emplace(iter1a, "bbb"))) << "Emplace in the middle of a non-empty list produced incorrect return iterator";
    EXPECT_EQ(std::string("ccc"), *(list1.emplace(iter1a, "ccc"))) << "Emplace in the middle of a non-empty list produced incorrect return iterator";
    auto iter1b = list1.begin();
    EXPECT_EQ(std::string("aaa"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("bbb"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("ccc"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("ddd"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
}

TEST(emplacing_list_test, multi_moving_emplace_middle)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("aaa");
    list1.emplace_back("eee");
    auto iter1a = list1.begin();
    ++iter1a;
    EXPECT_EQ(std::string("ccc"), *(list1.emplace(iter1a, "ccc"))) << "Emplace in the middle of a non-empty list produced incorrect return iterator";
    --iter1a;
    EXPECT_EQ(std::string("bbb"), *(list1.emplace(iter1a, "bbb"))) << "Emplace in the middle of a non-empty list produced incorrect return iterator";
    ++iter1a;
    EXPECT_EQ(std::string("ddd"), *(list1.emplace(iter1a, "ddd"))) << "Emplace in the middle of a non-empty list produced incorrect return iterator";
    auto iter1b = list1.begin();
    EXPECT_EQ(std::string("aaa"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("bbb"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("ccc"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("ddd"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("eee"), *iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
}

TEST(emplacing_list_test, pop_front_invalid)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.pop_front();
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list is empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list is empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When popping an already empty list the size is not 0";
    list1.emplace_front("blah");
    list1.pop_front();
    list1.pop_front();
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list is empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list is empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When popping an already empty list the size is not 0";
}

TEST(emplacing_list_test, pop_front_basic)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("blah");
    list1.pop_front();
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
    list1.emplace_front("bar");
    list1.emplace_front("foo");
    list1.pop_front();
    EXPECT_EQ(std::string("bar"), *(list1.begin())) << "When list is not empty pop_front failed";
    EXPECT_EQ(std::string("bar"), *(list1.rbegin())) << "When list is not empty pop_front failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after popping list of 2 element is not 1";
    list1.pop_front();
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
    list1.emplace_front("123");
    list1.emplace_back("456");
    list1.emplace_back("789");
    list1.pop_front();
    EXPECT_EQ(std::string("456"), *(list1.begin())) << "When list is not empty pop_front failed";
    EXPECT_EQ(std::string("789"), *(list1.rbegin())) << "When list is not empty pop_front failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after popping list of 3 element is not 2";
    auto iter1a = list1.cbegin();
    ++iter1a;
    EXPECT_EQ(std::string("789"), *iter1a) << "After popping the next links in the list have become invalid";
    auto iter1b = list1.crbegin();
    ++iter1b;
    EXPECT_EQ(std::string("456"), *iter1b) << "After popping the previous links in the list have become invalid";
    list1.pop_front();
    EXPECT_EQ(std::string("789"), *(list1.begin())) << "When list is not empty pop_front failed";
    EXPECT_EQ(std::string("789"), *(list1.rbegin())) << "When list is not empty pop_front failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after popping list of 2 element is not 1";
}

TEST(emplacing_list_test, pop_back_invalid)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.pop_back();
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list is empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list is empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When popping an already empty list the size is not 0";
    list1.emplace_back("blah");
    list1.pop_back();
    list1.pop_back();
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list is empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list is empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When popping an already empty list the size is not 0";
}

TEST(emplacing_list_test, pop_back_basic)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_back("blah");
    list1.pop_back();
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
    list1.emplace_back("bar");
    list1.emplace_back("foo");
    list1.pop_back();
    EXPECT_EQ(std::string("bar"), *(list1.begin())) << "When list is not empty pop_back failed";
    EXPECT_EQ(std::string("bar"), *(list1.rbegin())) << "When list is not empty pop_back failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after popping list of 2 element is not 1";
    list1.pop_back();
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
    list1.emplace_front("123");
    list1.emplace_back("456");
    list1.emplace_back("789");
    list1.pop_back();
    EXPECT_EQ(std::string("123"), *(list1.begin())) << "When list is not empty pop_back failed";
    EXPECT_EQ(std::string("456"), *(list1.rbegin())) << "When list is not empty pop_back failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after popping list of 3 element is not 2";
    auto iter1a = list1.cbegin();
    ++iter1a;
    EXPECT_EQ(std::string("456"), *iter1a) << "After popping the next links in the list have become invalid";
    auto iter1b = list1.crbegin();
    ++iter1b;
    EXPECT_EQ(std::string("123"), *iter1b) << "After popping the previous links in the list have become invalid";
    list1.pop_back();
    EXPECT_EQ(std::string("123"), *(list1.begin())) << "When list is not empty pop_back failed";
    EXPECT_EQ(std::string("123"), *(list1.rbegin())) << "When list is not empty pop_back failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after popping list of 2 element is not 1";
}

TEST(emplacing_list_test, erase_invalid)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    EXPECT_EQ(list1.end(), list1.erase(list1.cbegin())) << "When the list is empty the erase result iterator is not equal to end iterator";
    EXPECT_EQ(list1.end(), list1.erase(list1.cend())) << "When the list is empty the erase result iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When erasing an already empty list the size is not 0";
    list1.emplace_front("blah");
    list1.erase(list1.cbegin());
    EXPECT_EQ(list1.end(), list1.erase(list1.cbegin())) << "When the list is empty the erase result iterator is not equal to end iterator";
    EXPECT_EQ(list1.end(), list1.erase(list1.cbegin())) << "When the list is empty the erase result iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When erasing an already empty list the size is not 0";
}

TEST(emplacing_list_test, erase_front)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("blah");
    EXPECT_EQ(list1.end(), list1.erase(list1.cbegin())) << "Iterator returned by erasing the only element is not the end iterator";
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
    list1.emplace_front("bar");
    list1.emplace_front("foo");
    EXPECT_EQ(std::string("bar"), *(list1.erase(list1.cbegin()))) << "Iterator returned by erasing the first element is not the next element iterator";
    EXPECT_EQ(std::string("bar"), *(list1.begin())) << "When list is not empty erase failed";
    EXPECT_EQ(std::string("bar"), *(list1.rbegin())) << "When list is not empty erase failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after erasing list of 2 element is not 1";
    EXPECT_EQ(list1.end(), list1.erase(list1.cbegin())) << "Iterator returned by erasing the only element is not the end iterator";
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
    list1.emplace_front("123");
    list1.emplace_back("456");
    list1.emplace_back("789");
    EXPECT_EQ(std::string("456"), *(list1.erase(list1.cbegin()))) << "Iterator returned by erasing the first element is not the next element iterator";
    EXPECT_EQ(std::string("456"), *(list1.begin())) << "When list is not empty erase failed";
    EXPECT_EQ(std::string("789"), *(list1.rbegin())) << "When list is not empty erase failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after erasing list of 3 element is not 2";
    auto iter1a = list1.cbegin();
    ++iter1a;
    EXPECT_EQ(std::string("789"), *iter1a) << "After erasing the next links in the list have become invalid";
    auto iter1b = list1.crbegin();
    ++iter1b;
    EXPECT_EQ(std::string("456"), *iter1b) << "After erasing the previous links in the list have become invalid";
    EXPECT_EQ(std::string("789"), *(list1.erase(list1.cbegin()))) << "Iterator returned by erasing the first element is not the next element iterator";
    EXPECT_EQ(std::string("789"), *(list1.begin())) << "When list is not empty erase failed";
    EXPECT_EQ(std::string("789"), *(list1.rbegin())) << "When list is not empty erase failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after erasing list of 2 element is not 1";
    EXPECT_EQ(list1.end(), list1.erase(list1.cbegin())) << "Iterator returned by erasing the only element is not the end iterator";
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
}

TEST(emplacing_list_test, erase_back)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_back("blah");
    auto iter1a = list1.cbegin();
    EXPECT_EQ(list1.end(), list1.erase(iter1a)) << "Iterator returned by erasing last element is not the end iterator";
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
    list1.emplace_back("foo");
    list1.emplace_back("bar");
    auto iter1b = list1.cbegin();
    ++iter1b;
    EXPECT_EQ(list1.end(), list1.erase(iter1b)) << "Iterator returned by erasing last element is not the end iterator";
    EXPECT_EQ(std::string("foo"), *(list1.begin())) << "When list is not empty erase failed";
    EXPECT_EQ(std::string("foo"), *(list1.rbegin())) << "When list is not empty erase failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after erasing list of 2 element is not 1";
    auto iter1c = list1.cbegin();
    EXPECT_EQ(list1.end(), list1.erase(iter1c)) << "Iterator returned by erasing last element is not the end iterator";
    EXPECT_EQ(list1.cend(), list1.cbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(list1.crend(), list1.crbegin()) << "When the list becomes empty begin iterator is not equal to end iterator";
    EXPECT_EQ(0U, list1.size()) << "When the list becomes empty size is not 0";
    list1.emplace_front("123");
    list1.emplace_back("456");
    list1.emplace_back("789");
    auto iter1d = list1.cbegin();
    ++iter1d;
    ++iter1d;
    EXPECT_EQ(list1.end(), list1.erase(iter1d)) << "Iterator returned by erasing last element is not the end iterator";
    EXPECT_EQ(std::string("123"), *(list1.begin())) << "When list is not empty erase failed";
    EXPECT_EQ(std::string("456"), *(list1.rbegin())) << "When list is not empty erase failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after erasing list of 3 element is not 2";
    auto iter1e = list1.cbegin();
    ++iter1e;
    EXPECT_EQ(std::string("456"), *iter1e) << "After erasing the next links in the list have become invalid";
    auto iter1f = list1.crbegin();
    ++iter1f;
    EXPECT_EQ(std::string("123"), *iter1f) << "After erasing the previous links in the list have become invalid";
    auto iter1g = list1.cbegin();
    ++iter1g;
    EXPECT_EQ(list1.end(), list1.erase(iter1g)) << "Iterator returned by erasing last element is not the end iterator";
    EXPECT_EQ(std::string("123"), *(list1.begin())) << "When list is not empty erase failed";
    EXPECT_EQ(std::string("123"), *(list1.rbegin())) << "When list is not empty erase failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after erasing list of 2 element is not 1";
}

TEST(emplacing_list_test, single_erase_middle)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_back("123");
    list1.emplace_back("456");
    list1.emplace_back("789");
    auto iter1a = list1.cbegin();
    ++iter1a;
    EXPECT_EQ(std::string("789"), *(list1.erase(iter1a))) << "Erase did not return an iterator to the succeeding element";
    auto iter1b = list1.cbegin();
    EXPECT_EQ(std::string("123"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("789"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(list1.cend(), iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
    auto iter1c = list1.crbegin();
    EXPECT_EQ(std::string("789"), *iter1c) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1c;
    EXPECT_EQ(std::string("123"), *iter1c) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1c;
    EXPECT_EQ(list1.crend(), iter1c) << "Emplace in the middle of a non-empty list produced invalid links";
}

TEST(emplacing_list_test, multi_moving_erase_middle)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_back("aaa");
    list1.emplace_back("bbb");
    list1.emplace_back("ccc");
    list1.emplace_back("ddd");
    list1.emplace_back("eee");
    auto iter = list1.begin();
    ++iter;
    iter = list1.erase(iter);
    EXPECT_EQ(std::string("ccc"), *iter) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter;
    iter = list1.erase(iter);
    EXPECT_EQ(std::string("eee"), *iter) << "Erase in the middle of a non-empty list produced invalid links";
    auto iter1a = list1.cbegin();
    EXPECT_EQ(std::string("aaa"), *iter1a) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1a;
    EXPECT_EQ(std::string("ccc"), *iter1a) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1a;
    EXPECT_EQ(std::string("eee"), *iter1a) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1a;
    EXPECT_EQ(list1.cend(), iter1a) << "Emplace in the middle of a non-empty list produced invalid links";
    auto iter1b = list1.crbegin();
    EXPECT_EQ(std::string("eee"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("ccc"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("aaa"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(list1.crend(), iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
}

TEST(emplacing_list_test, multi_consecutive_erase_middle)
{
    typedef tco::emplacing_list<std::string, tme::concurrent_sized_slab> string_list;
    tme::concurrent_sized_slab allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_back("aaa");
    list1.emplace_back("bbb");
    list1.emplace_back("ccc");
    list1.emplace_back("ddd");
    list1.emplace_back("eee");
    list1.emplace_back("fff");
    auto iter = list1.begin();
    ++iter;
    ++iter;
    iter = list1.erase(iter);
    EXPECT_EQ(std::string("ddd"), *iter) << "Erase in the middle of a non-empty list produced invalid links";
    iter = list1.erase(iter);
    EXPECT_EQ(std::string("eee"), *iter) << "Erase in the middle of a non-empty list produced invalid links";
    auto iter1a = list1.cbegin();
    EXPECT_EQ(std::string("aaa"), *iter1a) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1a;
    EXPECT_EQ(std::string("bbb"), *iter1a) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1a;
    EXPECT_EQ(std::string("eee"), *iter1a) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1a;
    EXPECT_EQ(std::string("fff"), *iter1a) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1a;
    EXPECT_EQ(list1.cend(), iter1a) << "Emplace in the middle of a non-empty list produced invalid links";
    auto iter1b = list1.crbegin();
    EXPECT_EQ(std::string("fff"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("eee"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("bbb"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(std::string("aaa"), *iter1b) << "Erase in the middle of a non-empty list produced invalid links";
    ++iter1b;
    EXPECT_EQ(list1.crend(), iter1b) << "Emplace in the middle of a non-empty list produced invalid links";
}

class emplacing_list_perf_test : public ::testing::Test
{
public:
    typedef tco::emplacing_list<std::uint32_t, tme::concurrent_sized_slab> uint_list;
    emplacing_list_perf_test()
	:
	    allocator1(8U, { {uint_list::allocation_size(), std::numeric_limits<std::uint16_t>::max()} })
    { }
protected:
    tme::concurrent_sized_slab allocator1;
};

TEST_F(emplacing_list_perf_test, perf_test_list_emplace)
{
    typedef tco::emplacing_list<std::uint32_t, tme::concurrent_sized_slab> uint_list;
    uint_list list1(allocator1);
    list1.emplace_front(0U);
    list1.emplace_back(0U);
    auto iter = list1.begin();
    ++iter;
    std::random_device device;
    for (std::uint32_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max() - 2U; ++counter)
    {
	std::uint32_t value = device() >> 16U;
	list1.emplace(iter, value);
    }
}

TEST_F(emplacing_list_perf_test, perf_test_stdlist_emplace)
{
    typedef std::list<std::uint32_t> uint_list;
    uint_list list1;
    list1.emplace_front(0U);
    list1.emplace_back(0U);
    auto iter = list1.begin();
    ++iter;
    std::random_device device;
    for (std::uint32_t counter = 0U; counter <= std::numeric_limits<std::uint16_t>::max() - 2U; ++counter)
    {
	std::uint32_t value = device() >> 16U;
	list1.emplace(iter, value);
    }
}
