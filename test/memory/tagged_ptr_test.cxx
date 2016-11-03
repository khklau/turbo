#include <turbo/memory/tagged_ptr.hpp>
#include <cstdint>
#include <atomic>
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

TEST(tagged_ptr_test, type_traits)
{
    typedef tme::tagged_ptr<std::uint32_t, colour> colour_ptr;
    std::uint32_t value2 = 24U;
    colour_ptr ptr2(&value2);
    std::atomic<colour_ptr> atomic2(ptr2);
    ASSERT_TRUE(std::atomic_is_lock_free(&atomic2)) << "tagged_ptr is not atomic";
}

TEST(tagged_ptr_test, invalid_construction)
{
    typedef tme::tagged_ptr<std::uint8_t, colour> colour_ptr;
    std::unique_ptr<std::uint8_t[]> buffer1(new std::uint8_t[8]);
    ASSERT_THROW(colour_ptr(static_cast<std::uint8_t*>(&buffer1[1])), tme::unaligned_ptr_error) << "Construction with unaligned pointer succeeded";
}

TEST(tagged_ptr_test, valid_construction)
{
    struct foo
    {
	bool operator==(const foo& other) const { return id == other.id && name == other.name; }
	std::uint32_t id;
	std::string name;
    };
    typedef tme::tagged_ptr<foo, colour> colour_ptr;
    foo value1{ 21U, "blah" };
    colour_ptr ptr1(&value1);
    EXPECT_EQ(&value1, ptr1.get_ptr()) << "Pointer returned by get is not the pointer passed to constructor";
    EXPECT_EQ(value1, *ptr1) << "Deferenced value is not equal to value being pointed at";
    EXPECT_EQ(std::string("blah"), ptr1->name) << "Deferenced member value is not equal to value being pointed at";
    colour_ptr ptr2(nullptr);
    EXPECT_TRUE(ptr2.is_empty()) << "The is_empty predicate did not return true for tagged_ptr with nullptr value";
}

TEST(tagged_ptr_test, copy_basic)
{
    typedef tme::tagged_ptr<std::uint32_t, colour> colour_ptr;
    std::uint32_t value1 = 9U;
    colour_ptr ptr1(&value1);
    colour_ptr ptr2(ptr1);
    EXPECT_TRUE(ptr1 == ptr2) << "Copy constructed tagged_ptr is not equal to the original";
    std::uint32_t value2 = 7U;
    colour_ptr ptr3(&value2);
    ptr3 = ptr2;
    EXPECT_TRUE(ptr1 == ptr2) << "Copy assigned tagged_ptr is not equal to the original";
    ptr2.reset(ptr3.get_ptr());
    EXPECT_TRUE(ptr1 == ptr2) << "A tagged_ptr reset with another tagged_ptr's pointer is not equal to the original";
}

TEST(tagged_ptr_test, tag_basic)
{
    typedef tme::tagged_ptr<std::uint32_t, colour> colour_ptr;
    std::uint32_t value1 = 9U;
    colour_ptr ptr1(&value1);
    ptr1.set_tag(colour::yellow);
    EXPECT_EQ(colour::yellow, ptr1.get_tag()) << "The set_tag member function failed";
    ptr1.set_tag(colour::green);
    EXPECT_EQ(colour::green, ptr1.get_tag()) << "The set_tag member function failed";
    std::uint32_t value2 = 24U;
    colour_ptr ptr2(&value2);
    std::atomic<colour_ptr> atomic2(ptr2);
    EXPECT_TRUE(atomic2.compare_exchange_strong(ptr2, ptr2 | colour::blue, std::memory_order_acq_rel, std::memory_order_relaxed)) << "Atomic compare and exchange does not work on tagged_ptr";
    colour_ptr ptr3 = atomic2.load(std::memory_order_acquire);
    EXPECT_EQ(colour::blue, ptr3.get_tag()) << "Atomic compare and exchange failed to update tagged_ptr";
}
