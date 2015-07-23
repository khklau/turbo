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

class front;
class back;
typedef std::pair<front, back> end_pair;
struct process_limit_reached_error {};
struct system_limit_reached_error {};
struct race_condition_error {};

end_pair make_pipe(std::vector<option>& options);

class front
{
public:
    front(front&& other) noexcept;
    ~front();
    front& operator=(front&& other);
    replace_result replace_stdin();
private:
    typedef int handle;
    friend end_pair make_pipe(std::vector<option>& options);
    front() = delete;
    front(int options, const handle& handle);
    front(const front& other) = delete;
    front& operator=(const front& other) = delete;
    int options_;
    handle handle_;
};

class back
{
public:
    back(back&& other) noexcept;
    ~back();
    back& operator=(back&& other);
    replace_result replace_stdout();
    replace_result replace_stderr();
private:
    typedef int handle;
    friend end_pair make_pipe(std::vector<option>& options);
    back() = delete;
    back(int options, const handle& handle);
    back(const back& other) = delete;
    back& operator=(const back& other) = delete;
    int options_;
    handle handle_;
};

} // namespace pipe
} // namespace posix
} // namespace ipc
} // namespace turbo

#endif
