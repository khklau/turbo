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

class key;
struct process_limit_reached_error {};
struct system_limit_reached_error {};
struct race_condition_error {};

class front
{
public:
    typedef int handle;
    front(const key&, int options, const handle& handle);
    front(front&& other) noexcept;
    ~front();
    front& operator=(front&& other);
    replace_result replace_stdin();
private:
    front() = delete;
    front(const front& other) = delete;
    front& operator=(const front& other) = delete;
    int options_;
    handle handle_;
};

class back
{
public:
    typedef int handle;
    back(const key&, int options, const handle& handle);
    back(back&& other) noexcept;
    ~back();
    back& operator=(back&& other);
    replace_result replace_stdout();
    replace_result replace_stderr();
private:
    back() = delete;
    back(const back& other) = delete;
    back& operator=(const back& other) = delete;
    int options_;
    handle handle_;
};

typedef std::pair<front, back> end_pair;

end_pair make_pipe(std::vector<option>& options);

} // namespace pipe
} // namespace posix
} // namespace ipc
} // namespace turbo

#endif
