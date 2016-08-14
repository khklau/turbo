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
#include <turbo/container/mpmc_ring_queue.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>

namespace tar = turbo::algorithm::recovery;
namespace tco = turbo::container;
namespace tme = turbo::memory;

TEST(block_test, invalid_construction)
{
    ASSERT_THROW(tme::block(0U, 3U, alignof(std::uint64_t)), tme::invalid_size_error);
    ASSERT_THROW(tme::block(sizeof(std::uint8_t), 2U, alignof(std::uint64_t)), tme::invalid_alignment_error);
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

TEST(block_test, messasge_pass_struct)
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
