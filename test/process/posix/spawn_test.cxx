#include <cstring>
#include <string>
#include <turbo/ipc/posix/pipe.hpp>
#include <turbo/filesystem/path.hpp>
#include <turbo/process/status.hpp>
#include <turbo/process/posix/spawn.hpp>
#include <gtest/gtest.h>

namespace tf = turbo::filesystem;
namespace tip = turbo::ipc::posix;
namespace tpp = turbo::process::posix;
namespace tps = turbo::process::status;

tpp::child spawn_child(const char* exe, char* const args[], char* const env[])
{
    tpp::child&& child = tpp::spawn(exe, args, env, 2 << 16);
    const char* expected = "READY\n";
    char signal[7];
    child.err.read_all(signal, strlen(expected));
    if (strncmp(expected, signal, sizeof(signal)) != 0)
    {
	std::cerr << "ERROR: ready signal not received; instead received: " << signal << std::endl;
    }
    return std::move(child);
}

TEST(spawn_test, stdstream_check)
{
    tf::path exe = tps::current_exe_path().parent_path() /= "spawn_child";
    tpp::child&& child = spawn_child(exe.c_str(), {}, {});

    char input[] = "FOO\n";
    child.in.write_all(input, strlen(input));

    const char* expected = "FOOBAR\n";
    char output[8];
    child.out.read_all(output, strlen(expected));
    EXPECT_EQ(strncmp(expected, output, sizeof(output)), 0) << "Unexpected message from stdout: " << output;
}
