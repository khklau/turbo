#include <turbo/memory/alignment.hpp>
#include <cstdint>
#include <gtest/gtest.h>

namespace tme = turbo::memory;

TEST(alignment_test, align_single_element)
{
    std::uint8_t buffer[64];
    std::uint8_t* pointer1 = &buffer[0];
    if ((reinterpret_cast<std::uintptr_t>(pointer1) % 2U) == 0)
    {
	++pointer1;
    }
    std::uint8_t* expected1 = pointer1 + 1;
    std::size_t size1 = sizeof(buffer);
    void* tmp1 = static_cast<void*>(pointer1);
    tme::align(sizeof(std::uint16_t), sizeof(std::uint16_t), tmp1, size1);
    pointer1 = static_cast<std::uint8_t*>(tmp1);
    EXPECT_EQ(expected1, pointer1) << "Pointer is not aligned";
}
