#include <turbo/container/mpmc_ring_queue.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <turbo/algorithm/recovery.hpp>

namespace tco = turbo::container;
namespace tar = turbo::algorithm::recovery;

TEST(mpmc_ring_queue_test, dequeue_basic)
{
    typedef tco::mpmc_ring_queue<std::string> string_queue;

    string_queue queue1(8, 2);
    string_queue::producer& producer1 = queue1.get_producer();
    string_queue::consumer& consumer1 = queue1.get_consumer();
    std::string expected1("foo");
    ASSERT_NE(producer1.try_enqueue_copy(expected1), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    std::string actual1;
    ASSERT_NE(consumer1.try_dequeue_copy(actual1), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    EXPECT_EQ(expected1, actual1) << "Value enqueued is not the same value dequeued";

    string_queue queue2(8, 2);
    string_queue::producer& producer2 = queue2.get_producer();
    string_queue::consumer& consumer2 = queue2.get_consumer();
    std::string expected2a("123");
    std::string expected2b("456");
    std::string expected2c("789");
    ASSERT_NE(producer2.try_enqueue_copy(expected2a), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(producer2.try_enqueue_copy(expected2b), string_queue::producer::result::queue_full) << "Queue should not be full";
    std::string actual2a;
    std::string actual2b;
    std::string actual2c;
    ASSERT_NE(consumer2.try_dequeue_copy(actual2a), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    ASSERT_NE(consumer2.try_dequeue_copy(actual2b), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(expected2a, actual2a) << "First value enqueued is not the first value dequeued";
    EXPECT_EQ(expected2a, actual2a) << "Second value enqueued is not the second value dequeued";
    ASSERT_NE(producer2.try_enqueue_copy(expected2c), string_queue::producer::result::queue_full) << "Queue should not be full";
    ASSERT_NE(consumer2.try_dequeue_copy(actual2c), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(expected2c, actual2c) << "Third value enqueued is not the third value dequeued";
}

TEST(mpmc_ring_queue_test, dequeue_move)
{
    typedef tco::mpmc_ring_queue<std::unique_ptr<std::string>> string_queue;

    string_queue queue1(8, 2);
    string_queue::producer& producer1 = queue1.get_producer();
    string_queue::consumer& consumer1 = queue1.get_consumer();
    std::string expected1("foo");
    std::unique_ptr<std::string> copy1(new std::string(expected1));
    ASSERT_NE(producer1.try_enqueue_move(std::move(copy1)), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    std::unique_ptr<std::string> actual1;
    ASSERT_NE(consumer1.try_dequeue_move(actual1), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    EXPECT_EQ(expected1, *actual1) << "Value enqueued is not the same value dequeued";

    string_queue queue2(8, 2);
    string_queue::producer& producer2 = queue2.get_producer();
    string_queue::consumer& consumer2 = queue2.get_consumer();
    std::string expected2a("123");
    std::string expected2b("456");
    std::string expected2c("789");
    std::unique_ptr<std::string> copy2a(new std::string(expected2a));
    std::unique_ptr<std::string> copy2b(new std::string(expected2b));
    std::unique_ptr<std::string> copy2c(new std::string(expected2c));
    ASSERT_NE(producer2.try_enqueue_move(std::move(copy2a)), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(producer2.try_enqueue_move(std::move(copy2b)), string_queue::producer::result::queue_full) << "Queue should not be full";
    std::unique_ptr<std::string> actual2a;
    std::unique_ptr<std::string> actual2b;
    std::unique_ptr<std::string> actual2c;
    ASSERT_NE(consumer2.try_dequeue_move(actual2a), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    ASSERT_NE(consumer2.try_dequeue_move(actual2b), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(expected2a, *actual2a) << "First value enqueued is not the first value dequeued";
    EXPECT_EQ(expected2a, *actual2a) << "Second value enqueued is not the second value dequeued";
    ASSERT_NE(producer2.try_enqueue_move(std::move(copy2c)), string_queue::producer::result::queue_full) << "Queue should not be full";
    ASSERT_NE(consumer2.try_dequeue_move(actual2c), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(expected2c, *actual2c) << "Third value enqueued is not the third value dequeued";
}

TEST(mpmc_ring_queue_test, loop_back)
{
    typedef tco::mpmc_ring_queue<std::string> string_queue;

    string_queue queue1(2, 2);
    string_queue::producer& producer1 = queue1.get_producer();
    string_queue::consumer& consumer1 = queue1.get_consumer();
    std::string expected1a("123");
    std::string expected1b("456");
    std::string expected1c("789");
    std::string actual1a;
    std::string actual1b;
    std::string actual1c;

    ASSERT_NE(producer1.try_enqueue_copy(expected1a), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(consumer1.try_dequeue_copy(actual1a), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(expected1a, actual1a) << "First value enqueued is not the first value dequeued";

    ASSERT_NE(producer1.try_enqueue_copy(expected1b), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(consumer1.try_dequeue_copy(actual1b), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(expected1b, actual1b) << "Second value enqueued is not the second value dequeued";

    ASSERT_NE(producer1.try_enqueue_copy(expected1c), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(consumer1.try_dequeue_copy(actual1c), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(expected1c, actual1c) << "Third value enqueued is not the third value dequeued";
}

TEST(mpmc_ring_queue_test, empty_queue)
{
    typedef tco::mpmc_ring_queue<std::string> string_queue;

    string_queue queue1(8, 2);
    string_queue::producer& producer1 = queue1.get_producer();
    string_queue::consumer& consumer1 = queue1.get_consumer();
    std::string expected1("foo");
    std::string actual1;
    ASSERT_EQ(consumer1.try_dequeue_copy(actual1), string_queue::consumer::result::queue_empty) << "Just initialised queue is not empty";
    ASSERT_NE(producer1.try_enqueue_copy(expected1), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(consumer1.try_dequeue_copy(actual1), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    EXPECT_EQ(consumer1.try_dequeue_copy(actual1), string_queue::consumer::result::queue_empty) << "Queue should be empty";

    string_queue queue2(8, 2);
    string_queue::producer& producer2 = queue2.get_producer();
    string_queue::consumer& consumer2 = queue2.get_consumer();
    std::string expected2a("123");
    std::string expected2b("456");
    std::string expected2c("789");
    ASSERT_NE(producer2.try_enqueue_copy(expected2a), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(producer2.try_enqueue_copy(expected2b), string_queue::producer::result::queue_full) << "Queue should not be full";
    std::string actual2a;
    std::string actual2b;
    std::string actual2c;
    ASSERT_NE(consumer2.try_dequeue_copy(actual2a), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    ASSERT_NE(consumer2.try_dequeue_copy(actual2b), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    ASSERT_EQ(consumer2.try_dequeue_copy(actual2c), string_queue::consumer::result::queue_empty) << "Queue should be empty";
    ASSERT_NE(producer2.try_enqueue_copy(expected2c), string_queue::producer::result::queue_full) << "Queue should not be full";
    ASSERT_NE(consumer2.try_dequeue_copy(actual2c), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(consumer2.try_dequeue_copy(actual2c), string_queue::consumer::result::queue_empty) << "Queue should be empty";
}

TEST(mpmc_ring_queue_test, full_queue)
{
    typedef tco::mpmc_ring_queue<std::string> string_queue;

    string_queue queue1(2, 2);
    string_queue::producer& producer1 = queue1.get_producer();
    std::string expected1a("123");
    std::string expected1b("456");
    std::string expected1c("789");
    ASSERT_NE(producer1.try_enqueue_copy(expected1a), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(producer1.try_enqueue_copy(expected1b), string_queue::producer::result::queue_full) << "Queue should not be full";
    EXPECT_EQ(producer1.try_enqueue_copy(expected1a), string_queue::producer::result::queue_full) << "Queue should be full";

    string_queue::consumer& consumer1 = queue1.get_consumer();
    std::string actual1a;
    ASSERT_NE(consumer1.try_dequeue_copy(actual1a), string_queue::consumer::result::queue_empty) << "Queue should not be empty";
    ASSERT_NE(producer1.try_enqueue_copy(expected1c), string_queue::producer::result::queue_full) << "Queue should not be full";
    EXPECT_EQ(producer1.try_enqueue_copy(expected1a), string_queue::producer::result::queue_full) << "Queue should be full";
}

template <class value_t, std::size_t limit>
class produce_task
{
public:
    typedef tco::mpmc_ring_queue<value_t> queue;
    produce_task(typename queue::producer& producer, std::array<value_t, limit>& input);
    ~produce_task() noexcept;
    void run_copy();
    void run_move();
    void produce_copy();
    void produce_move();
private:
    typename queue::producer& producer_;
    std::array<value_t, limit>& input_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
produce_task<value_t, limit>::produce_task(typename queue::producer& producer, std::array<value_t, limit>& input)
    :
	producer_(producer),
	input_(input),
	thread_(nullptr)
{ }

template <class value_t, std::size_t limit>
produce_task<value_t, limit>::~produce_task() noexcept
{
    try
    {
	if (thread_)
	{
	    thread_->join();
	    delete thread_;
	    thread_ = nullptr;
	}
    }
    catch(...)
    {
	// do nothing
    }
}

template <class value_t, std::size_t limit>
void produce_task<value_t, limit>::run_copy()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&produce_task::produce_copy, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void produce_task<value_t, limit>::run_move()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&produce_task::produce_move, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void produce_task<value_t, limit>::produce_copy()
{
    for (auto iter = input_.cbegin(); iter != input_.cend();)
    {
	tar::retry_with_random_backoff([&] () -> bool
	{
	    if (producer_.try_enqueue_copy(*iter) == queue::producer::result::success)
	    {
		++iter;
		return true;
	    }
	    else
	    {
		return false;
	    }
	});
    }
}

template <class value_t, std::size_t limit>
void produce_task<value_t, limit>::produce_move()
{
    for (auto iter = input_.begin(); iter != input_.end();)
    {
	tar::retry_with_random_backoff([&] () -> bool
	{
	    if (producer_.try_enqueue_move(std::move(*iter)) == queue::producer::result::success)
	    {
		++iter;
		return true;
	    }
	    else
	    {
		return false;
	    }
	});
    }
}

template <class value_t, std::size_t limit>
class consume_task
{
public:
    typedef tco::mpmc_ring_queue<value_t> queue;
    consume_task(typename queue::consumer& consumer, std::array<value_t, limit>& output);
    ~consume_task() noexcept;
    void run_copy();
    void run_move();
    void consume_copy();
    void consume_move();
private:
    typename queue::consumer& consumer_;
    std::array<value_t, limit>& output_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
consume_task<value_t, limit>::consume_task(typename queue::consumer& consumer, std::array<value_t, limit>& output)
    :
	consumer_(consumer),
	output_(output),
	thread_(nullptr)
{ }

template <class value_t, std::size_t limit>
consume_task<value_t, limit>::~consume_task() noexcept
{
    try
    {
	if (thread_)
	{
	    thread_->join();
	    delete thread_;
	    thread_ = nullptr;
	}
    }
    catch(...)
    {
	// do nothing
    }
}

template <class value_t, std::size_t limit>
void consume_task<value_t, limit>::run_copy()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&consume_task::consume_copy, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void consume_task<value_t, limit>::run_move()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&consume_task::consume_move, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void consume_task<value_t, limit>::consume_copy()
{
    for (auto iter = output_.begin(); iter != output_.end();)
    {
	tar::retry_with_random_backoff([&] () -> bool
	{
	    if (consumer_.try_dequeue_copy(*iter) == queue::consumer::result::success)
	    {
		++iter;
		return true;
	    }
	    else
	    {
		return false;
	    }
	});
    }
}

template <class value_t, std::size_t limit>
void consume_task<value_t, limit>::consume_move()
{
    for (auto iter = output_.begin(); iter != output_.end();)
    {
	tar::retry_with_random_backoff([&] () -> bool
	{
	    if (consumer_.try_dequeue_move(*iter) == queue::consumer::result::success)
	    {
		++iter;
		return true;
	    }
	    else
	    {
		return false;
	    }
	});
    }
}

struct record
{
    bool operator==(const record& other) const;
    uint16_t first;
    uint32_t second;
    uint64_t third;
};

bool record::operator==(const record& other) const
{
    return first == other.first && second == other.second && third == other.third;
}

TEST(mpmc_ring_queue_test, async_struct_copy)
{
    typedef tco::mpmc_ring_queue<record> record_queue;
    record_queue queue1(8U, 4U);
    std::unique_ptr<std::array<record, 2048U>> input1(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> input2(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> input3(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> input4(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> output1(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> output2(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> output3(new std::array<record, 2048U>());
    std::unique_ptr<std::array<record, 2048U>> output4(new std::array<record, 2048U>());
    for (uint64_t counter1 = 0U; counter1 < input1->max_size(); ++counter1)
    {
	uint16_t base1 = 3U + (counter1 * 5U) + 0U;
	(*input1)[counter1] = record{base1, base1 * 3U, base1 * 9UL};
    }
    for (uint64_t counter2 = 0U; counter2 < input2->max_size(); ++counter2)
    {
	uint16_t base2 = 3U + (counter2 * 5U) + 10240U;
	(*input2)[counter2] = record{base2, base2 * 3U, base2 * 9UL};
    }
    for (uint64_t counter3 = 0U; counter3 < input3->max_size(); ++counter3)
    {
	uint16_t base3 = 3U + (counter3 * 5U) + 20480;
	(*input3)[counter3] = record{base3, base3 * 3U, base3 * 9UL};
    }
    for (uint64_t counter4 = 0U; counter4 < input4->max_size(); ++counter4)
    {
	uint16_t base4 = 3U + (counter4 * 5U) + 30720U;
	(*input4)[counter4] = record{base4, base4 * 3U, base4 * 9UL};
    }
    {
	produce_task<record, 2048U> producer1(queue1.get_producer(), *input1);
	produce_task<record, 2048U> producer2(queue1.get_producer(), *input2);
	produce_task<record, 2048U> producer3(queue1.get_producer(), *input3);
	produce_task<record, 2048U> producer4(queue1.get_producer(), *input4);
	consume_task<record, 2048U> consumer1(queue1.get_consumer(), *output1);
	consume_task<record, 2048U> consumer2(queue1.get_consumer(), *output2);
	consume_task<record, 2048U> consumer3(queue1.get_consumer(), *output3);
	consume_task<record, 2048U> consumer4(queue1.get_consumer(), *output4);
	producer1.run_copy();
	consumer1.run_copy();
	producer2.run_copy();
	consumer2.run_copy();
	producer3.run_copy();
	consumer3.run_copy();
	producer4.run_copy();
	consumer4.run_copy();
    }
    std::unique_ptr<std::array<record, 8192U>> actual_output(new std::array<record, 8192U>());
    {
	auto actual_iter = actual_output->begin();
	for (auto out_iter = output1->cbegin(); actual_iter != actual_output->end() && out_iter != output1->cend(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output2->cbegin(); actual_iter != actual_output->end() && out_iter != output2->cend(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output3->cbegin(); actual_iter != actual_output->end() && out_iter != output3->cend(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output4->cbegin(); actual_iter != actual_output->end() && out_iter != output4->cend(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
    }
    std::sort(actual_output->begin(), actual_output->end(), [] (const record& left, const record& right) -> bool
    {
	return left.first < right.first;
    });
    std::unique_ptr<std::array<record, 8192U>> expected_input(new std::array<record, 8192U>());
    {
	auto expected_iter = expected_input->begin();
	for (auto in_iter = input1->cbegin(); expected_iter != expected_input->end() && in_iter != input1->cend(); ++expected_iter, ++in_iter)
	{
	    *expected_iter = *in_iter;
	}
	for (auto in_iter = input2->cbegin(); expected_iter != expected_input->end() && in_iter != input2->cend(); ++expected_iter, ++in_iter)
	{
	    *expected_iter = *in_iter;
	}
	for (auto in_iter = input3->cbegin(); expected_iter != expected_input->end() && in_iter != input3->cend(); ++expected_iter, ++in_iter)
	{
	    *expected_iter = *in_iter;
	}
	for (auto in_iter = input4->cbegin(); expected_iter != expected_input->end() && in_iter != input4->cend(); ++expected_iter, ++in_iter)
	{
	    *expected_iter = *in_iter;
	}
    }
    auto expected_iter = expected_input->cbegin();
    auto actual_iter = actual_output->cbegin();
    for (; expected_iter != expected_input->cend() && actual_iter != actual_output->cend(); ++expected_iter, ++actual_iter)
    {
	EXPECT_EQ(*expected_iter, *actual_iter) << "Mismatching record consumed " <<
		"- expected {" << expected_iter->first << ", " << expected_iter->second << ", " << expected_iter->third << "} " <<
		"- actual {" << actual_iter->first << ", " << actual_iter->second << ", " << actual_iter->third << "}";
    }
}

TEST(mpmc_ring_queue_test, overflow)
{
    typedef tco::mpmc_ring_queue<uint32_t> uint_queue;

    uint_queue queue1(4, 2);
    uint_queue::producer& producer1 = queue1.get_producer();
    uint_queue::consumer& consumer1 = queue1.get_consumer();
    uint32_t expected = 0;
    uint32_t actual = 0;

    for (expected = 0; expected < std::numeric_limits<decltype(expected)>::max(); ++expected)
    {
	producer1.try_enqueue_copy(expected);
	consumer1.try_dequeue_copy(actual);
    }

    // Next enqueue will cause head index to overflow
    ASSERT_NE(producer1.try_enqueue_copy(1), uint_queue::producer::result::queue_full) << "Queue should not be full";
    ASSERT_NE(producer1.try_enqueue_copy(2), uint_queue::producer::result::queue_full) << "Queue should not be full";
    ASSERT_NE(producer1.try_enqueue_copy(3), uint_queue::producer::result::queue_full) << "Queue should not be full";
    ASSERT_NE(producer1.try_enqueue_copy(4), uint_queue::producer::result::queue_full) << "Queue should not be full";
    EXPECT_EQ(producer1.try_enqueue_copy(5), uint_queue::producer::result::queue_full) << "Queue should be full";

    // Next dequeue will cause tail index to overflow
    ASSERT_NE(consumer1.try_dequeue_copy(actual), uint_queue::consumer::result::queue_empty) << "Queue should not be empty";
    ASSERT_NE(consumer1.try_dequeue_copy(actual), uint_queue::consumer::result::queue_empty) << "Queue should not be empty";
    ASSERT_NE(consumer1.try_dequeue_copy(actual), uint_queue::consumer::result::queue_empty) << "Queue should not be empty";
    ASSERT_NE(consumer1.try_dequeue_copy(actual), uint_queue::consumer::result::queue_empty) << "Queue should not be empty";
    EXPECT_EQ(consumer1.try_dequeue_copy(actual), uint_queue::consumer::result::queue_empty) << "Queue should be empty";
}
