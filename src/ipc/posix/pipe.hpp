#ifndef TURBO_IPC_POSIX_PIPE_HPP
#define TURBO_IPC_POSIX_PIPE_HPP

#include <tuple>
#include <utility>
#include <vector>

namespace turbo {
namespace ipc {
namespace posix {

enum class pipe_option
{
    non_blocking,
    fork_compatible
};

enum class replace_result
{
    success,
    interrupted
};

class pipe_front;
class pipe_back;
typedef std::pair<pipe_front, pipe_back> pipe2;
struct process_limit_reached_error {};
struct system_limit_reached_error {};
struct race_condition_error {};

pipe2 make_pipe(std::vector<pipe_option>& options);

class pipe_front
{
public:
    pipe_front(pipe_front&& other) noexcept;
    ~pipe_front();
    pipe_front& operator=(pipe_front&& other);
    replace_result replace_stdin();
private:
    typedef int handle;
    friend pipe2 make_pipe(std::vector<pipe_option>& options);
    pipe_front() = delete;
    pipe_front(int options, const handle& handle);
    pipe_front(const pipe_front& other) = delete;
    pipe_front& operator=(const pipe_front& other) = delete;
    int options_;
    handle handle_;
};

class pipe_back
{
public:
    pipe_back(pipe_back&& other) noexcept;
    ~pipe_back();
    pipe_back& operator=(pipe_back&& other);
    replace_result replace_stdout();
    replace_result replace_stderr();
private:
    typedef int handle;
    friend pipe2 make_pipe(std::vector<pipe_option>& options);
    pipe_back() = delete;
    pipe_back(int options, const handle& handle);
    pipe_back(const pipe_back& other) = delete;
    pipe_back& operator=(const pipe_back& other) = delete;
    int options_;
    handle handle_;
};

class pipe
{
public:
    enum class option
    {
	non_blocking,
	fork_compatible
    };
    enum class result
    {
	success,
	interrupted
    };
    struct process_limit_reached_error {};
    struct system_limit_reached_error {};
    struct race_condition_error {};
    explicit pipe(std::vector<option>& options);
    ~pipe();
    result replace_stdin();
    result replace_stdout();
    result replace_stderr();
private:
    typedef int handle;
    std::tuple<int, handle, handle> init(std::vector<option>& options);
    explicit pipe(std::tuple<int, handle, handle> args);
    pipe(const pipe& other) = delete;
    pipe& operator=(const pipe& other) = delete;
    result replace(handle end, handle stdstream);
    int options_;
    handle front_;
    handle back_;
};

} // namespace posix
} // namespace ipc
} // namespace turbo

#endif
