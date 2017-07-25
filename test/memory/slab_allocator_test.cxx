#include <turbo/memory/slab_allocator.hpp>
#include <turbo/memory/slab_allocator.hxx>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hxx>

namespace turbo {
namespace memory {

class concurrent_sized_slab_tester
{
public:
    concurrent_sized_slab_tester(concurrent_sized_slab& a_slab)
	:
	    slab_(a_slab)
    { }
    inline std::size_t find_block_bucket(std::size_t allocation_size) const
    {
	return slab_.find_block_bucket(allocation_size);
    }
private:
    concurrent_sized_slab& slab_;
};

} // namespace memory
} // namespace turbo

namespace tar = turbo::algorithm::recovery;
namespace tco = turbo::container;
namespace tme = turbo::memory;

typedef std::array<std::uint16_t, 8> oct_short;

struct record
{
    record();
    record(uint16_t f, uint32_t s, uint64_t t);
    record(const record& other);
    record& operator=(const record& other);
    bool operator==(const record& other) const;
    bool operator<(const record& other) const;
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

record& record::operator=(const record& other)
{
    if (this != &other)
    {
	first = other.first;
	second = other.second;
	third = other.third;
    }
    return *this;
}

bool record::operator==(const record& other) const
{
    return first == other.first && second == other.second && third == other.third;
}

bool record::operator<(const record& other) const
{
    if (first != other.first)
    {
	return first < other.first;
    }
    else if (second != other.second)
    {
	return second < other.second;
    }
    else
    {
	return third < other.third;
    }
}

void random_spin()
{
    std::random_device device;
    std::uint64_t limit = 0U;
    limit = device() % 128;
    for (std::uint64_t iter = 0U; iter < limit; ++iter) { };
}

TEST(concurrent_sized_slab_test, find_block_bucket_basic)
{
    tme::concurrent_sized_slab slab1(16U, { {2U, 16U}, {8U, 16U}, {32U, 16U} });
    tme::concurrent_sized_slab_tester tester1(slab1);
    EXPECT_EQ(0U, tester1.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, tester1.find_block_bucket(2U)) << "Unexpected bucket with bucket size parameter of 2U";
    EXPECT_EQ(1U, tester1.find_block_bucket(3U)) << "Unexpected bucket with bucket size parameter of 3U";
    EXPECT_EQ(1U, tester1.find_block_bucket(4U)) << "Unexpected bucket with bucket size parameter of 3U";
    EXPECT_EQ(2U, tester1.find_block_bucket(5U)) << "Unexpected bucket with bucket size parameter of 8U";
    EXPECT_EQ(2U, tester1.find_block_bucket(8U)) << "Unexpected bucket with bucket size parameter of 8U";
    EXPECT_EQ(3U, tester1.find_block_bucket(9U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(3U, tester1.find_block_bucket(16U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(4U, tester1.find_block_bucket(17U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(4U, tester1.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";

    tme::concurrent_sized_slab slab2(16U, { {8U, 16U}, {32U, 16U}, {128U, 16U} });
    tme::concurrent_sized_slab_tester tester2(slab2);
    EXPECT_EQ(0U, tester2.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, tester2.find_block_bucket(8U)) << "Unexpected bucket with bucket size parameter of 8U";
    EXPECT_EQ(1U, tester2.find_block_bucket(9U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(1U, tester2.find_block_bucket(16U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(2U, tester2.find_block_bucket(17U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(2U, tester2.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(3U, tester2.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";
    EXPECT_EQ(3U, tester2.find_block_bucket(64U)) << "Unexpected bucket with bucket size parameter of 33U";
    EXPECT_EQ(4U, tester2.find_block_bucket(65U)) << "Unexpected bucket with bucket size parameter of 128U";
    EXPECT_EQ(4U, tester2.find_block_bucket(128U)) << "Unexpected bucket with bucket size parameter of 128U";

    tme::concurrent_sized_slab slab3(8U, { {32U, 16U}, {64U, 16U}, {128U, 16U}, {256U, 16U} });
    tme::concurrent_sized_slab_tester tester3(slab3);
    EXPECT_EQ(0U, tester3.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, tester3.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(1U, tester3.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";
    EXPECT_EQ(1U, tester3.find_block_bucket(64U)) << "Unexpected bucket with bucket size parameter of 64U";
    EXPECT_EQ(2U, tester3.find_block_bucket(65U)) << "Unexpected bucket with bucket size parameter of 65U";
    EXPECT_EQ(2U, tester3.find_block_bucket(128U)) << "Unexpected bucket with bucket size parameter of 128U";
}

TEST(concurrent_sized_slab_test, find_block_bucket_invalid)
{
    tme::concurrent_sized_slab slab1(16U, { {2U, 16U}, {8U, 16U}, {32U, 16U} });
    tme::concurrent_sized_slab_tester tester1(slab1);
    EXPECT_EQ(0U, tester1.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(5U, tester1.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";

    tme::concurrent_sized_slab slab2(16U, { {8U, 16U}, {32U, 16U}, {128U, 16U} });
    tme::concurrent_sized_slab_tester tester2(slab2);
    EXPECT_EQ(0U, tester2.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(5U, tester2.find_block_bucket(129U)) << "Unexpected bucket with bucket size parameter of 129U";

    tme::concurrent_sized_slab slab3(8U, { {32U, 16U}, {64U, 16U}, {128U, 16U}, {256U, 16U} });
    tme::concurrent_sized_slab_tester tester3(slab3);
    EXPECT_EQ(0U, tester3.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(4U, tester3.find_block_bucket(257U)) << "Unexpected bucket with bucket size parameter of 257U";
}

template <class value_t, std::size_t limit>
class concurrent_sized_slab_producer_task
{
public:
    typedef tco::mpmc_ring_queue<value_t*> queue;
    concurrent_sized_slab_producer_task(typename queue::producer& producer, tme::concurrent_sized_slab& slab, const std::array<value_t, limit>& input);
    ~concurrent_sized_slab_producer_task() noexcept;
    void run();
    void produce();
private:
    typename queue::producer& producer_;
    tme::concurrent_sized_slab& slab_;
    const std::array<value_t, limit>& input_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
concurrent_sized_slab_producer_task<value_t, limit>::concurrent_sized_slab_producer_task(typename queue::producer& producer, tme::concurrent_sized_slab& slab, const std::array<value_t, limit>& input)
    :
	producer_(producer),
	slab_(slab),
	input_(input),
	thread_(nullptr)
{ }

template <class value_t, std::size_t limit>
concurrent_sized_slab_producer_task<value_t, limit>::~concurrent_sized_slab_producer_task() noexcept
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
void concurrent_sized_slab_producer_task<value_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&concurrent_sized_slab_producer_task::produce, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void concurrent_sized_slab_producer_task<value_t, limit>::produce()
{
    for (auto input_iter = input_.cbegin(); input_iter != input_.cend();)
    {
	value_t* result = slab_.allocate<value_t>();
	if (result != nullptr)
	{
	    new (result) value_t(*input_iter);
	    tar::retry_with_random_backoff([&] () -> tar::try_state
	    {
		if (producer_.try_enqueue_copy(result) == queue::producer::result::success)
		{
		    ++input_iter;
		    return tar::try_state::done;
		}
		else
		{
		    return tar::try_state::retry;
		}
	    });
	}
	else
	{
	    std::cerr << "ERROR: slab allocation failed" << std::endl;
	}
    }
}

template <class value_t, std::size_t limit>
class concurrent_sized_slab_consumer_task
{
public:
    typedef tco::mpmc_ring_queue<value_t*> queue;
    concurrent_sized_slab_consumer_task(typename queue::consumer& consumer, tme::concurrent_sized_slab& slab, std::array<value_t, limit>& output);
    ~concurrent_sized_slab_consumer_task() noexcept;
    void run();
    void consume();
private:
    typename queue::consumer& consumer_;
    tme::concurrent_sized_slab& slab_;
    std::array<value_t, limit>& output_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
concurrent_sized_slab_consumer_task<value_t, limit>::concurrent_sized_slab_consumer_task(typename queue::consumer& consumer, tme::concurrent_sized_slab& slab, std::array<value_t, limit>& output)
    :
	consumer_(consumer),
	slab_(slab),
	output_(output),
	thread_(nullptr)
{ }

template <class value_t, std::size_t limit>
concurrent_sized_slab_consumer_task<value_t, limit>::~concurrent_sized_slab_consumer_task() noexcept
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
void concurrent_sized_slab_consumer_task<value_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&concurrent_sized_slab_consumer_task::consume, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void concurrent_sized_slab_consumer_task<value_t, limit>::consume()
{
    for (auto output_iter = output_.begin(); output_iter != output_.end();)
    {
	value_t* tmp;
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    if (consumer_.try_dequeue_copy(tmp) == queue::consumer::result::success)
	    {
		*output_iter = *tmp;
		random_spin();
		tmp->~value_t();
		slab_.deallocate<value_t>(tmp);
		++output_iter;
		return tar::try_state::done;
	    }
	    else
	    {
		return tar::try_state::retry;
	    }
	});
    }
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_message_pass_string)
{
    typedef tco::mpmc_ring_queue<std::string*> string_queue;
    string_queue queue1(8U, 4U);
    tme::concurrent_sized_slab slab1(2U, { {sizeof(std::string), 2U} });
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
	concurrent_sized_slab_producer_task<std::string, 2048U> producer1(queue1.get_producer(), slab1, *input1);
	concurrent_sized_slab_producer_task<std::string, 2048U> producer2(queue1.get_producer(), slab1, *input2);
	concurrent_sized_slab_producer_task<std::string, 2048U> producer3(queue1.get_producer(), slab1, *input3);
	concurrent_sized_slab_producer_task<std::string, 2048U> producer4(queue1.get_producer(), slab1, *input4);
	concurrent_sized_slab_consumer_task<std::string, 2048U> consumer1(queue1.get_consumer(), slab1, *output1);
	concurrent_sized_slab_consumer_task<std::string, 2048U> consumer2(queue1.get_consumer(), slab1, *output2);
	concurrent_sized_slab_consumer_task<std::string, 2048U> consumer3(queue1.get_consumer(), slab1, *output3);
	concurrent_sized_slab_consumer_task<std::string, 2048U> consumer4(queue1.get_consumer(), slab1, *output4);
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

TEST(concurrent_sized_slab_test, concurrent_sized_slab_message_pass_record)
{
    typedef tco::mpmc_ring_queue<record*> record_queue;
    record_queue queue1(8U, 4U);
    tme::concurrent_sized_slab slab1(2U, { {sizeof(record), 2U} });
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
	concurrent_sized_slab_producer_task<record, 2048U> producer1(queue1.get_producer(), slab1, *input1);
	concurrent_sized_slab_producer_task<record, 2048U> producer2(queue1.get_producer(), slab1, *input2);
	concurrent_sized_slab_producer_task<record, 2048U> producer3(queue1.get_producer(), slab1, *input3);
	concurrent_sized_slab_producer_task<record, 2048U> producer4(queue1.get_producer(), slab1, *input4);
	concurrent_sized_slab_consumer_task<record, 2048U> consumer1(queue1.get_consumer(), slab1, *output1);
	concurrent_sized_slab_consumer_task<record, 2048U> consumer2(queue1.get_consumer(), slab1, *output2);
	concurrent_sized_slab_consumer_task<record, 2048U> consumer3(queue1.get_consumer(), slab1, *output3);
	concurrent_sized_slab_consumer_task<record, 2048U> consumer4(queue1.get_consumer(), slab1, *output4);
	producer4.run();
	consumer1.run();
	producer3.run();
	consumer2.run();
	producer2.run();
	consumer3.run();
	producer1.run();
	consumer4.run();
    }
    std::unique_ptr<std::array<record, 8192U>> actual_output(new std::array<record, 8192U>());
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
    std::stable_sort(actual_output->begin(), actual_output->end(), [] (const record& left, const record& right) -> bool
    {
	return left < right;
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

template <class input_t, class output_t, std::size_t limit>
class concurrent_sized_slab_user_task
{
public:
    concurrent_sized_slab_user_task(tme::concurrent_sized_slab& slab, const std::array<input_t, limit>& input, std::array<output_t, limit>& output, const std::function<output_t (const input_t&)>& process);
    ~concurrent_sized_slab_user_task() noexcept;
    void run();
    void use();
private:
    tme::concurrent_sized_slab& slab_;
    const std::array<input_t, limit>& input_;
    std::array<output_t, limit>& output_;
    const std::function<output_t (const input_t&)>& process_;
    std::thread* thread_;
};

template <class input_t, class output_t, std::size_t limit>
concurrent_sized_slab_user_task<input_t, output_t, limit>::concurrent_sized_slab_user_task(
	tme::concurrent_sized_slab& slab,
	const std::array<input_t, limit>& input,
	std::array<output_t, limit>& output,
	const std::function<output_t (const input_t&)>& process)
    :
	slab_(slab),
	input_(input),
	output_(output),
	process_(process),
	thread_(nullptr)
{ }

template <class input_t, class output_t, std::size_t limit>
concurrent_sized_slab_user_task<input_t, output_t, limit>::~concurrent_sized_slab_user_task() noexcept
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

template <class input_t, class output_t, std::size_t limit>
void concurrent_sized_slab_user_task<input_t, output_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&concurrent_sized_slab_user_task::use, this));
	thread_ = new std::thread(entry);
    }
}

template <class input_t, class output_t, std::size_t limit>
void concurrent_sized_slab_user_task<input_t, output_t, limit>::use()
{
    for (auto iter = 0U; iter < limit; ++iter)
    {
	input_t* result = slab_.allocate<input_t>();
	if (result != nullptr)
	{
	    random_spin();
	    new (result) input_t(input_[iter]);
	    output_[iter] = process_(*result);
	    random_spin();
	    result->~input_t();
	    slab_.deallocate<input_t>(result);
	}
	else
	{
	    std::cerr << "ERROR: slab allocation failed" << std::endl;
	}
    }
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_parallel_use_string)
{
    tme::concurrent_sized_slab slab1(2U, { {sizeof(std::string), 2U} });
    std::unique_ptr<std::array<std::string, 2048U>> input1(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> input2(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> input3(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> input4(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> actual_output1(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> actual_output2(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> actual_output3(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> actual_output4(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> expected_output1(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> expected_output2(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> expected_output3(new std::array<std::string, 2048U>());
    std::unique_ptr<std::array<std::string, 2048U>> expected_output4(new std::array<std::string, 2048U>());
    for (uint64_t counter1 = 0U; counter1 < input1->max_size(); ++counter1)
    {
	std::ostringstream ostream;
	ostream << "a";
	ostream << std::setw(8) << std::setfill('0');
	ostream << std::to_string(std::hash<uint64_t>()(counter1 + 0U));
	ostream << "a";
	(*input1)[counter1] = ostream.str();
	std::copy((*input1)[counter1].crbegin(), (*input1)[counter1].crend(), (*expected_output1)[counter1].begin());
    }
    for (uint64_t counter2 = 0U; counter2 < input2->max_size(); ++counter2)
    {
	std::ostringstream ostream;
	ostream << "b";
	ostream << std::setw(8) << std::setfill('0');
	ostream << std::to_string(std::hash<uint64_t>()(counter2 + 2048U));
	ostream << "b";
	(*input2)[counter2] = ostream.str();
	std::copy((*input2)[counter2].crbegin(), (*input2)[counter2].crend(), (*expected_output2)[counter2].begin());
    }
    for (uint64_t counter3 = 0U; counter3 < input3->max_size(); ++counter3)
    {
	std::ostringstream ostream;
	ostream << "c";
	ostream << std::setw(8) << std::setfill('0');
	ostream << std::to_string(std::hash<uint64_t>()(counter3 + 4096U));
	ostream << "c";
	(*input3)[counter3] = ostream.str();
	std::copy((*input3)[counter3].crbegin(), (*input3)[counter3].crend(), (*expected_output3)[counter3].begin());
    }
    for (uint64_t counter4 = 0U; counter4 < input4->max_size(); ++counter4)
    {
	std::ostringstream ostream;
	ostream << "d";
	ostream << std::setw(8) << std::setfill('0');
	ostream << std::to_string(std::hash<uint64_t>()(counter4 + 6144U));
	ostream << "d";
	(*input4)[counter4] = ostream.str();
	std::copy((*input4)[counter4].crbegin(), (*input4)[counter4].crend(), (*expected_output4)[counter4].begin());
    }
    {
	auto process = [] (const std::string& input) -> std::string
	{
	    std::string output;
	    std::copy(input.crbegin(), input.crend(), output.begin());
	    return std::move(output);
	};
	concurrent_sized_slab_user_task<std::string, std::string, 2048U> task1(slab1, *input1, *actual_output1, process);
	concurrent_sized_slab_user_task<std::string, std::string, 2048U> task2(slab1, *input2, *actual_output2, process);
	concurrent_sized_slab_user_task<std::string, std::string, 2048U> task3(slab1, *input3, *actual_output3, process);
	concurrent_sized_slab_user_task<std::string, std::string, 2048U> task4(slab1, *input4, *actual_output4, process);
	task1.run();
	task2.run();
	task3.run();
	task4.run();
    }
    auto expected_iter1 = expected_output1->cbegin();
    auto actual_iter1 = actual_output1->cbegin();
    for (; expected_iter1 != expected_output1->cend() && actual_iter1 != actual_output1->cend(); ++expected_iter1, ++actual_iter1)
    {
	EXPECT_EQ(*expected_iter1, *actual_iter1) << "Mismatching std::string reverse result " <<
		"- expected " << *expected_iter1 <<
		"- actual " << *actual_iter1;
    }
    auto expected_iter2 = expected_output2->cbegin();
    auto actual_iter2 = actual_output2->cbegin();
    for (; expected_iter2 != expected_output2->cend() && actual_iter2 != actual_output2->cend(); ++expected_iter2, ++actual_iter2)
    {
	EXPECT_EQ(*expected_iter2, *actual_iter2) << "Mismatching std::string reverse result " <<
		"- expected " << *expected_iter2 <<
		"- actual " << *actual_iter2;
    }
    auto expected_iter3 = expected_output3->cbegin();
    auto actual_iter3 = actual_output3->cbegin();
    for (; expected_iter3 != expected_output3->cend() && actual_iter3 != actual_output3->cend(); ++expected_iter3, ++actual_iter3)
    {
	EXPECT_EQ(*expected_iter3, *actual_iter3) << "Mismatching std::string reverse result " <<
		"- expected " << *expected_iter3 <<
		"- actual " << *actual_iter3;
    }
    auto expected_iter4 = expected_output4->cbegin();
    auto actual_iter4 = actual_output4->cbegin();
    for (; expected_iter4 != expected_output4->cend() && actual_iter4 != actual_output4->cend(); ++expected_iter4, ++actual_iter4)
    {
	EXPECT_EQ(*expected_iter4, *actual_iter4) << "Mismatching std::string reverse result " <<
		"- expected " << *expected_iter4 <<
		"- actual " << *actual_iter4;
    }
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_parallel_use_octshort)
{
    tme::concurrent_sized_slab slab1(2U, { {sizeof(oct_short), 2U} });
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
	auto process = [] (const oct_short& input) -> std::uint32_t
	{
	    std::uint32_t output = 0U;
	    std::for_each(input.cbegin(), input.cend(), [&] (const std::uint16_t value) -> void
	    {
		output += value;
	    });
	    return output;
	};
	concurrent_sized_slab_user_task<oct_short, std::uint32_t, 2048U> task1(slab1, *input1, *actual_output1, process);
	concurrent_sized_slab_user_task<oct_short, std::uint32_t, 2048U> task2(slab1, *input2, *actual_output2, process);
	concurrent_sized_slab_user_task<oct_short, std::uint32_t, 2048U> task3(slab1, *input3, *actual_output3, process);
	concurrent_sized_slab_user_task<oct_short, std::uint32_t, 2048U> task4(slab1, *input4, *actual_output4, process);
	task1.run();
	task2.run();
	task3.run();
	task4.run();
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

TEST(concurrent_sized_slab_test, allocate_invalid)
{
    tme::concurrent_sized_slab slab1(2U, { {sizeof(std::string), 2U} });
    EXPECT_EQ(nullptr, slab1.allocate<std::string>(0U)) << "Allocating a quantity of 0 succeeded";
}

TEST(concurrent_sized_slab_test, malloc_invalid)
{
    tme::concurrent_sized_slab slab1(2U, { {sizeof(std::string), 2U} });
    EXPECT_EQ(nullptr, slab1.malloc(0U)) << "Allocating a quantity of 0 succeeded";
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_copy_construction)
{
    tme::concurrent_sized_slab slab1(2U, { {sizeof(std::string), 2U} });
    auto result1a = slab1.make_unique<std::string>("abc123");
    EXPECT_EQ(tme::make_result::success, result1a.first) << "Make unique slab string failed";
    EXPECT_EQ(std::string("abc123"), *result1a.second) << "String in memory slab didn't initialise";
    auto result1b = slab1.make_unique<std::string>("xyz789");
    EXPECT_EQ(tme::make_result::success, result1b.first) << "Make unique slab string failed";
    EXPECT_EQ(std::string("xyz789"), *result1b.second) << "String in memory slab didn't initialise";
    tme::concurrent_sized_slab slab2(slab1);
    EXPECT_TRUE(slab1 == slab2) << "Copy constructed slab is not equal to the original";

    tme::concurrent_sized_slab slab3(2U, { {sizeof(std::string), 2U} });
    auto result3a = slab3.make_unique<std::string>("abc321");
    EXPECT_EQ(tme::make_result::success, result3a.first) << "Make unique slab string failed";
    EXPECT_EQ(std::string("abc321"), *result3a.second) << "String in memory slab didn't initialise";
    auto result3b = slab3.make_unique<std::string>("xyz987");
    EXPECT_EQ(tme::make_result::success, result3b.first) << "Make unique slab string failed";
    EXPECT_EQ(std::string("xyz987"), *result3b.second) << "String in memory slab didn't initialise";
    auto result3c = slab3.make_unique<std::string>("lmn654");
    EXPECT_EQ(tme::make_result::success, result3c.first) << "Make unique slab string failed";
    EXPECT_EQ(std::string("lmn654"), *result3c.second) << "String in memory slab didn't initialise";
    auto result3d = slab3.make_unique<std::string>("!@#000");
    EXPECT_EQ(tme::make_result::success, result3d.first) << "Make unique slab string failed";
    EXPECT_EQ(std::string("!@#000"), *result3d.second) << "String in memory slab didn't initialise";
    auto result3e = slab3.make_unique<std::string>("   ");
    EXPECT_EQ(tme::make_result::success, result3e.first) << "Make unique slab string failed";
    EXPECT_EQ(std::string("   "), *result3e.second) << "String in memory slab didn't initialise";
    tme::concurrent_sized_slab slab4(slab3);
    EXPECT_TRUE(slab3 == slab4) << "Copy constructed slab is not equal to the original";
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_copy_assignment_same_length)
{
    tme::concurrent_sized_slab slab1(2U, { {sizeof(std::uint64_t), 2U}, {sizeof(std::uint16_t), 2U} });
    auto result1a = slab1.make_unique<std::uint64_t>(123U);
    EXPECT_EQ(tme::make_result::success, result1a.first) << "Make unique slab string failed";
    EXPECT_EQ(123U, *result1a.second) << "String in memory slab didn't initialise";
    auto result1b = slab1.make_unique<std::uint64_t>(789U);
    EXPECT_EQ(tme::make_result::success, result1b.first) << "Make unique slab string failed";
    EXPECT_EQ(789U, *result1b.second) << "String in memory slab didn't initialise";
    auto result1c = slab1.make_unique<std::uint16_t>(123U);
    EXPECT_EQ(tme::make_result::success, result1c.first) << "Make unique slab string failed";
    EXPECT_EQ(123U, *result1c.second) << "String in memory slab didn't initialise";
    auto result1d = slab1.make_unique<std::uint16_t>(789U);
    EXPECT_EQ(tme::make_result::success, result1d.first) << "Make unique slab string failed";
    EXPECT_EQ(789U, *result1d.second) << "String in memory slab didn't initialise";
    tme::concurrent_sized_slab slab2(2U, { {sizeof(std::uint64_t), 2U}, {sizeof(std::uint16_t), 2U} });
    auto result2a = slab2.make_unique<std::uint64_t>(456U);
    EXPECT_EQ(tme::make_result::success, result2a.first) << "Make unique slab string failed";
    EXPECT_EQ(456U, *result2a.second) << "String in memory slab didn't initialise";
    auto result2b = slab2.make_unique<std::uint64_t>(0U);
    EXPECT_EQ(tme::make_result::success, result2b.first) << "Make unique slab string failed";
    EXPECT_EQ(0U, *result2b.second) << "String in memory slab didn't initialise";
    auto result2c = slab2.make_unique<std::uint16_t>(456U);
    EXPECT_EQ(tme::make_result::success, result2c.first) << "Make unique slab string failed";
    EXPECT_EQ(456U, *result2c.second) << "String in memory slab didn't initialise";
    auto result2d = slab2.make_unique<std::uint16_t>(0U);
    EXPECT_EQ(tme::make_result::success, result2d.first) << "Make unique slab string failed";
    EXPECT_EQ(0U, *result2d.second) << "String in memory slab didn't initialise";
    slab2 = slab1;
    EXPECT_TRUE(slab1 == slab2) << "Copy assigned slab is not equal to the original";
    EXPECT_EQ(123U, *result2a.second) << "Memory blocks in slab weren't copied during assignment";
    EXPECT_EQ(789U, *result2b.second) << "String in memory slab didn't initialise";
    EXPECT_EQ(123U, *result2c.second) << "String in memory slab didn't initialise";
    EXPECT_EQ(789U, *result2d.second) << "String in memory slab didn't initialise";

    tme::concurrent_sized_slab slab3(2U, { {sizeof(std::uint64_t), 2U} });
    auto result3a = slab3.make_unique<std::uint64_t>(321U);
    EXPECT_EQ(tme::make_result::success, result3a.first) << "Make unique slab string failed";
    EXPECT_EQ(321U, *result3a.second) << "String in memory slab didn't initialise";
    auto result3b = slab3.make_unique<std::uint64_t>(987U);
    EXPECT_EQ(tme::make_result::success, result3b.first) << "Make unique slab string failed";
    EXPECT_EQ(987U, *result3b.second) << "String in memory slab didn't initialise";
    auto result3c = slab3.make_unique<std::uint64_t>(654U);
    EXPECT_EQ(tme::make_result::success, result3c.first) << "Make unique slab string failed";
    EXPECT_EQ(654U, *result3c.second) << "String in memory slab didn't initialise";
    auto result3d = slab3.make_unique<std::uint64_t>(999U);
    EXPECT_EQ(tme::make_result::success, result3d.first) << "Make unique slab string failed";
    EXPECT_EQ(999U, *result3d.second) << "String in memory slab didn't initialise";
    auto result3e = slab3.make_unique<std::uint64_t>(111U);
    EXPECT_EQ(tme::make_result::success, result3e.first) << "Make unique slab string failed";
    EXPECT_EQ(111U, *result3e.second) << "String in memory slab didn't initialise";
    tme::concurrent_sized_slab slab4(2U, { {sizeof(std::uint64_t), 2U} });
    auto result4a = slab4.make_unique<std::uint64_t>(1991U);
    EXPECT_EQ(tme::make_result::success, result4a.first) << "Make unique slab string failed";
    EXPECT_EQ(1991U, *result4a.second) << "String in memory slab didn't initialise";
    auto result4b = slab4.make_unique<std::uint64_t>(5665U);
    EXPECT_EQ(tme::make_result::success, result4b.first) << "Make unique slab string failed";
    EXPECT_EQ(5665U, *result4b.second) << "String in memory slab didn't initialise";
    auto result4c = slab4.make_unique<std::uint64_t>(2882U);
    EXPECT_EQ(tme::make_result::success, result4c.first) << "Make unique slab string failed";
    EXPECT_EQ(2882U, *result4c.second) << "String in memory slab didn't initialise";
    auto result4d = slab4.make_unique<std::uint64_t>(3773U);
    EXPECT_EQ(tme::make_result::success, result4d.first) << "Make unique slab string failed";
    EXPECT_EQ(3773U, *result4d.second) << "String in memory slab didn't initialise";
    slab4 = slab3;
    EXPECT_TRUE(slab3 == slab4) << "Copy assigned slab is not equal to the original";
    EXPECT_EQ(321U, *result4a.second) << "Memory blocks in slab weren't copied during assignment";
    EXPECT_EQ(987U, *result4b.second) << "String in memory slab didn't initialise";
    EXPECT_EQ(654U, *result4c.second) << "String in memory slab didn't initialise";
    EXPECT_EQ(999U, *result4d.second) << "String in memory slab didn't initialise";
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_copy_assignment_expand_length)
{
    tme::concurrent_sized_slab slab1(2U, { {sizeof(std::uint64_t), 2U}, {sizeof(std::uint16_t), 2U} });
    auto result1a = slab1.make_unique<std::uint64_t>(321U);
    EXPECT_EQ(tme::make_result::success, result1a.first) << "Make unique slab string failed";
    EXPECT_EQ(321U, *result1a.second) << "String in memory slab didn't initialise";
    auto result1b = slab1.make_unique<std::uint64_t>(987U);
    EXPECT_EQ(tme::make_result::success, result1b.first) << "Make unique slab string failed";
    EXPECT_EQ(987U, *result1b.second) << "String in memory slab didn't initialise";
    auto result1c = slab1.make_unique<std::uint64_t>(654U);
    EXPECT_EQ(tme::make_result::success, result1c.first) << "Make unique slab string failed";
    EXPECT_EQ(654U, *result1c.second) << "String in memory slab didn't initialise";
    auto result1d = slab1.make_unique<std::uint64_t>(999U);
    EXPECT_EQ(tme::make_result::success, result1d.first) << "Make unique slab string failed";
    EXPECT_EQ(999U, *result1d.second) << "String in memory slab didn't initialise";
    auto result1e = slab1.make_unique<std::uint64_t>(111U);
    EXPECT_EQ(tme::make_result::success, result1e.first) << "Make unique slab string failed";
    EXPECT_EQ(111U, *result1e.second) << "String in memory slab didn't initialise";
    auto result1f = slab1.make_unique<std::uint16_t>(321U);
    EXPECT_EQ(tme::make_result::success, result1f.first) << "Make unique slab string failed";
    EXPECT_EQ(321U, *result1f.second) << "String in memory slab didn't initialise";
    auto result1g = slab1.make_unique<std::uint16_t>(987U);
    EXPECT_EQ(tme::make_result::success, result1g.first) << "Make unique slab string failed";
    EXPECT_EQ(987U, *result1g.second) << "String in memory slab didn't initialise";
    auto result1h = slab1.make_unique<std::uint16_t>(654U);
    EXPECT_EQ(tme::make_result::success, result1h.first) << "Make unique slab string failed";
    EXPECT_EQ(654U, *result1h.second) << "String in memory slab didn't initialise";
    auto result1i = slab1.make_unique<std::uint16_t>(999U);
    EXPECT_EQ(tme::make_result::success, result1i.first) << "Make unique slab string failed";
    EXPECT_EQ(999U, *result1i.second) << "String in memory slab didn't initialise";
    auto result1j = slab1.make_unique<std::uint16_t>(111U);
    EXPECT_EQ(tme::make_result::success, result1j.first) << "Make unique slab string failed";
    EXPECT_EQ(111U, *result1j.second) << "String in memory slab didn't initialise";
    tme::concurrent_sized_slab slab2(2U, { {sizeof(std::uint64_t), 2U}, {sizeof(std::uint16_t), 2U} });
    auto result2a = slab2.make_unique<std::uint64_t>(1991U);
    EXPECT_EQ(tme::make_result::success, result2a.first) << "Make unique slab string failed";
    EXPECT_EQ(1991U, *result2a.second) << "String in memory slab didn't initialise";
    auto result2b = slab2.make_unique<std::uint64_t>(5665U);
    EXPECT_EQ(tme::make_result::success, result2b.first) << "Make unique slab string failed";
    EXPECT_EQ(5665U, *result2b.second) << "String in memory slab didn't initialise";
    auto result2c = slab2.make_unique<std::uint16_t>(1991U);
    EXPECT_EQ(tme::make_result::success, result2c.first) << "Make unique slab string failed";
    EXPECT_EQ(1991U, *result2c.second) << "String in memory slab didn't initialise";
    auto result2d = slab2.make_unique<std::uint16_t>(5665U);
    EXPECT_EQ(tme::make_result::success, result2d.first) << "Make unique slab string failed";
    EXPECT_EQ(5665U, *result2d.second) << "String in memory slab didn't initialise";
    slab2 = slab1;
    EXPECT_TRUE(slab1 == slab2) << "Copy assigned slab is not equal to the original";
    EXPECT_EQ(321U, *result2a.second) << "Memory blocks in slab weren't copied during assignment";
    EXPECT_EQ(987U, *result2b.second) << "String in memory slab didn't initialise";
    EXPECT_EQ(321U, *result2c.second) << "String in memory slab didn't initialise";
    EXPECT_EQ(987U, *result2d.second) << "String in memory slab didn't initialise";
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_copy_assignment_strink_length)
{
    tme::concurrent_sized_slab slab1(2U, { {sizeof(std::uint64_t), 2U}, {sizeof(std::uint16_t), 2U} });
    auto result1a = slab1.make_unique<std::uint64_t>(1991U);
    EXPECT_EQ(tme::make_result::success, result1a.first) << "Make unique slab string failed";
    EXPECT_EQ(1991U, *result1a.second) << "String in memory slab didn't initialise";
    auto result1b = slab1.make_unique<std::uint64_t>(5665U);
    EXPECT_EQ(tme::make_result::success, result1b.first) << "Make unique slab string failed";
    EXPECT_EQ(5665U, *result1b.second) << "String in memory slab didn't initialise";
    auto result1c = slab1.make_unique<std::uint16_t>(1991U);
    EXPECT_EQ(tme::make_result::success, result1c.first) << "Make unique slab string failed";
    EXPECT_EQ(1991U, *result1c.second) << "String in memory slab didn't initialise";
    auto result1d = slab1.make_unique<std::uint16_t>(5665U);
    EXPECT_EQ(tme::make_result::success, result1d.first) << "Make unique slab string failed";
    EXPECT_EQ(5665U, *result1d.second) << "String in memory slab didn't initialise";
    tme::concurrent_sized_slab slab2(2U, { {sizeof(std::uint64_t), 2U}, {sizeof(std::uint16_t), 2U} });
    auto result2a = slab2.make_unique<std::uint64_t>(321U);
    EXPECT_EQ(tme::make_result::success, result2a.first) << "Make unique slab string failed";
    EXPECT_EQ(321U, *result2a.second) << "String in memory slab didn't initialise";
    auto result2b = slab2.make_unique<std::uint64_t>(987U);
    EXPECT_EQ(tme::make_result::success, result2b.first) << "Make unique slab string failed";
    EXPECT_EQ(987U, *result2b.second) << "String in memory slab didn't initialise";
    auto result2c = slab2.make_unique<std::uint64_t>(654U);
    EXPECT_EQ(tme::make_result::success, result2c.first) << "Make unique slab string failed";
    EXPECT_EQ(654U, *result2c.second) << "String in memory slab didn't initialise";
    auto result2d = slab2.make_unique<std::uint64_t>(999U);
    EXPECT_EQ(tme::make_result::success, result2d.first) << "Make unique slab string failed";
    EXPECT_EQ(999U, *result2d.second) << "String in memory slab didn't initialise";
    auto result2e = slab2.make_unique<std::uint64_t>(111U);
    EXPECT_EQ(tme::make_result::success, result2e.first) << "Make unique slab string failed";
    EXPECT_EQ(111U, *result2e.second) << "String in memory slab didn't initialise";
    auto result2f = slab2.make_unique<std::uint16_t>(321U);
    EXPECT_EQ(tme::make_result::success, result2f.first) << "Make unique slab string failed";
    EXPECT_EQ(321U, *result2f.second) << "String in memory slab didn't initialise";
    auto result2g = slab2.make_unique<std::uint16_t>(987U);
    EXPECT_EQ(tme::make_result::success, result2g.first) << "Make unique slab string failed";
    EXPECT_EQ(987U, *result2g.second) << "String in memory slab didn't initialise";
    auto result2h = slab2.make_unique<std::uint16_t>(654U);
    EXPECT_EQ(tme::make_result::success, result2h.first) << "Make unique slab string failed";
    EXPECT_EQ(654U, *result2h.second) << "String in memory slab didn't initialise";
    auto result2i = slab2.make_unique<std::uint16_t>(999U);
    EXPECT_EQ(tme::make_result::success, result2i.first) << "Make unique slab string failed";
    EXPECT_EQ(999U, *result2i.second) << "String in memory slab didn't initialise";
    auto result2j = slab2.make_unique<std::uint16_t>(111U);
    EXPECT_EQ(tme::make_result::success, result2j.first) << "Make unique slab string failed";
    EXPECT_EQ(111U, *result2j.second) << "String in memory slab didn't initialise";
    slab2 = slab1;
    EXPECT_TRUE(slab1 == slab2) << "Copy assigned slab is not equal to the original";
    EXPECT_EQ(1991U, *result2a.second) << "Memory blocks in slab weren't copied during assignment";
    EXPECT_EQ(5665U, *result2b.second) << "String in memory slab didn't initialise";
    EXPECT_EQ(1991U, *result2f.second) << "String in memory slab didn't initialise";
    EXPECT_EQ(5665U, *result2g.second) << "String in memory slab didn't initialise";
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_make_unique_basic)
{
    tme::concurrent_sized_slab slab1(3U, { {sizeof(std::string), 3U} });
    {
	auto result1 = slab1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique slab string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "String in memory slab didn't initialise";
	auto result2 = slab1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique slab string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "String in memory slab didn't initialise";
	auto result3 = slab1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique slab string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "String in memory slab didn't initialise";
    }
    tme::concurrent_sized_slab slab2(2U, { {2U, 32U}, {32U, 32U} });
    {
	auto result1 = slab1.make_unique<std::uint64_t>(123U);
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique slab string failed";
	EXPECT_EQ(123U, *result1.second) << "Integer in initially empty memory slab didn't initialise";
	auto result2 = slab1.make_unique<std::uint64_t>(456U);
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique slab string failed";
	EXPECT_EQ(456U, *result2.second) << "Integer in initially empty memory slab didn't initialise";
	auto result3 = slab1.make_unique<std::uint64_t>(789U);
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique slab string failed";
	EXPECT_EQ(789U, *result3.second) << "Integer in initially empty memory slab didn't initialise";
    }
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_make_unique_invalid)
{
    tme::concurrent_sized_slab slab1(8U, { {sizeof(std::uint8_t), 8U} });
    {
	auto result1 = slab1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::slab_full, result1.first) << "Making a unique_ptr for a value that is larger than the configured slab succeeded";
	EXPECT_EQ(nullptr, result1.second.get()) << "The unique_ptr returned when the requested value is larger than the configured slab is not nullptr";
    }
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_make_shared_basic)
{
    tme::concurrent_sized_slab slab1(3U, { {sizeof(std::string), 3U} });
    {
	auto result1 = slab1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared slab string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared slab string didn't initialise";
	auto result2 = slab1.make_shared<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared slab string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared slab string didn't initialise";
	auto result3 = slab1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared slab string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared slab string didn't initialise";
    }
    tme::concurrent_sized_slab slab2(2U, { {2U, 32U}, {32U, 32U} });
    {
	auto result1 = slab1.make_shared<std::uint64_t>(123U);
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared slab string failed";
	EXPECT_EQ(123U, *result1.second) << "Integer in initially empty memory slab didn't initialise";
	auto result2 = slab1.make_shared<std::uint64_t>(456U);
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared slab string failed";
	EXPECT_EQ(456U, *result2.second) << "Integer in initially empty memory slab didn't initialise";
	auto result3 = slab1.make_shared<std::uint64_t>(789U);
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared slab string failed";
	EXPECT_EQ(789U, *result3.second) << "Integer in initially empty memory slab didn't initialise";
    }
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_make_shared_invalid)
{
    tme::concurrent_sized_slab slab1(8U, { {sizeof(std::uint8_t), 8U} });
    {
	auto result1 = slab1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::slab_full, result1.first) << "Making a shared_ptr for a value that is larger than the configured slab succeeded";
	EXPECT_EQ(nullptr, result1.second.get()) << "The shared_ptr returned when the requested value is larger than the configured slab is not nullptr";
    }
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_make_mixed_basic)
{
    tme::concurrent_sized_slab slab1(4U, { {sizeof(std::string), 4U} });
    {
	auto result1 = slab1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique slab string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique slab string didn't initialise";
	auto result2 = slab1.make_shared<std::string>("!@#");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared slab string failed";
	EXPECT_EQ(std::string("!@#"), *result2.second) << "Shared slab string didn't initialise";
	auto result3 = slab1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique slab string failed";
	EXPECT_EQ(std::string("xyz789"), *result3.second) << "Unique slab string didn't initialise";
	auto result4 = slab1.make_shared<std::string>("$%^");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make shared slab string failed";
	EXPECT_EQ(std::string("$%^"), *result4.second) << "Shared slab string didn't initialise";
    }
}

TEST(concurrent_sized_slab_test, concurrent_sized_slab_make_mixed_invalid)
{
    tme::concurrent_sized_slab slab1(8U, { {sizeof(std::uint8_t), 8U} });
    {
	auto result1 = slab1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::slab_full, result1.first) << "Making a unique_ptr for a value that is larger than the configured slab succeeded";
	EXPECT_EQ(nullptr, result1.second.get()) << "The unique_ptr returned when the requested value is larger than the configured slab is not nullptr";
	auto result2 = slab1.make_shared<record>(4U, 5U, 6U);
	EXPECT_EQ(tme::make_result::slab_full, result2.first) << "Making a shared_ptr for a value that is larger than the configured slab succeeded";
	EXPECT_EQ(nullptr, result2.second.get()) << "The shared_ptr returned when the requested value is larger than the configured slab is not nullptr";
    }
}

TEST(concurrent_sized_slab_test, calibrate_positive)
{
    std::vector<tme::block_config> input1{ {64U, 4U}, {32U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected1{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual1(tme::calibrate(2U, input1));
    EXPECT_TRUE(!actual1.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected1.cbegin(), expected1.cend(), actual1.cbegin())) << "Incorrect calibration for config requiring just a reorder: "
	    << "expected [ "
	    << "{" << expected1[0].block_size << ", " << expected1[0].initial_capacity << "}, "
	    << "{" << expected1[1].block_size << ", " << expected1[1].initial_capacity << "}, "
	    << "{" << expected1[2].block_size << ", " << expected1[2].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual1[0].block_size << ", " << actual1[0].initial_capacity << "}, "
	    << "{" << actual1[1].block_size << ", " << actual1[1].initial_capacity << "}, "
	    << "{" << actual1[2].block_size << ", " << actual1[2].initial_capacity << "} ]";

    std::vector<tme::block_config> input2{ {64U, 4U}, {24U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected2{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual2(tme::calibrate(2U, input2));
    EXPECT_TRUE(!actual2.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected2.cbegin(), expected2.cend(), actual2.cbegin())) << "Incorrect calibration for config requiring a bucket size adjustment: "
	    << "expected [ "
	    << "{" << expected2[0].block_size << ", " << expected2[0].initial_capacity << "}, "
	    << "{" << expected2[1].block_size << ", " << expected2[1].initial_capacity << "}, "
	    << "{" << expected2[2].block_size << ", " << expected2[2].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual2[0].block_size << ", " << actual2[0].initial_capacity << "}, "
	    << "{" << actual2[1].block_size << ", " << actual2[1].initial_capacity << "}, "
	    << "{" << actual2[2].block_size << ", " << actual2[2].initial_capacity << "} ]";

    std::vector<tme::block_config> input3{ {32U, 4U}, {24U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected3{ {16U, 16U}, {32U, 12U} };
    std::vector<tme::block_config> actual3(tme::calibrate(2U, input3));
    EXPECT_TRUE(!actual3.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected3.cbegin(), expected3.cend(), actual3.cbegin())) << "Incorrect calibration for config requiring input bucket merging: "
	    << "expected [ "
	    << "{" << expected3[0].block_size << ", " << expected3[0].initial_capacity << "}, "
	    << "{" << expected3[1].block_size << ", " << expected3[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual3[0].block_size << ", " << actual3[0].initial_capacity << "}, "
	    << "{" << actual3[1].block_size << ", " << actual3[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input4{ {16U, 16U}, {64U, 4U} };
    std::vector<tme::block_config> expected4{ {16U, 16U}, {32U, 0U, 2U}, {64U, 4U} };
    std::vector<tme::block_config> actual4(tme::calibrate(2U, input4));
    EXPECT_TRUE(!actual4.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected4.cbegin(), expected4.cend(), actual4.cbegin())) << "Incorrect calibration for config requiring a generated empty bucket: "
	    << "expected [ "
	    << "{" << expected4[0].block_size << ", " << expected4[0].initial_capacity << "}, "
	    << "{" << expected4[1].block_size << ", " << expected4[1].initial_capacity << "}, "
	    << "{" << expected4[2].block_size << ", " << expected4[2].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual4[0].block_size << ", " << actual4[0].initial_capacity << "}, "
	    << "{" << actual4[1].block_size << ", " << actual4[1].initial_capacity << "}, "
	    << "{" << actual4[2].block_size << ", " << actual4[2].initial_capacity << "} ]";

    std::vector<tme::block_config> input5{ {25U, 16U}, {5U, 4U} };
    std::vector<tme::block_config> expected5{ {8U, 4U}, {16U, 0U, 2U}, {32U, 16U} };
    std::vector<tme::block_config> actual5(tme::calibrate(2U, input5));
    EXPECT_TRUE(!actual5.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected5.cbegin(), expected5.cend(), actual5.cbegin())) << "Incorrect calibration for config requiring a generated empty bucket and reordering: "
	    << "expected [ "
	    << "{" << expected5[0].block_size << ", " << expected5[0].initial_capacity << "}, "
	    << "{" << expected5[1].block_size << ", " << expected5[1].initial_capacity << "}, "
	    << "{" << expected5[2].block_size << ", " << expected5[2].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual5[0].block_size << ", " << actual5[0].initial_capacity << "}, "
	    << "{" << actual5[1].block_size << ", " << actual5[1].initial_capacity << "}, "
	    << "{" << actual5[2].block_size << ", " << actual5[2].initial_capacity << "} ]";
}

TEST(concurrent_sized_slab_test, calibrate_repeating)
{
    std::vector<tme::block_config> input1{ {64U, 4U}, {32U, 8U}, {64U, 16U} };
    std::vector<tme::block_config> expected1{ {32U, 8U}, {64U, 20U} };
    std::vector<tme::block_config> actual1(tme::calibrate(2U, input1));
    EXPECT_TRUE(!actual1.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected1.cbegin(), expected1.cend(), actual1.cbegin())) << "Incorrect calibration for config requiring just a reorder: "
	    << "expected [ "
	    << "{" << expected1[0].block_size << ", " << expected1[0].initial_capacity << "}, "
	    << "{" << expected1[1].block_size << ", " << expected1[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual1[0].block_size << ", " << actual1[0].initial_capacity << "}, "
	    << "{" << actual1[1].block_size << ", " << actual1[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input2{ {32U, 4U}, {32U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected2{ {16U, 16U}, {32U, 12U} };
    std::vector<tme::block_config> actual2(tme::calibrate(2U, input2));
    EXPECT_TRUE(!actual2.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected2.cbegin(), expected2.cend(), actual2.cbegin())) << "Incorrect calibration for config requiring just a reorder: "
	    << "expected [ "
	    << "{" << expected2[0].block_size << ", " << expected2[0].initial_capacity << "}, "
	    << "{" << expected2[1].block_size << ", " << expected2[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual2[0].block_size << ", " << actual2[0].initial_capacity << "}, "
	    << "{" << actual2[1].block_size << ", " << actual2[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input3{ {8U, 4U}, {24U, 8U}, {24U, 16U} };
    std::vector<tme::block_config> expected3{ {8U, 4U}, {16U, 0U, 2U}, {32U, 24U} };
    std::vector<tme::block_config> actual3(tme::calibrate(2U, input3));
    EXPECT_TRUE(!actual3.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected3.cbegin(), expected3.cend(), actual3.cbegin())) << "Incorrect calibration for config requiring a bucket size adjustment: "
	    << "expected [ "
	    << "{" << expected3[0].block_size << ", " << expected3[0].initial_capacity << "}, "
	    << "{" << expected3[1].block_size << ", " << expected3[1].initial_capacity << "}, "
	    << "{" << expected3[2].block_size << ", " << expected3[2].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual3[0].block_size << ", " << actual3[0].initial_capacity << "}, "
	    << "{" << actual3[1].block_size << ", " << actual3[1].initial_capacity << "}, "
	    << "{" << actual3[2].block_size << ", " << actual3[2].initial_capacity << "} ]";

    std::vector<tme::block_config> input4{ {24U, 4U}, {16U, 8U}, {24U, 16U} };
    std::vector<tme::block_config> expected4{ {16U, 8U}, {32U, 20U} };
    std::vector<tme::block_config> actual4(tme::calibrate(2U, input4));
    EXPECT_TRUE(!actual4.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected4.cbegin(), expected4.cend(), actual4.cbegin())) << "Incorrect calibration for config requiring a bucket size adjustment: "
	    << "expected [ "
	    << "{" << expected4[0].block_size << ", " << expected4[0].initial_capacity << "}, "
	    << "{" << expected4[1].block_size << ", " << expected4[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual4[0].block_size << ", " << actual4[0].initial_capacity << "}, "
	    << "{" << actual4[1].block_size << ", " << actual4[1].initial_capacity << "} ]";
}

TEST(concurrent_sized_slab_test, calibrate_negative)
{
    std::vector<tme::block_config> input1;
    std::vector<tme::block_config> actual1(tme::calibrate(2U, input1));
    EXPECT_TRUE(actual1.empty()) << "Empty output from non-empty input";

    std::vector<tme::block_config> input2{ {64U, 4U}, {24U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected2{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual2(tme::calibrate(2U, input2));
    EXPECT_TRUE(!actual2.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected2.cbegin(), expected2.cend(), actual2.cbegin())) << "Incorrect reordering: "
	    << "expected [ "
	    << "{" << expected2[0].block_size << ", " << expected2[0].initial_capacity << "}, "
	    << "{" << expected2[1].block_size << ", " << expected2[1].initial_capacity << "}, "
	    << "{" << expected2[2].block_size << ", " << expected2[2].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual2[0].block_size << ", " << actual2[0].initial_capacity << "}, "
	    << "{" << actual2[1].block_size << ", " << actual2[1].initial_capacity << "}, "
	    << "{" << actual2[2].block_size << ", " << actual2[2].initial_capacity << "} ]";

    std::vector<tme::block_config> input3{ {32U, 4U}, {24U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected3{ {16U, 16U}, {32U, 12U} };
    std::vector<tme::block_config> actual3(tme::calibrate(2U, input3));
    EXPECT_TRUE(!actual3.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected3.cbegin(), expected3.cend(), actual3.cbegin())) << "Incorrect reordering: "
	    << "expected [ "
	    << "{" << expected3[0].block_size << ", " << expected3[0].initial_capacity << "}, "
	    << "{" << expected3[1].block_size << ", " << expected3[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual3[0].block_size << ", " << actual3[0].initial_capacity << "}, "
	    << "{" << actual3[1].block_size << ", " << actual3[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input4{ {16U, 16U}, {64U, 4U} };
    std::vector<tme::block_config> expected4{ {16U, 16U}, {32U, 0U, 2U}, {64U, 4U} };
    std::vector<tme::block_config> actual4(tme::calibrate(2U, input4));
    EXPECT_TRUE(!actual4.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected4.cbegin(), expected4.cend(), actual4.cbegin())) << "Incorrect reordering: "
	    << "expected [ "
	    << "{" << expected4[0].block_size << ", " << expected4[0].initial_capacity << "}, "
	    << "{" << expected4[1].block_size << ", " << expected4[1].initial_capacity << "}, "
	    << "{" << expected4[2].block_size << ", " << expected4[2].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual4[0].block_size << ", " << actual4[0].initial_capacity << "}, "
	    << "{" << actual4[1].block_size << ", " << actual4[1].initial_capacity << "}, "
	    << "{" << actual4[2].block_size << ", " << actual4[2].initial_capacity << "} ]";
}
