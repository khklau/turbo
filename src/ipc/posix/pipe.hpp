#ifndef TURBO_IPC_POSIX_PIPE_HPP
#define TURBO_IPC_POSIX_PIPE_HPP

#include <tuple>
#include <vector>

namespace turbo {
namespace ipc {
namespace posix {

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
