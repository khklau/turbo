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
    class reader
    {
    public:
	~reader();
    private:
	friend class pipe;
	typedef int handle;
	reader(handle read_end);
	reader(const reader& other) = delete;
	reader& operator=(const reader& other) = delete;
	handle read_end_;
    };
    class writer
    {
    public:
	~writer();
    private:
	friend class pipe;
	typedef int handle;
	writer(handle write_end);
	writer(const writer& other) = delete;
	writer& operator=(const writer& other) = delete;
	handle write_end_;
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
    static std::pair<reader::handle, writer::handle> init(std::vector<option>& options);
    pipe(std::pair<reader::handle, writer::handle> handles);
    reader reader_;
    writer writer_;
};

} // namespace posix
} // namespace ipc
} // namespace turbo

#endif
