#include <cstring>
#include <string>
#include <turbo/ipc/posix/pipe.hpp>
#include <turbo/process/posix/spawn.hpp>
#include <gtest/gtest.h>

namespace tip = turbo::ipc::posix;
namespace tpp = turbo::process::posix;

tpp::child spawn_child(const char* exe, char* const args[], char* const env[])
{
    tpp::child&& child = tpp::spawn(exe, {}, {});
    const char* expected = "READY\n";
    char signal[256];
    char* signal_pos = &signal[0];
    std::size_t read_count = 0;
    ssize_t remaining_count = strlen(expected);
    do
    {
	if (remaining_count > 0 && child.err.read(signal_pos, remaining_count, read_count) == tip::pipe::io_result::success)
	{
	    remaining_count -= read_count;
	    signal_pos += (read_count / sizeof(char));
	}
	else if (remaining_count <= 0)
	{
	    remaining_count = strlen(expected);
	    std::cerr << "ERROR: ready signal not received; instead received: " << signal << std::endl;
	    //exit(99);
	}
    }
    while (strncmp(expected, signal, sizeof(signal)) != 0);
    return std::move(child);
}

TEST(spawn_test, stdstream_check)
{
    const char* exe = "/home/kean/workspace/product/turbo/build/test/turbo/process/spawn_child";
    tpp::child&& child = spawn_child(exe, {}, {});

    char input[] = "FOO\n";
    char* input_pos = input;
    std::size_t write_count = 0;
    ssize_t remaining_count = strlen(input);
    do
    {
	if (remaining_count > 0 && child.in.write(input_pos, remaining_count, write_count) == tip::pipe::io_result::success)
	{
	    remaining_count -= write_count;
	    input_pos += (write_count / sizeof(char));
	}
    }
    while (remaining_count > 0);

    const char* expected = "FOOBAR\n";
    char output[256];
    char* output_pos = &output[0];
    std::size_t read_count = 0;
    remaining_count = strlen(expected);
    do
    {
	if (remaining_count > 0 && child.out.read(output_pos, remaining_count, read_count) == tip::pipe::io_result::success)
	{
	    remaining_count -= read_count;
	    output_pos += (read_count / sizeof(char));
	}
    }
    while (remaining_count > 0);

    EXPECT_EQ(strncmp(expected, output, sizeof(output)), 0) << "Unexpected message from stdout: " << output;
}
