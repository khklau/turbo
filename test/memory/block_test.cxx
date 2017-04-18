#include <turbo/memory/block.hpp>
#include <cstdint>
#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <gtest/gtest.h>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hxx>
#include <turbo/container/mpmc_ring_queue.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>

namespace tar = turbo::algorithm::recovery;
namespace tco = turbo::container;
namespace tme = turbo::memory;

TEST(block_test, invalid_construction)
{
    ASSERT_THROW(tme::block(0U, 3U, alignof(std::uint64_t)), tme::invalid_size_error) << "block constructed with 0 value size did not throw";
    ASSERT_THROW(tme::block(sizeof(std::uint8_t), 2U, alignof(std::uint64_t)), tme::invalid_alignment_error) << "block constructed with alignment exceeding the total size did not throw";
    tme::block block1(sizeof(std::uint8_t), 0U, alignof(std::uint8_t));
    ASSERT_TRUE(block1.is_empty()) << "block constructed with 0 capacity is not empty";
}

TEST(block_test, invalid_allocate)
{
    tme::block block1(sizeof(std::uint8_t), 0U, alignof(std::uint8_t));
    EXPECT_EQ(nullptr, block1.allocate()) << "empty block still allocated";
}

TEST(block_test, invalid_free)
{
    tme::block block1(sizeof(std::uint64_t), 3U, alignof(std::uint64_t));
    ASSERT_NO_THROW(block1.free(nullptr)) << "exception thrown for freeing a nullptr argument; should be a no-op";
    std::uint64_t stack1 = 54U;
    ASSERT_THROW(block1.free(&stack1), tme::invalid_pointer_error) << "invalid_pointer_error not thrown for freeing a pointer to stack argument";
    static std::uint16_t constant1 = 72U;
    std::uint16_t* constantptr1 = &constant1;
    ASSERT_THROW(block1.free(&constantptr1), tme::invalid_pointer_error) << "invalid_pointer_error not thrown for freeing a pointer to constant argument";
    tme::block block2(sizeof(std::uint16_t), 16U, alignof(std::uint16_t));
    std::uint16_t* heap1 = static_cast<std::uint16_t*>(block2.allocate());
    ASSERT_THROW(block1.free(heap1), tme::invalid_pointer_error) << "invalid_pointer_error not thrown for freeing address from a different block";
    ASSERT_NO_THROW(block2.free(heap1));
    tme::block block3(sizeof(std::uint64_t), 3U, alignof(std::uint64_t));
    std::uint64_t* heap2 = static_cast<std::uint64_t*>(block3.allocate());
    ASSERT_THROW(block1.free(heap2), tme::invalid_pointer_error) << "invalid_pointer_error not thrown for freeing address from a different block";
    ASSERT_NO_THROW(block3.free(heap2));
    std::uint8_t* heap3 = static_cast<std::uint8_t*>(block1.allocate());
    std::uint8_t* heap4 = heap3 + 3;
    ASSERT_THROW(block1.free(heap4), tme::invalid_pointer_error) << "invalid_pointer_error not thrown for freeing an address that refers to the middle of value";
    ASSERT_NO_THROW(block1.free(heap3));
    std::uint64_t* heap5 = static_cast<std::uint64_t*>(block1.allocate());
    std::uint64_t* heap6 = heap5 + (block1.get_usable_size() / sizeof(std::uint64_t)) + 2;
    ASSERT_THROW(block1.free(heap6), tme::invalid_pointer_error) << "invalid_pointer_error not thrown for freeing an address outside the block's address range";
    ASSERT_NO_THROW(block1.free(heap5));
    tme::block block4(sizeof(std::uint64_t), 0U, alignof(std::uint64_t));
    ASSERT_NO_THROW(block4.free(heap1)) << "exception thrown for freeing from an empty block; should be a no-op";
}

TEST(block_test, allocate_basic)
{
    tme::block block1(sizeof(std::uint64_t), 3U, alignof(std::uint64_t));
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";

    tme::block block2(sizeof(std::uint64_t), 0U, alignof(std::uint64_t));
    EXPECT_EQ(nullptr, block2.allocate()) << "Empty block still allocated";
}

TEST(block_test, free_basic)
{
    tme::block block1(sizeof(std::uint16_t), 3U, alignof(std::uint64_t));
    void* result1 = block1.allocate();
    void* result2 = block1.allocate();
    void* result3 = block1.allocate();
    ASSERT_NO_THROW(block1.free(result1)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result2)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result3)) << "Free failed";
}

TEST(block_test, recycle_basic)
{
    tme::block block1(sizeof(std::uint32_t), 3U, alignof(std::uint64_t));
    void* result1 = block1.allocate();
    void* result2 = block1.allocate();
    EXPECT_NE(nullptr, result1) << "Allocation failed";
    EXPECT_NE(nullptr, result2) << "Allocation failed";
    ASSERT_NO_THROW(block1.free(result1)) << "Free failed";
    void* result3 = block1.allocate();
    void* result4 = block1.allocate();
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
    ASSERT_NO_THROW(block1.free(result2)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result3)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result4)) << "Free failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
}

TEST(block_test, copy_construction)
{
    tme::block block1(sizeof(std::uint64_t), 4U, alignof(std::uint64_t));
    std::uint64_t* ptr1a = static_cast<std::uint64_t*>(block1.allocate());
    EXPECT_NE(nullptr, ptr1a) << "Allocation failed";
    *ptr1a = 67U;
    std::uint64_t* ptr1b = static_cast<std::uint64_t*>(block1.allocate());
    EXPECT_NE(nullptr, ptr1b) << "Allocation failed";
    *ptr1b = 927U;
    std::uint64_t* ptr1c = static_cast<std::uint64_t*>(block1.allocate());
    EXPECT_NE(nullptr, ptr1c) << "Allocation failed";
    *ptr1c = 2871U;
    tme::block block2(block1);
    EXPECT_TRUE(block1 == block2) << "Copy constructed block is not equal to the original";
    EXPECT_NE(nullptr, block2.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block2.allocate()) << "Allocation on full block succeeded";
}

TEST(block_test, copy_assignment)
{
    tme::block block1(sizeof(std::uint64_t), 4U, alignof(std::uint64_t));
    std::uint64_t* ptr1a = static_cast<std::uint64_t*>(block1.allocate());
    EXPECT_NE(nullptr, ptr1a) << "Allocation failed";
    *ptr1a = 67U;
    std::uint64_t* ptr1b = static_cast<std::uint64_t*>(block1.allocate());
    EXPECT_NE(nullptr, ptr1b) << "Allocation failed";
    *ptr1b = 927U;
    std::uint64_t* ptr1c = static_cast<std::uint64_t*>(block1.allocate());
    EXPECT_NE(nullptr, ptr1c) << "Allocation failed";
    *ptr1c = 2871U;
    tme::block block2(sizeof(std::uint64_t), 4U, alignof(std::uint64_t));
    std::uint64_t* ptr2a = static_cast<std::uint64_t*>(block2.allocate());
    EXPECT_NE(nullptr, ptr2a) << "Allocation failed";
    *ptr2a = 704U;
    std::uint64_t* ptr2b = static_cast<std::uint64_t*>(block2.allocate());
    EXPECT_NE(nullptr, ptr2b) << "Allocation failed";
    *ptr2b = 30872U;
    block2 = block1;
    EXPECT_TRUE(block1 == block2) << "Copy constructed block is not equal to the original";
    EXPECT_NE(nullptr, block2.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block2.allocate()) << "Allocation on full block succeeded";
    EXPECT_EQ(*ptr1a, *ptr2a) << "Block contents was not changed after assignment";
    EXPECT_EQ(*ptr1b, *ptr2b) << "Block contents was not changed after assignment";
}

struct record
{
    record();
    record(uint16_t f, uint32_t s, uint64_t t);
    record(const record& other);
    bool operator==(const record& other) const;
    uint16_t first;
    uint32_t second;
    uint64_t third;
};

record::record()
    :
	first(0U),
	second(0U),
	third(0U)
{ }

record::record(uint16_t f, uint32_t s, uint64_t t)
    :
	first(f),
	second(s),
	third(t)
{ }

record::record(const record& other)
    :
	first(other.first),
	second(other.second),
	third(other.third)
{ }

bool record::operator==(const record& other) const
{
    return first == other.first && second == other.second && third == other.third;
}

TEST(block_test, recycle_struct)
{
    tme::block block1(sizeof(record), 3U, alignof(record));
    void* result1 = block1.allocate();
    void* result2 = block1.allocate();
    EXPECT_NE(nullptr, result1) << "Allocation failed";
    EXPECT_NE(nullptr, result2) << "Allocation failed";
    ASSERT_NO_THROW(block1.free(result1)) << "Free failed";
    void* result3 = block1.allocate();
    void* result4 = block1.allocate();
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
    ASSERT_NO_THROW(block1.free(result2)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result3)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result4)) << "Free failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
}

typedef std::array<char, 64> char_64_array;

TEST(block_test, recycle_array)
{
    tme::block block1(sizeof(char_64_array), 3U, alignof(char_64_array));
    void* result1 = block1.allocate();
    void* result2 = block1.allocate();
    EXPECT_NE(nullptr, result1) << "Allocation failed";
    EXPECT_NE(nullptr, result2) << "Allocation failed";
    ASSERT_NO_THROW(block1.free(result1)) << "Free failed";
    void* result3 = block1.allocate();
    void* result4 = block1.allocate();
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
    ASSERT_NO_THROW(block1.free(result2)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result3)) << "Free failed";
    ASSERT_NO_THROW(block1.free(result4)) << "Free failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_NE(nullptr, block1.allocate()) << "Allocation failed";
    EXPECT_EQ(nullptr, block1.allocate()) << "Full block still allocated";
}

template <class value_t, std::size_t limit>
class produce_task
{
public:
    typedef tco::mpmc_ring_queue<value_t*> queue;
    produce_task(typename queue::producer& producer, tme::block& block, const std::array<value_t, limit>& input);
    ~produce_task() noexcept;
    void run();
    void produce();
private:
    typename queue::producer& producer_;
    tme::block& block_;
    const std::array<value_t, limit>& input_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
produce_task<value_t, limit>::produce_task(typename queue::producer& producer, tme::block& block, const std::array<value_t, limit>& input)
    :
	producer_(producer),
	block_(block),
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
void produce_task<value_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&produce_task::produce, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void produce_task<value_t, limit>::produce()
{
    for (auto iter = input_.cbegin(); iter != input_.cend();)
    {
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    value_t* result = static_cast<value_t*>(block_.allocate());
	    if (result != nullptr)
	    {
		new (result) value_t(*iter);
		tar::retry_with_random_backoff([&] () -> tar::try_state
		{
		    if (producer_.try_enqueue_copy(result) == queue::producer::result::success)
		    {
			++iter;
			return tar::try_state::done;
		    }
		    else
		    {
			return tar::try_state::retry;
		    }
		});
		return tar::try_state::done;
	    }
	    else
	    {
		return tar::try_state::retry;
	    }
	});
    }
}

template <class value_t, std::size_t limit>
class consume_task
{
public:
    typedef tco::mpmc_ring_queue<value_t*> queue;
    consume_task(typename queue::consumer& consumer, tme::block& block, std::array<value_t, limit>& output);
    ~consume_task() noexcept;
    void run();
    void consume();
private:
    typename queue::consumer& consumer_;
    tme::block& block_;
    std::array<value_t, limit>& output_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
consume_task<value_t, limit>::consume_task(typename queue::consumer& consumer, tme::block& block, std::array<value_t, limit>& output)
    :
	consumer_(consumer),
	block_(block),
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
void consume_task<value_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&consume_task::consume, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void consume_task<value_t, limit>::consume()
{
    for (auto iter = output_.begin(); iter != output_.end();)
    {
	value_t* tmp;
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    if (consumer_.try_dequeue_copy(tmp) == queue::consumer::result::success)
	    {
		*iter = *tmp;
		tmp->~value_t();
		block_.free(tmp);
		++iter;
		return tar::try_state::done;
	    }
	    else
	    {
		return tar::try_state::retry;
	    }
	});
    }
}

TEST(block_test, message_pass_struct)
{
    typedef tco::mpmc_ring_queue<record*> record_queue;
    record_queue queue1(64U, 4U);
    tme::block block1(sizeof(record), 8192U, alignof(record));
    std::unique_ptr<std::array<record, 8192U>> expected_output(new std::array<record, 8192U>());
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
	record tmp{base1, base1 * 3U, base1 * 9UL};
	(*input1)[counter1] = tmp;
	(*expected_output)[counter1 + 0U] = tmp;
    }
    for (uint64_t counter2 = 0U; counter2 < input2->max_size(); ++counter2)
    {
	uint16_t base2 = 3U + (counter2 * 5U) + 10240U;
	record tmp{base2, base2 * 3U, base2 * 9UL};
	(*input2)[counter2] = tmp;
	(*expected_output)[counter2 + 2048U] = tmp;
    }
    for (uint64_t counter3 = 0U; counter3 < input3->max_size(); ++counter3)
    {
	uint16_t base3 = 3U + (counter3 * 5U) + 20480;
	record tmp{base3, base3 * 3U, base3 * 9UL};
	(*input3)[counter3] = tmp;
	(*expected_output)[counter3 + 4096U] = tmp;
    }
    for (uint64_t counter4 = 0U; counter4 < input4->max_size(); ++counter4)
    {
	uint16_t base4 = 3U + (counter4 * 5U) + 30720U;
	record tmp{base4, base4 * 3U, base4 * 9UL};
	(*input4)[counter4] = tmp;
	(*expected_output)[counter4 + 6144U] = tmp;
    }
    {
	produce_task<record, 2048U> producer1(queue1.get_producer(), block1, *input1);
	produce_task<record, 2048U> producer2(queue1.get_producer(), block1, *input2);
	produce_task<record, 2048U> producer3(queue1.get_producer(), block1, *input3);
	produce_task<record, 2048U> producer4(queue1.get_producer(), block1, *input4);
	consume_task<record, 2048U> consumer1(queue1.get_consumer(), block1, *output1);
	consume_task<record, 2048U> consumer2(queue1.get_consumer(), block1, *output2);
	consume_task<record, 2048U> consumer3(queue1.get_consumer(), block1, *output3);
	consume_task<record, 2048U> consumer4(queue1.get_consumer(), block1, *output4);
	producer1.run();
	consumer2.run();
	producer2.run();
	consumer3.run();
	producer3.run();
	consumer4.run();
	producer4.run();
	consumer1.run();
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
    std::stable_sort(actual_output->begin(), actual_output->end(), [] (const record& left, const record& right) -> bool
    {
	return left.first < right.first;
    });
    auto expected_iter = expected_output->cbegin();
    auto actual_iter = actual_output->cbegin();
    for (; expected_iter != expected_output->cend() && actual_iter != actual_output->cend(); ++expected_iter, ++actual_iter)
    {
	EXPECT_EQ(*expected_iter, *actual_iter) << "Mismatching record consumed " <<
		"- expected {" << expected_iter->first << ", " << expected_iter->second << ", " << expected_iter->third << "} " <<
		"- actual {" << actual_iter->first << ", " << actual_iter->second << ", " << actual_iter->third << "}";
    }
}

TEST(block_test, message_pass_string)
{
    typedef tco::mpmc_ring_queue<std::string*> string_queue;
    string_queue queue1(8U, 4U);
    tme::block block1(sizeof(std::string), 8192U, alignof(std::string));
    std::unique_ptr<std::array<std::string, 8192U>> expected_output(new std::array<std::string, 8192U>());
    std::unique_ptr<std::array<std::string, 2048U>> input1(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> input2(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> input3(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> input4(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> output1(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> output2(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> output3(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> output4(new std::array<std::string, 2048U>());
    for (uint64_t counter1 = 0U; counter1 < input1->max_size(); ++counter1)
    {
	std::ostringstream ostream;
	ostream << "a";
	ostream << std::setw(8) << std::setfill('0');
	ostream << std::to_string(std::hash<uint64_t>()(counter1 + 0U));
	(*input1)[counter1] = ostream.str();
	(*expected_output)[counter1 + 0U] = ostream.str();
    }
    for (uint64_t counter2 = 0U; counter2 < input2->max_size(); ++counter2)
    {
	std::ostringstream ostream;
	ostream << "b";
	ostream << std::setw(8) << std::setfill('0');
	ostream << std::to_string(std::hash<uint64_t>()(counter2 + 2048U));
	(*input2)[counter2] = ostream.str();
	(*expected_output)[counter2 + 2048U] = ostream.str();
    }
    for (uint64_t counter3 = 0U; counter3 < input3->max_size(); ++counter3)
    {
	std::ostringstream ostream;
	ostream << "c";
	ostream << std::setw(8) << std::setfill('0');
	ostream << std::to_string(std::hash<uint64_t>()(counter3 + 4096U));
	(*input3)[counter3] = ostream.str();
	(*expected_output)[counter3 + 4096U] = ostream.str();
    }
    for (uint64_t counter4 = 0U; counter4 < input4->max_size(); ++counter4)
    {
	std::ostringstream ostream;
	ostream << "d";
	ostream << std::setw(8) << std::setfill('0');
	ostream << std::to_string(std::hash<uint64_t>()(counter4 + 6144U));
	(*input4)[counter4] = ostream.str();
	(*expected_output)[counter4 + 6144U] = ostream.str();
    }
    {
	produce_task<std::string, 2048U> producer1(queue1.get_producer(), block1, *input1);
	produce_task<std::string, 2048U> producer2(queue1.get_producer(), block1, *input2);
	produce_task<std::string, 2048U> producer3(queue1.get_producer(), block1, *input3);
	produce_task<std::string, 2048U> producer4(queue1.get_producer(), block1, *input4);
	consume_task<std::string, 2048U> consumer1(queue1.get_consumer(), block1, *output1);
	consume_task<std::string, 2048U> consumer2(queue1.get_consumer(), block1, *output2);
	consume_task<std::string, 2048U> consumer3(queue1.get_consumer(), block1, *output3);
	consume_task<std::string, 2048U> consumer4(queue1.get_consumer(), block1, *output4);
	producer4.run();
	consumer1.run();
	producer3.run();
	consumer2.run();
	producer2.run();
	consumer3.run();
	producer1.run();
	consumer4.run();
    }
    std::unique_ptr<std::array<std::string, 8192U>> actual_output(new std::array<std::string, 8192U>());
    {
	auto actual_iter = actual_output->begin();
	for (auto out_iter = output1->begin(); actual_iter != actual_output->end() && out_iter != output1->end(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output2->begin(); actual_iter != actual_output->end() && out_iter != output2->end(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output3->begin(); actual_iter != actual_output->end() && out_iter != output3->end(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
	for (auto out_iter = output4->begin(); actual_iter != actual_output->end() && out_iter != output4->end(); ++actual_iter, ++out_iter)
	{
	    *actual_iter = *out_iter;
	}
    }
    std::stable_sort(actual_output->begin(), actual_output->end(), [] (const std::string& left, const std::string& right) -> bool
    {
	return left < right;
    });
    auto expected_iter = expected_output->cbegin();
    auto actual_iter = actual_output->cbegin();
    for (; expected_iter != expected_output->cend() && actual_iter != actual_output->cend(); ++expected_iter, ++actual_iter)
    {
	EXPECT_EQ(*expected_iter, *actual_iter) << "Mismatching std::string consumed " <<
		"- expected '" << expected_iter->c_str() << "' " <<
		"- actual '" << actual_iter->c_str() << "'";
    }
}

void random_spin()
{
    std::random_device device;
    std::uint64_t limit = 0U;
    limit = device() % 128;
    for (std::uint64_t iter = 0U; iter < limit; ++iter) { };
}

typedef std::array<std::uint16_t, 8> oct_short;

template <std::size_t limit>
class sum_task
{
public:
    sum_task(tme::block& block, const std::array<oct_short, limit>& input, std::array<std::uint32_t, limit>& output);
    ~sum_task() noexcept;
    void run();
    void sum();
private:
    tme::block& block_;
    const std::array<oct_short, limit>& input_;
    std::array<std::uint32_t, limit>& output_;
    std::thread* thread_;
};

template <std::size_t limit>
sum_task<limit>::sum_task(tme::block& block, const std::array<oct_short, limit>& input, std::array<std::uint32_t, limit>& output)
    :
	block_(block),
	input_(input),
	output_(output),
	thread_(nullptr)
{ }

template <std::size_t limit>
sum_task<limit>::~sum_task() noexcept
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

template <std::size_t limit>
void sum_task<limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&sum_task::sum, this));
	thread_ = new std::thread(entry);
    }
}

template <std::size_t limit>
void sum_task<limit>::sum()
{
    for (auto iter = 0U; iter < limit; ++iter)
    {
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    oct_short* copy = static_cast<oct_short*>(block_.allocate());
	    if (copy != nullptr)
	    {
		std::copy(input_[iter].cbegin(), input_[iter].cend(), copy->begin());
		output_[iter] = 0U;
		std::for_each(copy->cbegin(), copy->cend(), [&] (const std::uint16_t value) -> void
		{
		    output_[iter] += value;
		});
		random_spin();
		block_.free(copy);
		return tar::try_state::done;
	    }
	    else
	    {
		return tar::try_state::retry;
	    }
	});
    }
}

TEST(block_test, parallel_use)
{
    tme::block pool1(sizeof(oct_short), 8192U, alignof(oct_short));
    std::unique_ptr<std::array<oct_short, 2048U>> input1(new std::array<oct_short, 2048U>());
    std::unique_ptr<std::array<oct_short, 2048U>> input2(new std::array<oct_short, 2048U>());
    std::unique_ptr<std::array<oct_short, 2048U>> input3(new std::array<oct_short, 2048U>());
    std::unique_ptr<std::array<oct_short, 2048U>> input4(new std::array<oct_short, 2048U>());
    std::unique_ptr<std::array<std::uint32_t, 2048U>> actual_output1(new std::array<std::uint32_t, 2048U>());
    std::unique_ptr<std::array<std::uint32_t, 2048U>> actual_output2(new std::array<std::uint32_t, 2048U>());
    std::unique_ptr<std::array<std::uint32_t, 2048U>> actual_output3(new std::array<std::uint32_t, 2048U>());
    std::unique_ptr<std::array<std::uint32_t, 2048U>> actual_output4(new std::array<std::uint32_t, 2048U>());
    std::unique_ptr<std::array<std::uint32_t, 2048U>> expected_output1(new std::array<std::uint32_t, 2048U>());
    std::unique_ptr<std::array<std::uint32_t, 2048U>> expected_output2(new std::array<std::uint32_t, 2048U>());
    std::unique_ptr<std::array<std::uint32_t, 2048U>> expected_output3(new std::array<std::uint32_t, 2048U>());
    std::unique_ptr<std::array<std::uint32_t, 2048U>> expected_output4(new std::array<std::uint32_t, 2048U>());
    std::fill_n(actual_output1->begin(), actual_output1->max_size(), 0U);
    std::fill_n(actual_output2->begin(), actual_output2->max_size(), 0U);
    std::fill_n(actual_output3->begin(), actual_output3->max_size(), 0U);
    std::fill_n(actual_output4->begin(), actual_output4->max_size(), 0U);
    for (std::uint16_t counter1 = 0U; counter1 < input1->max_size(); ++counter1)
    {
	(*input1)[counter1][0] = counter1 * 5U;
	(*input1)[counter1][1] = counter1 * 7U;
	(*input1)[counter1][2] = counter1 * 11U;
	(*input1)[counter1][3] = counter1 * 13U;
	(*input1)[counter1][4] = counter1 * 17U;
	(*input1)[counter1][5] = counter1 * 19U;
	(*input1)[counter1][6] = counter1 * 23U;
	(*input1)[counter1][7] = counter1 * 29U;
	(*expected_output1)[counter1] = 0U;
	std::for_each((*input1)[counter1].cbegin(), (*input1)[counter1].cend(), [&] (const std::uint16_t value) -> void
	{
	    (*expected_output1)[counter1] += value;
	});
    }
    for (std::uint16_t counter2 = 0U; counter2 < input2->max_size(); ++counter2)
    {
	(*input2)[counter2][0] = 3U + counter2 * 5U;
	(*input2)[counter2][1] = 3U + counter2 * 7U;
	(*input2)[counter2][2] = 3U + counter2 * 11U;
	(*input2)[counter2][3] = 3U + counter2 * 13U;
	(*input2)[counter2][4] = 3U + counter2 * 17U;
	(*input2)[counter2][5] = 3U + counter2 * 19U;
	(*input2)[counter2][6] = 3U + counter2 * 23U;
	(*input2)[counter2][7] = 3U + counter2 * 29U;
	(*expected_output2)[counter2] = 0U;
	std::for_each((*input2)[counter2].cbegin(), (*input2)[counter2].cend(), [&] (const std::uint16_t value) -> void
	{
	    (*expected_output2)[counter2] += value;
	});
    }
    for (std::uint16_t counter3 = 0U; counter3 < input3->max_size(); ++counter3)
    {
	(*input3)[counter3][0] = counter3 * 5U;
	(*input3)[counter3][1] = counter3 * 7U;
	(*input3)[counter3][2] = counter3 * 11U;
	(*input3)[counter3][3] = counter3 * 13U;
	(*input3)[counter3][4] = counter3 * 17U;
	(*input3)[counter3][5] = counter3 * 19U;
	(*input3)[counter3][6] = counter3 * 23U;
	(*input3)[counter3][7] = counter3 * 29U;
	(*expected_output3)[counter3] = 0U;
	std::for_each((*input3)[counter3].cbegin(), (*input3)[counter3].cend(), [&] (const std::uint16_t value) -> void
	{
	    (*expected_output3)[counter3] += value;
	});
    }
    for (std::uint16_t counter4 = 0U; counter4 < input4->max_size(); ++counter4)
    {
	(*input4)[counter4][0] = 3u + counter4 * 5U;
	(*input4)[counter4][1] = 3u + counter4 * 7U;
	(*input4)[counter4][2] = 3u + counter4 * 11U;
	(*input4)[counter4][3] = 3u + counter4 * 13U;
	(*input4)[counter4][4] = 3u + counter4 * 17U;
	(*input4)[counter4][5] = 3u + counter4 * 19U;
	(*input4)[counter4][6] = 3u + counter4 * 23U;
	(*input4)[counter4][7] = 3u + counter4 * 29U;
	(*expected_output4)[counter4] = 0U;
	std::for_each((*input4)[counter4].cbegin(), (*input4)[counter4].cend(), [&] (const std::uint16_t value) -> void
	{
	    (*expected_output4)[counter4] += value;
	});
    }
    {
	sum_task<2048U> sum1(pool1, *input1, *actual_output1);
	sum_task<2048U> sum2(pool1, *input2, *actual_output2);
	sum_task<2048U> sum3(pool1, *input3, *actual_output3);
	sum_task<2048U> sum4(pool1, *input4, *actual_output4);
	sum1.run();
	sum2.run();
	sum3.run();
	sum4.run();
    }
    auto expected_iter1 = expected_output1->cbegin();
    auto actual_iter1 = actual_output1->cbegin();
    for (; expected_iter1 != expected_output1->cend() && actual_iter1 != actual_output1->cend(); ++expected_iter1, ++actual_iter1)
    {
	EXPECT_EQ(*expected_iter1, *actual_iter1) << "Mismatching oct_short sum result " <<
		"- expected " << *expected_iter1 <<
		"- actual " << *actual_iter1;
    }
    auto expected_iter2 = expected_output2->cbegin();
    auto actual_iter2 = actual_output2->cbegin();
    for (; expected_iter2 != expected_output2->cend() && actual_iter2 != actual_output2->cend(); ++expected_iter2, ++actual_iter2)
    {
	EXPECT_EQ(*expected_iter2, *actual_iter2) << "Mismatching oct_short sum result " <<
		"- expected " << *expected_iter2 <<
		"- actual " << *actual_iter2;
    }
    auto expected_iter3 = expected_output3->cbegin();
    auto actual_iter3 = actual_output3->cbegin();
    for (; expected_iter3 != expected_output3->cend() && actual_iter3 != actual_output3->cend(); ++expected_iter3, ++actual_iter3)
    {
	EXPECT_EQ(*expected_iter3, *actual_iter3) << "Mismatching oct_short sum result " <<
		"- expected " << *expected_iter3 <<
		"- actual " << *actual_iter3;
    }
    auto expected_iter4 = expected_output4->cbegin();
    auto actual_iter4 = actual_output4->cbegin();
    for (; expected_iter4 != expected_output4->cend() && actual_iter4 != actual_output4->cend(); ++expected_iter4, ++actual_iter4)
    {
	EXPECT_EQ(*expected_iter4, *actual_iter4) << "Mismatching oct_short sum result " <<
		"- expected " << *expected_iter4 <<
		"- actual " << *actual_iter4;
    }
}
