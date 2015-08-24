#ifndef TURBO_IPC_POSIX_PIPE_HPP
#define TURBO_IPC_POSIX_PIPE_HPP

#include <tuple>
#include <utility>
#include <vector>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace ipc {
namespace posix {
namespace pipe {

enum class TURBO_SYMBOL_DECL option
{
    non_blocking,
    fork_compatible
};

enum class TURBO_SYMBOL_DECL replace_result
{
    success,
    interrupted
};

enum class TURBO_SYMBOL_DECL io_result
{
    success,
    interrupted,
    would_block,
    pipe_full
};

class TURBO_SYMBOL_DECL key;
struct TURBO_SYMBOL_DECL process_limit_reached_error {};
struct TURBO_SYMBOL_DECL system_limit_reached_error {};
struct TURBO_SYMBOL_DECL race_condition_error {};
struct TURBO_SYMBOL_DECL used_after_move_error {};
struct TURBO_SYMBOL_DECL invalid_buffer_error {};

class TURBO_SYMBOL_DECL front
{
public:
    typedef int handle;
    front(const key&, const handle& handle);
    front(front&& other) noexcept;
    ~front();
    front& operator=(front&& other);
    inline const handle& get_handle() const { return handle_; }
    bool is_open() const;
    io_result read(void* buf, std::size_t requested_bytes, std::size_t& actual_bytes);
    void read_all(void* buf, std::size_t requested_bytes);
    std::size_t available() const;
    replace_result replace_stdin();
private:
    front() = delete;
    front(const front& other) = delete;
    front& operator=(const front& other) = delete;
    handle handle_;
};

class TURBO_SYMBOL_DECL back
{
public:
    typedef int handle;
    back(const key&, const handle& handle);
    back(back&& other) noexcept;
    ~back();
    back& operator=(back&& other);
    inline const handle& get_handle() const { return handle_; }
    bool is_open() const;
    io_result write(void* buf, std::size_t requested_bytes, std::size_t& actual_bytes);
    void write_all(void* buf, std::size_t requested_bytes);
    replace_result replace_stdout();
    replace_result replace_stderr();
private:
    back() = delete;
    back(const back& other) = delete;
    back& operator=(const back& other) = delete;
    handle handle_;
};

typedef std::pair<front, back> end_pair;

TURBO_SYMBOL_DECL end_pair make_pipe(std::vector<option>& options, std::size_t bufsize);

} // namespace pipe
} // namespace posix
} // namespace ipc
} // namespace turbo

#endif
