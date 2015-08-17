#include "signal_notifier.hpp"

namespace turbo {
namespace ipc {
namespace posix {

signal_notifier::signal_notifier() :
    thread_(nullptr), service_(), sigset_(service_)
{ }

signal_notifier::~signal_notifier()
{
    try
    {
        for (std::size_t iter = 0; iter < MAX_SIGNAL; ++iter)
        {
                chanset_[iter].clear();
        }
        if (!service_.stopped())
        {
            service_.stop();
            if (running())
            {
                thread_->join();
                delete thread_;
            }
        }
    }
    catch (...)
    {
        // do nothing
    }
}

void signal_notifier::add(int signal, const receiver& slot)
{
    std::size_t usignal = static_cast<std::size_t>(signal);
    if (!running() && usignal < MAX_SIGNAL)
    {
        chanset_[usignal].push_back(slot);
        sigset_.add(signal);
    }
}

void signal_notifier::reset()
{
    std::function<void (const asio::error_code&, int)> handler = std::bind(
	    &signal_notifier::handle,
	    this,
	    std::placeholders::_1,
	    std::placeholders::_2);
    sigset_.async_wait(handler);
}

void signal_notifier::start()
{
    if (!running())
    {
        std::function<void ()> entry(std::bind(&signal_notifier::run, this));
        thread_ = new std::thread(entry);
    }
}

void signal_notifier::stop()
{
    service_.stop();
}

void signal_notifier::run()
{
    reset();
    service_.run();
}

void signal_notifier::handle(const asio::error_code& error, int signal)
{
    std::size_t usignal = static_cast<std::size_t>(signal);
    if (!error && usignal < MAX_SIGNAL)
    {
	for (auto iter = chanset_[usignal].cbegin(); iter != chanset_[usignal].cend(); ++iter)
	{
	    (*iter)();
	}
    }
    reset();
}

} // namespace posix
} // namespace ipc
} // namespace turbo
