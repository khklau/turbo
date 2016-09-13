#include <turbo/container/concurrent_vector.hpp>
#include <turbo/container/concurrent_vector.hxx>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;

TEST(concurrent_vector_test, basic)
{
    typedef tco::concurrent_vector<std::string> string_vector;

    string_vector vector1(4, 8);
}
