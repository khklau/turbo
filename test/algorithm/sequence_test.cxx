#include <turbo/algorithm/sequence.hpp>
#include <turbo/algorithm/sequence.hxx>
#include <gtest/gtest.h>

namespace tal = turbo::algorithm;

TEST(sequence_test, invalid_swap)
{
    char value1 = 'M';
    std::mutex mutex1a;
    std::mutex mutex1b;
    tal::swap(value1, mutex1a, value1, mutex1b);
    EXPECT_EQ('M', value1) << "swap on same value changed the value";

    char value2 = 'P';
    std::mutex mutex2a;
    tal::swap(value2, mutex2a, value2, mutex2a);
    EXPECT_EQ('P', value2) << "swap on same value changed the value";
}

TEST(sequence_test, basic_swap)
{
    char value1 = 'M';
    std::mutex mutex1;
    char value2 = 'P';
    std::mutex mutex2;
    tal::swap(value1, mutex1, value2, mutex2);
    EXPECT_EQ('P', value1) << "swap failed";
    EXPECT_EQ('M', value2) << "swap failed";
}
