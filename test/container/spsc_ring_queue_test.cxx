#include <turbo/container/spsc_ring_queue.hpp>
#include <turbo/container/spsc_ring_queue.hxx>
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <string>

namespace tc = turbo::container;

TEST(spsc_ring_queue_test, dequeue_basic)
{
    typedef tc::spsc_ring_queue<std::string> string_queue;

    string_queue queue1(8);
    string_queue::producer& producer1 = queue1.get_producer();
    string_queue::consumer& consumer1 = queue1.get_consumer();
    std::string expected1("foo");
    ASSERT_NE(producer1.try_enqueue_copy(expected1), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    std::string actual1;
    ASSERT_NE(consumer1.try_dequeue_copy(actual1), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    EXPECT_EQ(expected1, actual1) << "Value enqueued is not the same value dequeued";

    string_queue queue2(8);
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

TEST(spsc_ring_queue_test, dequeue_move)
{
    typedef tc::spsc_ring_queue<std::unique_ptr<std::string>> string_queue;

    string_queue queue1(8);
    string_queue::producer& producer1 = queue1.get_producer();
    string_queue::consumer& consumer1 = queue1.get_consumer();
    std::string expected1("foo");
    std::unique_ptr<std::string> copy1(new std::string(expected1));
    ASSERT_NE(producer1.try_enqueue_move(std::move(copy1)), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    std::unique_ptr<std::string> actual1;
    ASSERT_NE(consumer1.try_dequeue_move(actual1), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    EXPECT_EQ(expected1, *actual1) << "Value enqueued is not the same value dequeued";

    string_queue queue2(8);
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

TEST(spsc_ring_queue_test, loop_back)
{
    typedef tc::spsc_ring_queue<std::string> string_queue;

    string_queue queue1(2);
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

TEST(spsc_ring_queue_test, empty_queue)
{
    typedef tc::spsc_ring_queue<std::string> string_queue;

    string_queue queue1(8);
    string_queue::producer& producer1 = queue1.get_producer();
    string_queue::consumer& consumer1 = queue1.get_consumer();
    std::string expected1("foo");
    std::string actual1;
    ASSERT_EQ(consumer1.try_dequeue_copy(actual1), string_queue::consumer::result::queue_empty) << "Just initialised queue is not empty";
    ASSERT_NE(producer1.try_enqueue_copy(expected1), string_queue::producer::result::queue_full) << "Just initialised queue is full";
    ASSERT_NE(consumer1.try_dequeue_copy(actual1), string_queue::consumer::result::queue_empty) << "Just enqueued queue is empty";
    EXPECT_EQ(consumer1.try_dequeue_copy(actual1), string_queue::consumer::result::queue_empty) << "Queue should be empty";

    string_queue queue2(8);
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

TEST(spsc_ring_queue_test, full_queue)
{
    typedef tc::spsc_ring_queue<std::string> string_queue;

    string_queue queue1(2);
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

TEST(spsc_ring_queue_test, overflow)
{
    typedef tc::spsc_ring_queue<uint32_t> uint_queue;

    uint_queue queue1(4);
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

TEST(spsc_ring_queue_test, excess_producer)
{
    typedef tc::spsc_ring_queue<uint32_t> uint_queue;
    uint_queue queue1(4);
    queue1.get_producer();
    ASSERT_THROW(queue1.get_producer(), std::system_error);
}

TEST(spsc_ring_queue_test, excess_consumer)
{
    typedef tc::spsc_ring_queue<uint32_t> uint_queue;
    uint_queue queue1(4);
    queue1.get_consumer();
    ASSERT_THROW(queue1.get_consumer(), std::system_error);
}
