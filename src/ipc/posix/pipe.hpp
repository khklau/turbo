#ifndef TURBO_IPC_POSIX_PIPE_HPP
#define TURBO_IPC_POSIX_PIPE_HPP

#include <tuple>
#include <utility>
#include <vector>

namespace turbo {
namespace ipc {
namespace posix {
namespace pipe {

enum class option
{
    non_blocking,
    fork_compatible
};

enum class replace_result
{
    success,
    interrupted
};

enum class io_result
{
    success,
    interrupted,
    would_block,
    pipe_full
};

class key;
struct process_limit_reached_error {};
struct system_limit_reached_error {};
struct race_condition_error {};
struct used_after_move_error {};
struct invalid_buffer_error {};

class front
{
public:
    typedef int handle;
    front(const key&, const handle& handle);
    front(front&& other) noexcept;
    ~front();
    front& operator=(front&& other);
    bool is_open() const;
    io_result read(void* buf, std::size_t requested_bytes, std::size_t& actual_bytes);
    replace_result replace_stdin();
private:
    front() = delete;
    front(const front& other) = delete;
    front& operator=(const front& other) = delete;
    handle handle_;
};

class back
{
public:
    typedef int handle;
    back(const key&, const handle& handle);
    back(back&& other) noexcept;
    ~back();
    back& operator=(back&& other);
    bool is_open() const;
    io_result write(void* buf, std::size_t requested_bytes, std::size_t& actual_bytes);
    replace_result replace_stdout();
    replace_result replace_stderr();
private:
    back() = delete;
    back(const back& other) = delete;
    back& operator=(const back& other) = delete;
    handle handle_;
};

typedef std::pair<front, back> end_pair;

end_pair make_pipe(std::vector<option>& options);

} // namespace pipe
} // namespace posix
} // namespace ipc
} // namespace turbo

#endif
