#include <turbo/container/emplacing_list.hpp>
#include <turbo/container/emplacing_list.hxx>
#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;
namespace tme = turbo::memory;

TEST(emplacing_list_test, empty_list)
{
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    EXPECT_EQ(list1.end(), list1.begin()) << "When list is empty begin and end are not equal";
    EXPECT_EQ(list1.rend(), list1.rbegin()) << "When list is empty rbegin and rend are not equal";
    EXPECT_EQ(0U, list1.size()) << "Size of empty list is not 0";
}

TEST(emplacing_list_test, emplace_front_basic)
{
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_front("foobar");
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is empty emplace_front failed";
    EXPECT_EQ(std::string("foobar"), *(list1.rbegin())) << "When list is empty emplace_front failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after emplacing to an empty list is not 1";
    list1.emplace_front("blah");
    EXPECT_EQ(std::string("blah"), *(list1.begin())) << "When list is not empty emplace_front failed";
    EXPECT_EQ(std::string("foobar"), *(list1.rbegin())) << "When list is not empty emplace_front failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after emplacing to a list of 1 element is not 2";
}

TEST(emplacing_list_test, emplace_back_basic)
{
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
    string_list list1(allocator1);
    list1.emplace_back("foobar");
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is empty emplace_back failed";
    EXPECT_EQ(std::string("foobar"), *(list1.rbegin())) << "When list is empty emplace_back failed";
    EXPECT_EQ(1U, list1.size()) << "Size of list after emplacing to an empty list is not 1";
    list1.emplace_back("blah");
    EXPECT_EQ(std::string("foobar"), *(list1.begin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(std::string("blah"), *(list1.rbegin())) << "When list is not empty emplace_back failed";
    EXPECT_EQ(2U, list1.size()) << "Size of list after emplacing to a list of 1 element is not 2";
}

TEST(emplacing_list_test, emplace_mixed_basic)
{
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
    tme::pool allocator2(8U, { {string_list::allocation_size(), 8U} });
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
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
    tme::pool allocator2(8U, { {string_list::allocation_size(), 8U} });
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
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
    tme::pool allocator2(8U, { {string_list::allocation_size(), 8U} });
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
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
    tme::pool allocator2(8U, { {string_list::allocation_size(), 8U} });
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

TEST(emplacing_list_test, pop_front_invalid)
{
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
    typedef tco::emplacing_list<std::string, tme::pool> string_list;
    tme::pool allocator1(8U, { {string_list::allocation_size(), 8U} });
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
