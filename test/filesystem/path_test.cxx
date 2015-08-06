#include <turbo/filesystem/path.hpp>
#include <gtest/gtest.h>

namespace tf = turbo::filesystem;

TEST(path_test, append_no_trail_positive)
{
    tf::path actual1("foo");
    actual1 /= "bar";
    EXPECT_EQ(std::string("foo/bar"), actual1.native()) << "appending relative path to relative path failed";

    tf::path actual2("foo");
    actual2 /= "/bar";
    EXPECT_EQ(std::string("foo/bar"), actual2.native()) << "appending absolute path to relative path failed";

    tf::path actual3("/foo");
    actual3 /= "bar";
    EXPECT_EQ(std::string("/foo/bar"), actual3.native()) << "appending relative path to absolute path failed";

    tf::path actual4("/foo");
    actual4 /= "/bar";
    EXPECT_EQ(std::string("/foo/bar"), actual4.native()) << "appending absolute path to absolute path failed";
}

TEST(path_test, append_path_trail_positive)
{
    tf::path actual1("foo/");
    actual1 /= "bar";
    EXPECT_EQ(std::string("foo/bar"), actual1.native()) << "appending relative path to relative path with trailing separator failed";

    tf::path actual2("foo/");
    actual2 /= "/bar";
    EXPECT_EQ(std::string("foo/bar"), actual2.native()) << "appending absolute path to relative path with trailing separator failed";

    tf::path actual3("/foo/");
    actual3 /= "bar";
    EXPECT_EQ(std::string("/foo/bar"), actual3.native()) << "appending relative path to absolute path with trailing separator failed";

    tf::path actual4("/foo/");
    actual4 /= "/bar";
    EXPECT_EQ(std::string("/foo/bar"), actual4.native()) << "appending absolute path to absolute path with trailing separator failed";
}

TEST(path_test, append_arg_trail_positive)
{
    tf::path actual1("foo");
    actual1 /= "bar/";
    EXPECT_EQ(std::string("foo/bar/"), actual1.native()) << "appending relative path with trailing separator to relative path failed";

    tf::path actual2("foo");
    actual2 /= "/bar/";
    EXPECT_EQ(std::string("foo/bar/"), actual2.native()) << "appending absolute path with trailing separator to relative path failed";

    tf::path actual3("/foo");
    actual3 /= "bar/";
    EXPECT_EQ(std::string("/foo/bar/"), actual3.native()) << "appending relative path with trailing separator to absolute path failed";

    tf::path actual4("/foo");
    actual4 /= "/bar/";
    EXPECT_EQ(std::string("/foo/bar/"), actual4.native()) << "appending absolute path with trailing separator to absolute path failed";
}

TEST(path_test, append_both_trail_positive)
{
    tf::path actual1("foo/");
    actual1 /= "bar/";
    EXPECT_EQ(std::string("foo/bar/"), actual1.native()) << "appending relative path to relative path with both having trailing separator failed";

    tf::path actual2("foo/");
    actual2 /= "/bar/";
    EXPECT_EQ(std::string("foo/bar/"), actual2.native()) << "appending absolute path to relative path with both having trailing separator failed";

    tf::path actual3("/foo/");
    actual3 /= "bar/";
    EXPECT_EQ(std::string("/foo/bar/"), actual3.native()) << "appending relative path to absolute path with both having trailing separator failed";

    tf::path actual4("/foo/");
    actual4 /= "/bar/";
    EXPECT_EQ(std::string("/foo/bar/"), actual4.native()) << "appending absolute path to absolute path with both having trailing separator failed";
}

TEST(path_test, append_negative)
{
    EXPECT_ANY_THROW(tf::path actual1(nullptr)) << "null path was constructed";

    tf::path actual2("");
    EXPECT_TRUE(actual2.empty()) << "empty argument did not create an empty path";

    tf::path actual3("/foo");
    EXPECT_ANY_THROW(actual3 /= nullptr) << "append to null path was accepted";

    tf::path actual4("/foo");
    actual4 /= "";
    EXPECT_EQ(std::string("/foo"), actual4.native()) << "appending empty path caused a change";
}

TEST(path_test, parent_path_positive)
{
    tf::path actual1("foo");
    EXPECT_EQ(std::string("."), actual1.parent_path().native()) << "parent of relative path is not dot";

    tf::path actual2("foo/");
    EXPECT_EQ(std::string("."), actual2.parent_path().native()) << "parent of relative path is not dot";

    tf::path actual3("foo/bar");
    EXPECT_EQ(std::string("foo"), actual3.parent_path().native()) << "parent of nested relative path is wrong";

    tf::path actual4("foo/bar/");
    EXPECT_EQ(std::string("foo"), actual4.parent_path().native()) << "parent of nested relative path is wrong";

    tf::path actual5("/foo");
    EXPECT_EQ(std::string("/"), actual5.parent_path().native()) << "parent of absolute path is not root";

    tf::path actual6("/foo/");
    EXPECT_EQ(std::string("/"), actual6.parent_path().native()) << "parent of absolute path is not root";

    tf::path actual7("/foo/bar");
    EXPECT_EQ(std::string("/foo"), actual7.parent_path().native()) << "parent of nested absolute path is wrong";

    tf::path actual8("/foo/bar/");
    EXPECT_EQ(std::string("/foo"), actual8.parent_path().native()) << "parent of nested absolute path is wrong";
}
