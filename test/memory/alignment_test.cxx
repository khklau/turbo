#include <turbo/memory/alignment.hpp>
#include <cstdint>
#include <gtest/gtest.h>

namespace tme = turbo::memory;

TEST(alignment_test, align_single_element)
{
    std::uint8_t buffer1[64];
    std::uint8_t* actual_ptr1 = &buffer1[0];
    std::size_t actual_size1 = sizeof(buffer1);
    if ((reinterpret_cast<std::uintptr_t>(actual_ptr1) % 2U) == 0)
    {
	++actual_ptr1;
    }
    std::uint8_t* expected_ptr1 = actual_ptr1 + 1;
    std::size_t expected_size1 = actual_size1 - 1;
    void* tmp1 = static_cast<void*>(actual_ptr1);
    tme::align(sizeof(std::uint16_t), sizeof(std::uint16_t), tmp1, actual_size1);
    actual_ptr1 = static_cast<std::uint8_t*>(tmp1);
    EXPECT_EQ(expected_ptr1, actual_ptr1) << "Pointer is not aligned";
    EXPECT_EQ(expected_size1, actual_size1) << "Incorrect available space";

    std::uint8_t buffer2[64];
    std::uint8_t* actual_ptr2 = &buffer2[0];
    std::size_t actual_size2 = sizeof(buffer2);
    if ((reinterpret_cast<std::uintptr_t>(actual_ptr2) % 2U) == 0)
    {
	++actual_ptr2;
    }
    std::uint8_t* expected_ptr2 = actual_ptr2 + 7;
    std::size_t expected_size2 = actual_size2 - 7;
    void* tmp2 = static_cast<void*>(actual_ptr2);
    tme::align(sizeof(std::uint64_t), sizeof(std::uint64_t), tmp2, actual_size2);
    actual_ptr2 = static_cast<std::uint8_t*>(tmp2);
    EXPECT_EQ(expected_ptr2, actual_ptr2) << "Pointer is not aligned";
    EXPECT_EQ(expected_size2, actual_size2) << "Incorrect available space";
}

TEST(alignment_test, align_multiple_element)
{
    std::uint8_t buffer1[64];
    std::uint8_t* actual_ptr1 = &buffer1[0];
    std::size_t actual_size1 = sizeof(buffer1);
    if ((reinterpret_cast<std::uintptr_t>(actual_ptr1) % 2U) == 0)
    {
	++actual_ptr1;
    }
    std::uint8_t* expected_ptr1 = actual_ptr1 + 3;
    std::size_t expected_size1 = actual_size1 - 3;
    void* tmp1 = static_cast<void*>(actual_ptr1);
    tme::align(sizeof(std::uint16_t) * 2, sizeof(std::uint16_t), tmp1, actual_size1);
    actual_ptr1 = static_cast<std::uint8_t*>(tmp1);
    EXPECT_EQ(expected_ptr1, actual_ptr1) << "Pointer is not aligned";
    EXPECT_EQ(expected_size1, actual_size1) << "Incorrect available space";

    std::uint8_t buffer2[64];
    std::uint8_t* actual_ptr2 = &buffer2[0];
    std::size_t actual_size2 = sizeof(buffer2);
    if ((reinterpret_cast<std::uintptr_t>(actual_ptr2) % 2U) == 0)
    {
	++actual_ptr2;
    }
    std::uint8_t* expected_ptr2 = actual_ptr2 + 15;
    std::size_t expected_size2 = actual_size2 - 15;
    void* tmp2 = static_cast<void*>(actual_ptr2);
    tme::align(sizeof(std::uint32_t) * 4, sizeof(std::uint32_t), tmp2, actual_size2);
    actual_ptr2 = static_cast<std::uint8_t*>(tmp2);
    EXPECT_EQ(expected_ptr2, actual_ptr2) << "Pointer is not aligned";
    EXPECT_EQ(expected_size2, actual_size2) << "Incorrect available space";
}

TEST(alignment_test, align_already_aligned)
{
    std::uint8_t buffer1[64];
    std::uint8_t* actual_ptr1 = &buffer1[0];
    std::size_t actual_size1 = sizeof(buffer1);
    if ((reinterpret_cast<std::uintptr_t>(actual_ptr1) % 2U) != 0)
    {
	++actual_ptr1;
    }
    std::uint8_t* expected_ptr1 = actual_ptr1;
    std::size_t expected_size1 = actual_size1;
    void* tmp1 = static_cast<void*>(actual_ptr1);
    tme::align(sizeof(std::uint16_t), sizeof(std::uint16_t), tmp1, actual_size1);
    actual_ptr1 = static_cast<std::uint8_t*>(tmp1);
    EXPECT_EQ(expected_ptr1, actual_ptr1) << "Aligned pointer was realigned";
    EXPECT_EQ(expected_size1, actual_size1) << "Space value was changed after unnecessary alignment";

    std::uint8_t buffer2[64];
    std::uint8_t* actual_ptr2 = &buffer2[0];
    std::size_t actual_size2 = sizeof(buffer2);
    void* tmp2 = static_cast<void*>(actual_ptr2);
    tme::align(sizeof(std::uint32_t) * 4, sizeof(std::uint32_t), tmp2, actual_size2);
    actual_ptr2 = static_cast<std::uint8_t*>(tmp2);
    std::uint8_t* expected_ptr2 = actual_ptr2;
    std::size_t expected_size2 = actual_size2;
    tme::align(sizeof(std::uint32_t) * 4, sizeof(std::uint32_t), tmp2, actual_size2);
    EXPECT_EQ(expected_ptr2, actual_ptr2) << "Aligned pointer was realigned";
    EXPECT_EQ(expected_size2, actual_size2) << "Space value was changed after unnecessary alignment";
}

TEST(alignment_test, align_excessive_required_alignment)
{
    std::uint8_t buffer1[4];
    std::uint8_t* ptr1 = &buffer1[0];
    std::size_t size1 = sizeof(buffer1);
    void* tmp1 = static_cast<void*>(ptr1);
    void* result1 = tme::align(sizeof(std::uint64_t), sizeof(std::uint16_t), tmp1, size1);
    ptr1 = static_cast<std::uint8_t*>(tmp1);
    EXPECT_EQ(nullptr, result1) << "When alignment is larger than size nullptr was not returned";

    std::uint8_t buffer2[8];
    std::uint8_t* ptr2 = &buffer2[0];
    std::size_t size2 = sizeof(buffer2);
    void* tmp2 = static_cast<void*>(ptr2);
    void* result2 = tme::align(sizeof(std::uint32_t) * 4, sizeof(std::uint32_t), tmp2, size2);
    ptr2 = static_cast<std::uint8_t*>(tmp2);
    EXPECT_EQ(nullptr, result2) << "When alignment is larger than size nullptr was not returned";
}

TEST(alignment_test, align_insufficient_space)
{
    std::uint8_t buffer1[8];
    std::uint8_t* ptr1 = &buffer1[0];
    std::size_t size1 = sizeof(buffer1);
    void* tmp1 = static_cast<void*>(ptr1);
    tme::align(sizeof(std::uint32_t), sizeof(std::uint32_t), tmp1, size1);
    ptr1 = static_cast<std::uint8_t*>(tmp1);
    ++ptr1;
    tmp1 = static_cast<void*>(ptr1);
    void* result1 = tme::align(sizeof(std::uint32_t), sizeof(std::uint64_t) * 2, tmp1, size1);
    ptr1 = static_cast<std::uint8_t*>(tmp1);
    EXPECT_EQ(nullptr, result1) << "When available space is too small nullptr was not returned";
}
