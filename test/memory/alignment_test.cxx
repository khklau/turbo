#include <turbo/memory/alignment.hpp>
#include <turbo/memory/alignment.hxx>
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

    std::uint8_t buffer2[8];
    std::uint8_t* ptr2 = &buffer2[0];
    std::size_t size2 = 0U;
    void* tmp2 = static_cast<void*>(ptr2);
    void* result2 = tme::align(sizeof(std::uint32_t) * 4, sizeof(std::uint32_t), tmp2, size2);
    ptr2 = static_cast<std::uint8_t*>(tmp2);
    EXPECT_EQ(nullptr, result2) << "When alignment is larger than size nullptr was not returned";
}

TEST(alignment_test, align_void_pointer)
{
    void* tmp1 = nullptr;
    std::size_t size1 = 4U;
    void* result1 = tme::align(sizeof(std::uint16_t), sizeof(std::uint16_t), tmp1, size1);
    EXPECT_EQ(nullptr, result1) << "When null pointer argument was given a nullptr was not returned";
}

TEST(alignment_test, align_zero_alignment)
{
    std::uint8_t buffer1[64];
    std::uint8_t* actual_ptr1 = &buffer1[0];
    std::size_t actual_size1 = sizeof(buffer1);
    if ((reinterpret_cast<std::uintptr_t>(actual_ptr1) % 2U) == 0)
    {
	++actual_ptr1;
    }
    std::uint8_t* expected_ptr1 = actual_ptr1;
    std::size_t expected_size1 = actual_size1;
    void* tmp1 = static_cast<void*>(actual_ptr1);
    tme::align(0U, sizeof(std::uint16_t), tmp1, actual_size1);
    actual_ptr1 = static_cast<std::uint8_t*>(tmp1);
    EXPECT_EQ(expected_ptr1, actual_ptr1) << "Pointer was aligned when alignment is zero";
    EXPECT_EQ(expected_size1, actual_size1) << "Space variable changed when alignment is zero";
}

TEST(alignment_test, align_zero_element_size)
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
    tme::align(sizeof(std::uint16_t), 0U, tmp1, actual_size1);
    actual_ptr1 = static_cast<std::uint8_t*>(tmp1);
    EXPECT_EQ(expected_ptr1, actual_ptr1) << "Pointer is not aligned";
    EXPECT_EQ(expected_size1, actual_size1) << "Incorrect available space";
}

TEST(alignment_test, calc_total_aligned_size_invalid_value)
{
    EXPECT_EQ(0U, tme::calc_total_aligned_size(0U, 0U, 1U)) << "Total size is not zero when the value size is zero";
    EXPECT_EQ(0U, tme::calc_total_aligned_size(0U, 4U, 1U)) << "Total size is not zero when the value size is zero";
    EXPECT_EQ(0U, tme::calc_total_aligned_size(0U, 0U, 8U)) << "Total size is not zero when the value size is zero";
    EXPECT_EQ(0U, tme::calc_total_aligned_size(0U, 4U, 8U)) << "Total size is not zero when the value size is zero";
}

TEST(alignment_test, calc_total_aligned_size_invalid_quantity)
{
    EXPECT_EQ(0U, tme::calc_total_aligned_size(1U, 0U, 0U)) << "Total size is not zero when the quantity is zero";
    EXPECT_EQ(0U, tme::calc_total_aligned_size(1U, 4U, 0U)) << "Total size is not zero when the quantity is zero";
    EXPECT_EQ(0U, tme::calc_total_aligned_size(8U, 0U, 0U)) << "Total size is not zero when the quantity is zero";
    EXPECT_EQ(0U, tme::calc_total_aligned_size(8U, 4U, 0U)) << "Total size is not zero when the quantity is zero";
}

TEST(alignment_test, calc_total_aligned_size_no_alignment)
{
    EXPECT_EQ(1U, tme::calc_total_aligned_size(1U, 0U, 1U)) << "Returned size is not the total size when there is no alignment requirement";
    EXPECT_EQ(3U, tme::calc_total_aligned_size(1U, 0U, 3U)) << "Returned size is not the total size when there is no alignment requirement";;
    EXPECT_EQ(4U, tme::calc_total_aligned_size(4U, 0U, 1U)) << "Returned size is not the total size when there is no alignment requirement";
    EXPECT_EQ(32U, tme::calc_total_aligned_size(4U, 0U, 8U)) << "Returned size is not the total size when there is no alignment requirement";;
}

TEST(alignment_test, calc_total_aligned_size_already_aligned)
{
    EXPECT_EQ(1U, tme::calc_total_aligned_size(1U, 1U, 1U)) << "Returned size is not the total size when requested alignment matches value size";
    EXPECT_EQ(4U, tme::calc_total_aligned_size(4U, 4U, 1U)) << "Returned size is not the total size when requested alignment matches value size";
    EXPECT_EQ(4U, tme::calc_total_aligned_size(1U, 1U, 4U)) << "Returned size is not the total size when requested alignment matches value size";
    EXPECT_EQ(16U, tme::calc_total_aligned_size(4U, 4U, 4U)) << "Returned size is not the total size when requested alignment matches value size";
}

TEST(alignment_test, calc_total_aligned_size_smaller_value_size)
{
    EXPECT_EQ(8U, tme::calc_total_aligned_size(1U, 8U, 1U)) << "Returned size is not alignment * quantity when the value size is smaller than alignment";
    EXPECT_EQ(4U, tme::calc_total_aligned_size(2U, 4U, 1U)) << "Returned size is not alignment * quantity when the value size is smaller than alignment";
    EXPECT_EQ(64U, tme::calc_total_aligned_size(33U, 64U, 1U)) << "Returned size is not alignment * quantity when the value size is smaller than alignment";
    EXPECT_EQ(32U, tme::calc_total_aligned_size(1U, 8U, 4U)) << "Returned size is not alignment * quantity when the value size is smaller than alignment";
    EXPECT_EQ(24U, tme::calc_total_aligned_size(4U, 6U, 4U)) << "Returned size is not alignment * quantity when the value size is smaller than alignment";
    EXPECT_EQ(30U, tme::calc_total_aligned_size(4U, 6U, 5U)) << "Returned size is not alignment * quantity when the value size is smaller than alignment";
    EXPECT_EQ(64U, tme::calc_total_aligned_size(4U, 16U, 4U)) << "Returned size is not alignment * quantity when the value size is smaller than alignment";
    EXPECT_EQ(192U, tme::calc_total_aligned_size(33U, 64U, 3U)) << "Returned size is not alignment * quantity when the value size is smaller than alignment";
}

TEST(alignment_test, calc_total_aligned_size_smaller_alignment)
{
    EXPECT_EQ(2U, tme::calc_total_aligned_size(2U, 1U, 1U)) << "Expected total size rounded to nearest multiple of alignment & quantity when alignment is smaller than value size";
    EXPECT_EQ(8U, tme::calc_total_aligned_size(5U, 4U, 1U)) << "Expected total size rounded to nearest multiple of alignment & quantity when alignment is smaller than value size";
    EXPECT_EQ(16U, tme::calc_total_aligned_size(4U, 1U, 4U)) << "Expected total size rounded to nearest multiple of alignment & quantity when alignment is smaller than value size";
    EXPECT_EQ(16U, tme::calc_total_aligned_size(4U, 2U, 4U)) << "Expected total size rounded to nearest multiple of alignment & quantity when alignment is smaller than value size";
    EXPECT_EQ(60U, tme::calc_total_aligned_size(8U, 6U, 5U)) << "Expected total size rounded to nearest multiple of alignment & quantity when alignment is smaller than value size";
    EXPECT_EQ(42U, tme::calc_total_aligned_size(5U, 3U, 7U)) << "Expected total size rounded to nearest multiple of alignment & quantity when alignment is smaller than value size";
    EXPECT_EQ(45U, tme::calc_total_aligned_size(7U, 3U, 5U)) << "Expected total size rounded to nearest multiple of alignment & quantity when alignment is smaller than value size";
    EXPECT_EQ(256U, tme::calc_total_aligned_size(72U, 64U, 2U)) << "Expected total size rounded to nearest multiple of alignment & quantity when alignment is smaller than value size";
}
