#include <turbo/type_utility/enum_iterator.hpp>
#include <turbo/type_utility/enum_iterator.hxx>
#include <gtest/gtest.h>

namespace tt = turbo::type_utility;

enum class test
{
    FOO,
    BAR,
    BLAH
};

TEST(enum_iterator_test, begin)
{
    ASSERT_EQ(*(tt::enum_iterator<test, test::FOO, test::BLAH>().begin()), test::FOO) << "begin does not return the first enum value";
}

TEST(enum_iterator_test, end)
{
    auto end = tt::enum_iterator<test, test::FOO, test::BLAH>().end();
    ASSERT_NE(*end, test::FOO) << "end is equal to one of the valid enum class values";
    ASSERT_NE(*end, test::BAR) << "end is equal to one of the valid enum class values";
    ASSERT_NE(*end, test::BLAH) << "end is equal to one of the valid enum class values";
}

TEST(enum_iterator_test, cbegin)
{
    ASSERT_EQ(*(tt::enum_iterator<test, test::FOO, test::BLAH>().cbegin()), test::FOO) << "begin does not return the first enum value";
}

TEST(enum_iterator_test, cend)
{
    auto end = tt::enum_iterator<test, test::FOO, test::BLAH>().cend();
    ASSERT_NE(*end, test::FOO) << "end is equal to one of the valid enum class values";
    ASSERT_NE(*end, test::BAR) << "end is equal to one of the valid enum class values";
    ASSERT_NE(*end, test::BLAH) << "end is equal to one of the valid enum class values";
}

TEST(enum_iterator_test, single_loop)
{
    std::size_t count = 0;
    for (auto value: tt::enum_iterator<test, test::BAR, test::BAR>())
    {
	ASSERT_EQ(value, test::BAR) << "iterator does not return the correct value";
	++count;
    }
    ASSERT_EQ(1U, count) << "loop over a single value iterated more than once";
}

TEST(enum_iterator_test, full_loop)
{
    std::size_t count = 0;
    for (auto value: tt::enum_iterator<test, test::FOO, test::BLAH>())
    {
	switch (count)
	{
	    case 0:
		ASSERT_EQ(value, test::FOO) << "iterator does not return the correct value";
		break;
	    case 1:
		ASSERT_EQ(value, test::BAR) << "iterator does not return the correct value";
		break;
	    case 2:
		ASSERT_EQ(value, test::BLAH) << "iterator does not return the correct value";
		break;
	}
	++count;
    }
    ASSERT_EQ(3U, count) << "full loop over 3 value enumeration did not iterate 3 times";
}
