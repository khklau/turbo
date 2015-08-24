#ifndef TURBO_IPC_POSIX_SIGNAL_NOTIFIER_HPP
#define TURBO_IPC_POSIX_SIGNAL_NOTIFIER_HPP

#include <functional>
#include <thread>
#include <vector>
#include <asio/error_code.hpp>
#include <asio/io_service.hpp>
#include <asio/signal_set.hpp>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace ipc {
namespace posix {

class TURBO_SYMBOL_DECL signal_notifier
{
public:
    typedef std::function<void ()> receiver;
    typedef std::vector<receiver> channel;
    signal_notifier();
    ~signal_notifier();
    bool running() const { return thread_ != nullptr; }
    void add(int signal, const receiver& slot);
    void reset();
    void start();
    void stop();
private:
    static const size_t MAX_SIGNAL = 64U; // TODO : work out how to base this on SIGRTMAX
    signal_notifier(const signal_notifier& other) = delete;
    signal_notifier& operator=(const signal_notifier& other) = delete;
    void run();
    void handle(const asio::error_code& error, int signal);
    std::thread* thread_;
    asio::io_service service_;
    asio::signal_set sigset_;
    channel chanset_[MAX_SIGNAL];
};

} // namespace posix
} // namespace ipc
} // namespace turbo

#endif
