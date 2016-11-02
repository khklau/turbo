#include <turbo/memory/marked_ptr.hpp>
#include <cstdint>
#include <memory>
#include <gtest/gtest.h>

namespace tme = turbo::memory;

enum class colour : std::uint8_t
{
    red = 0,
    yellow,
    green,
    blue
};

TEST(marked_ptr_test, invalid_construction)
{
    typedef tme::marked_ptr<std::uint8_t, colour> colour_ptr;
    std::unique_ptr<std::uint8_t[]> buffer1(new std::uint8_t[8]);
    ASSERT_THROW(colour_ptr(static_cast<std::uint8_t*>(&buffer1[1])), tme::unaligned_ptr_error) << "Construction with unaligned pointer succeeded";
}

TEST(marked_ptr_test, valid_construction)
{
    struct foo
    {
	bool operator==(const foo& other) const { return id == other.id && name == other.name; }
	std::uint32_t id;
	std::string name;
    };
    typedef tme::marked_ptr<foo, colour> colour_ptr;
    foo value1{ 21U, "blah" };
    colour_ptr ptr1(&value1);
    EXPECT_EQ(&value1, ptr1.get_ptr()) << "Pointer returned by get is not the pointer passed to constructor";
    EXPECT_EQ(value1, *ptr1) << "Deferenced value is not equal to value being pointed at";
    EXPECT_EQ(std::string("blah"), ptr1->name) << "Deferenced member value is not equal to value being pointed at";
}

TEST(marked_ptr_test, copy_basic)
{
    typedef tme::marked_ptr<std::uint32_t, colour> colour_ptr;
    std::uint32_t value1 = 9U;
    colour_ptr ptr1(&value1);
    colour_ptr ptr2(ptr1);
    EXPECT_TRUE(ptr1 == ptr2) << "Copy constructed marked_ptr is not equal to the original";
    std::uint32_t value2 = 7U;
    colour_ptr ptr3(&value2);
    ptr3 = ptr2;
    EXPECT_TRUE(ptr1 == ptr2) << "Copy assigned marked_ptr is not equal to the original";
    ptr2.reset(ptr3.get_ptr());
    EXPECT_TRUE(ptr1 == ptr2) << "A marked_ptr reset with another marked_ptr's pointer is not equal to the original";
}

TEST(marked_ptr_test, mark_basic)
{
    typedef tme::marked_ptr<std::uint32_t, colour> colour_ptr;
    std::uint32_t value1 = 9U;
    colour_ptr ptr1(&value1);
    ptr1.set_mark(colour::yellow);
    EXPECT_EQ(colour::yellow, ptr1.get_mark()) << "The set_mark member function failed";
    ptr1.set_mark(colour::green);
    EXPECT_EQ(colour::green, ptr1.get_mark()) << "The set_mark member function failed";
}
