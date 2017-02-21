#include <turbo/container/concurrent_vector.hpp>
#include <turbo/container/concurrent_vector.hxx>
#include <cstdint>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hxx>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;

TEST(concurrent_vector_test, invalid_construction)
{
    typedef tco::concurrent_vector<std::size_t> word_vector;
    ASSERT_THROW(word_vector(8U, 7U), turbo::container::invalid_capacity_argument) << "Initial capacity less than maximum did not cause exception";
    ASSERT_THROW(word_vector(8U, 33U), turbo::container::invalid_capacity_argument) << "Maximum capacity exponent greater than 2^32 did not cause exception";
}

TEST(concurrent_vector_test, basic_element_access)
{
    tco::concurrent_vector<std::uint32_t> vector1(2U, 3U);
    vector1.at(0) = 3U;
    vector1.at(1) = 7U;
    vector1.at(2) = vector1.at(0) + vector1.at(1);
    EXPECT_EQ(10U, vector1.at(2)) << "Element access and modification failed";
}
