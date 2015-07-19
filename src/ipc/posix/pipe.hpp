#ifndef TURBO_IPC_POSIX_PIPE_HPP
#define TURBO_IPC_POSIX_PIPE_HPP

#include <utility>
#include <vector>

namespace turbo {
namespace ipc {
namespace posix {

class pipe
{
public:
    class front
    {
    public:
	~front();
    private:
	friend class pipe;
	typedef int handle;
	front(handle front);
	front(const front& other) = delete;
	front& operator=(const front& other) = delete;
	handle front_;
    };
    class back
    {
    public:
	~back();
    private:
	friend class pipe;
	typedef int handle;
	back(handle back);
	back(const back& other) = delete;
	back& operator=(const back& other) = delete;
	handle back_;
    };
    struct process_limit_reached_error {};
    struct system_limit_reached_error {};
    enum class option
    {
	non_blocking,
	fork_compatible
    };
    pipe(std::vector<option>& options);
private:
    static std::pair<front::handle, back::handle> init(std::vector<option>& options);
    pipe(std::pair<front::handle, back::handle> handles);
    front front_;
    back back_;
};

} // namespace posix
} // namespace ipc
} // namespace turbo

#endif
