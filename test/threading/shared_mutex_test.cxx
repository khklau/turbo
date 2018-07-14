#include <turbo/threading/shared_mutex.hpp>
#include <turbo/threading/shared_lock.hpp>
#include <turbo/threading/shared_lock.hh>
#include <gtest/gtest.h>
#include <asio/io_service.hpp>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace tth = turbo::threading;

class write_task
{
public:
    write_task(tth::shared_mutex& mutex, std::string& shared_value);
    ~write_task();
    bool is_running() const { return thread_ != nullptr; }
    void start();
    void stop();
    template <typename then_f>
    void write(const std::string& value, then_f&& then_func);
private:
    void run();
    void exec_stop();
    template <typename then_f>
    void exec_write(const std::string& value, then_f&& then_func);
    tth::shared_mutex& mutex_;
    std::string& shared_value_;
    std::thread* thread_;
    asio::io_service service_;
};

write_task::write_task(tth::shared_mutex& mutex, std::string& shared_value)
    :
	mutex_(mutex),
	shared_value_(shared_value),
	thread_(nullptr),
	service_()
{ }

write_task::~write_task()
{
    stop();
    if (is_running())
    {
	thread_->join();
	delete thread_;
    }
}

void write_task::start()
{
    if (!is_running())
    {
	thread_ = new std::thread(std::bind(&write_task::run, this));
    }
}

void write_task::stop()
{
    service_.post(std::bind(&write_task::exec_stop, this));
}

void write_task::run()
{
    EXPECT_TRUE(1U <= service_.run());
    service_.reset();
}

template <typename then_f>
void write_task::write(const std::string& value, then_f&& then_func)
{
    service_.post([this, value, &then_func]() -> void
    {
	this->exec_write<then_f>(value, std::forward<then_f&&>(then_func));
    });
}

void write_task::exec_stop()
{
    service_.stop();
}

template <typename then_f>
void write_task::exec_write(const std::string& value, then_f&& then_func)
{
    {
	std::unique_lock<tth::shared_mutex> lock(mutex_);
	shared_value_ = value;
    }
    then_func();
}

class read_task
{
public:
    read_task(tth::shared_mutex& mutex, std::string& shared_value, const std::string& expected_value);
    ~read_task();
    bool is_running() const { return thread_ != nullptr; }
    void start();
    void stop();
    void read();
private:
    void run();
    void exec_stop();
    void exec_read();
    tth::shared_mutex& mutex_;
    std::string& shared_value_;
    std::thread* thread_;
    asio::io_service service_;
    std::string expected_;
};

read_task::read_task(tth::shared_mutex& mutex, std::string& shared_value, const std::string& expected_value)
    :
	mutex_(mutex),
	shared_value_(shared_value),
	thread_(nullptr),
	service_(),
	expected_(expected_value)
{ }

read_task::~read_task()
{
    stop();
    if (is_running())
    {
	thread_->join();
	delete thread_;
    }
}

void read_task::start()
{
    if (!is_running())
    {
	thread_ = new std::thread(std::bind(&read_task::run, this));
    }
}

void read_task::stop()
{
    service_.post(std::bind(&read_task::exec_stop, this));
}

void read_task::run()
{
    EXPECT_TRUE(1U <= service_.run());
    service_.reset();
}

void read_task::read()
{
    service_.post([&]() -> void
    {
	this->exec_read();
    });
}

void read_task::exec_stop()
{
    service_.stop();
}

void read_task::exec_read()
{
    std::string value;
    {
	tth::shared_lock<tth::shared_mutex> lock(mutex_);
	value = shared_value_;
    }
    auto result = std::search(value.cbegin(), value.cend(), expected_.cbegin(), expected_.cend());
    if (result == value.cend())
    {
	this->read();
    }
    else
    {
	EXPECT_EQ(value.cbegin(), result) << "read failed";
    }
}

TEST(shared_mutex_test, basic_read)
{
    tth::shared_mutex mutex1;
    std::string value1("foo");
    {
	read_task reader1(mutex1, value1, "foo");
	read_task reader2(mutex1, value1, "foo");
	read_task reader3(mutex1, value1, "foo");
	reader1.read();
	reader2.read();
	reader3.read();
	reader1.start();
	reader2.start();
	reader3.start();
    }
}

TEST(shared_mutex_test, basic_write)
{
    tth::shared_mutex mutex1;
    std::string value1("foo");
    {
	read_task reader1(mutex1, value1, "bar");
	read_task reader2(mutex1, value1, "bar");
	read_task reader3(mutex1, value1, "bar");
	write_task writer1(mutex1, value1);
	write_task writer2(mutex1, value1);
	reader1.read();
	reader2.read();
	reader3.read();
	writer1.write("bar1", [&]() -> void { });
	writer2.write("bar2", [&]() -> void { });
	reader1.start();
	reader2.start();
	reader3.start();
	writer1.start();
	writer2.start();
    }
}

TEST(shared_mutex_test, mixed_locks)
{
    tth::shared_mutex mutex1;
    {
	std::unique_lock<tth::shared_mutex> lock1a(mutex1, std::defer_lock);
	EXPECT_TRUE(lock1a.try_lock()) << "1st lock (unique) failed";
    }
    {
	tth::shared_lock<tth::shared_mutex> lock1b(mutex1, std::defer_lock);
	EXPECT_TRUE(lock1b.try_lock()) << "2nd lock (shared) failed";
    }
    {
	std::unique_lock<tth::shared_mutex> lock1c(mutex1, std::defer_lock);
	EXPECT_TRUE(lock1c.try_lock()) << "3rd lock (unique) failed";
    }
    {
	tth::shared_lock<tth::shared_mutex> lock1d(mutex1, std::defer_lock);
	EXPECT_TRUE(lock1d.try_lock()) << "4th lock (shared) failed";
    }

    tth::shared_mutex mutex2;
    {
	tth::shared_lock<tth::shared_mutex> lock2a(mutex2, std::defer_lock);
	EXPECT_TRUE(lock2a.try_lock()) << "2nd lock (shared) failed";
    }
    {
	std::unique_lock<tth::shared_mutex> lock2b(mutex2, std::defer_lock);
	EXPECT_TRUE(lock2b.try_lock()) << "1st lock (unique) failed";
    }
    {
	tth::shared_lock<tth::shared_mutex> lock2c(mutex2, std::defer_lock);
	EXPECT_TRUE(lock2c.try_lock()) << "4th lock (shared) failed";
    }
    {
	std::unique_lock<tth::shared_mutex> lock2d(mutex2, std::defer_lock);
	EXPECT_TRUE(lock2d.try_lock()) << "3rd lock (unique) failed";
    }
}
