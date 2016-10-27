#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <turbo/algorithm/recovery.hpp>

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

TEST(pool_test, invalid_iterator)
{
    tme::block_list list1(sizeof(std::int64_t), 4U);
    auto iter1 = list1.end();
    ASSERT_THROW(*iter1, tme::block_list::invalid_dereference) << "Deferencing invalid iterator did not throw";
    ASSERT_THROW(iter1->get_usable_size(), tme::block_list::invalid_dereference) << "Deferencing invalid iterator did not throw";
    ASSERT_THROW(iter1->get_base_address(), tme::block_list::invalid_dereference) << "Deferencing invalid iterator did not throw";
    auto iter1a = ++iter1;
    EXPECT_FALSE(iter1a.is_valid()) << "An invalid iterator became valid after incrementing";
    auto iter1b = iter1++;
    EXPECT_FALSE(iter1b.is_valid()) << "An invalid iterator became valid after incrementing";

    auto iter2 = list1.begin();
    ++iter2;
    ASSERT_THROW(*iter2, tme::block_list::invalid_dereference) << "Deferencing invalid iterator did not throw";
    ASSERT_THROW(iter2->get_usable_size(), tme::block_list::invalid_dereference) << "Deferencing invalid iterator did not throw";
    ASSERT_THROW(iter2->get_base_address(), tme::block_list::invalid_dereference) << "Deferencing invalid iterator did not throw";
    auto iter2a = ++iter2;
    EXPECT_FALSE(iter2a.is_valid()) << "An invalid iterator became valid after incrementing";
    auto iter2b = iter2++;
    EXPECT_FALSE(iter2b.is_valid()) << "An invalid iterator became valid after incrementing";
}

TEST(pool_test, invalid_append)
{
    tme::block_list list1(sizeof(std::int64_t), 4U);
    auto iter1 = list1.end();
    auto node1 = list1.create_node(8U);
    ASSERT_THROW(iter1.try_append(std::move(node1)), tme::block_list::invalid_dereference) << "Appending to invalid iterator succeeded";
}

TEST(pool_test, use_first_node)
{
    tme::block_list list1(sizeof(std::int64_t), 4U);
    auto iter1 = list1.begin();
    EXPECT_TRUE(iter1.is_valid()) << "Iterator pointing to first block is not valid";
    EXPECT_EQ(list1.begin(), iter1) << "Two iterators to the first block are not equivalent";
    EXPECT_NE(list1.end(), iter1) << "End iterator and iterator pointing to first block are equivalent";
    tme::block& block1 = *iter1;
    EXPECT_TRUE(4U <= block1.get_capacity()) << "Capacity of first block in block list is less than requested";
    EXPECT_TRUE(block1.get_base_address() != nullptr) << "Base address of first block in block list is nulltpr";
    EXPECT_TRUE(4U <= iter1->get_capacity()) << "Capacity of first block in block list is less than requested";
    EXPECT_TRUE(iter1->get_base_address() != nullptr) << "Base address of first block in block list is nulltpr";
    ++iter1;
    EXPECT_FALSE(iter1.is_valid()) << "Iterator pointing past last block is valid";
    EXPECT_NE(list1.begin(), iter1) << "Two iterators pointing to different parts of the block list are equivalent";
    EXPECT_EQ(list1.end(), iter1) << "End iterator and iterator pointing past last block are not equivalent";
}

TEST(pool_test, sequential_append)
{
    tme::block_list list1(sizeof(std::int64_t), 16U);
    auto iter1 = list1.begin();
    EXPECT_TRUE(16U <= iter1->get_capacity()) << "Capacity of first block in block list is less than requested";
    EXPECT_EQ(tme::block_list::append_result::success, iter1.try_append(std::move(list1.create_node(8U)))) << "Append failed";
    EXPECT_EQ(tme::block_list::append_result::beaten, iter1.try_append(std::move(list1.create_node(16U)))) << "Appending to middle of list succeeded";
    ++iter1;
    EXPECT_TRUE(8U <= iter1->get_capacity()) << "Capacity of first block in block list is less than requested";
    EXPECT_EQ(tme::block_list::append_result::success, iter1.try_append(std::move(list1.create_node(4U)))) << "Append failed";
    EXPECT_EQ(tme::block_list::append_result::beaten, iter1.try_append(std::move(list1.create_node(16U)))) << "Appending to middle of list succeeded";
    ++iter1;
    EXPECT_TRUE(4U <= iter1->get_capacity()) << "Capacity of first block in block list is less than requested";
}

template <class value_t, std::size_t limit>
class list_producer_task
{
public:
    typedef tco::mpmc_ring_queue<value_t*> queue;
    list_producer_task(typename queue::producer& producer, tme::block_list& list, const std::array<value_t, limit>& input);
    ~list_producer_task() noexcept;
    void run();
    void produce();
private:
    typename queue::producer& producer_;
    tme::block_list& list_;
    const std::array<value_t, limit>& input_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
list_producer_task<value_t, limit>::list_producer_task(typename queue::producer& producer, tme::block_list& list, const std::array<value_t, limit>& input)
    :
	producer_(producer),
	list_(list),
	input_(input),
	thread_(nullptr)
{ }

template <class value_t, std::size_t limit>
list_producer_task<value_t, limit>::~list_producer_task() noexcept
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
void list_producer_task<value_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&list_producer_task::produce, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void list_producer_task<value_t, limit>::produce()
{
    for (auto input_iter = input_.cbegin(); input_iter != input_.cend();)
    {
	auto block_iter = list_.begin();
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    value_t* result = static_cast<value_t*>(block_iter->allocate());
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
		return tar::try_state::done;
	    }
	    else
	    {
		if (block_iter.is_last())
		{
		    auto node = list_.create_node(block_iter->get_capacity() * 2);
		    block_iter.try_append(std::move(node));
		}
		++block_iter;
		return tar::try_state::retry;
	    }
	});
    }
}

template <class value_t, std::size_t limit>
class list_consumer_task
{
public:
    typedef tco::mpmc_ring_queue<value_t*> queue;
    list_consumer_task(typename queue::consumer& consumer, tme::block_list& list, std::array<value_t, limit>& output);
    ~list_consumer_task() noexcept;
    void run();
    void consume();
private:
    typename queue::consumer& consumer_;
    tme::block_list& list_;
    std::array<value_t, limit>& output_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
list_consumer_task<value_t, limit>::list_consumer_task(typename queue::consumer& consumer, tme::block_list& list, std::array<value_t, limit>& output)
    :
	consumer_(consumer),
	list_(list),
	output_(output),
	thread_(nullptr)
{ }

template <class value_t, std::size_t limit>
list_consumer_task<value_t, limit>::~list_consumer_task() noexcept
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
void list_consumer_task<value_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&list_consumer_task::consume, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void list_consumer_task<value_t, limit>::consume()
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
		auto block_iter = list_.begin();
		tar::retry_with_random_backoff([&] () -> tar::try_state
		{
		    if (block_iter->in_range(tmp))
		    {
			block_iter->free(tmp);
			return tar::try_state::done;
		    }
		    else if (block_iter.is_last())
		    {
			// leak!
			return tar::try_state::done;
		    }
		    else
		    {
			++block_iter;
			return tar::try_state::retry;
		    }
		});
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

TEST(pool_test, list_message_pass_string)
{
    typedef tco::mpmc_ring_queue<std::string*> string_queue;
    string_queue queue1(8U, 4U);
    tme::block_list block1(sizeof(std::string), 64U);
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
	list_producer_task<std::string, 2048U> producer1(queue1.get_producer(), block1, *input1);
	list_producer_task<std::string, 2048U> producer2(queue1.get_producer(), block1, *input2);
	list_producer_task<std::string, 2048U> producer3(queue1.get_producer(), block1, *input3);
	list_producer_task<std::string, 2048U> producer4(queue1.get_producer(), block1, *input4);
	list_consumer_task<std::string, 2048U> consumer1(queue1.get_consumer(), block1, *output1);
	list_consumer_task<std::string, 2048U> consumer2(queue1.get_consumer(), block1, *output2);
	list_consumer_task<std::string, 2048U> consumer3(queue1.get_consumer(), block1, *output3);
	list_consumer_task<std::string, 2048U> consumer4(queue1.get_consumer(), block1, *output4);
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

TEST(pool_test, list_message_pass_record)
{
    typedef tco::mpmc_ring_queue<record*> record_queue;
    record_queue queue1(8U, 4U);
    tme::block_list block1(sizeof(record), 64U);
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
	list_producer_task<record, 2048U> producer1(queue1.get_producer(), block1, *input1);
	list_producer_task<record, 2048U> producer2(queue1.get_producer(), block1, *input2);
	list_producer_task<record, 2048U> producer3(queue1.get_producer(), block1, *input3);
	list_producer_task<record, 2048U> producer4(queue1.get_producer(), block1, *input4);
	list_consumer_task<record, 2048U> consumer1(queue1.get_consumer(), block1, *output1);
	list_consumer_task<record, 2048U> consumer2(queue1.get_consumer(), block1, *output2);
	list_consumer_task<record, 2048U> consumer3(queue1.get_consumer(), block1, *output3);
	list_consumer_task<record, 2048U> consumer4(queue1.get_consumer(), block1, *output4);
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
class list_user_task
{
public:
    list_user_task(tme::block_list& list, const std::array<input_t, limit>& input, std::array<output_t, limit>& output, const std::function<output_t (const input_t&)>& process);
    ~list_user_task() noexcept;
    void run();
    void use();
private:
    tme::block_list& list_;
    const std::array<input_t, limit>& input_;
    std::array<output_t, limit>& output_;
    const std::function<output_t (const input_t&)>& process_;
    std::thread* thread_;
};

template <class input_t, class output_t, std::size_t limit>
list_user_task<input_t, output_t, limit>::list_user_task(
	tme::block_list& list,
	const std::array<input_t, limit>& input,
	std::array<output_t, limit>& output,
	const std::function<output_t (const input_t&)>& process)
    :
	list_(list),
	input_(input),
	output_(output),
	process_(process),
	thread_(nullptr)
{ }

template <class input_t, class output_t, std::size_t limit>
list_user_task<input_t, output_t, limit>::~list_user_task() noexcept
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
void list_user_task<input_t, output_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&list_user_task::use, this));
	thread_ = new std::thread(entry);
    }
}

template <class input_t, class output_t, std::size_t limit>
void list_user_task<input_t, output_t, limit>::use()
{
    for (auto iter = 0U; iter < limit; ++iter)
    {
	auto block_iter = list_.begin();
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    input_t* result = static_cast<input_t*>(block_iter->allocate());
	    if (result != nullptr)
	    {
		random_spin();
		new (result) input_t(input_[iter]);
		output_[iter] = process_(*result);
		random_spin();
		result->~input_t();
		block_iter->free(result);
		return tar::try_state::done;
	    }
	    else
	    {
		if (block_iter.is_last())
		{
		    auto node = list_.create_node(block_iter->get_capacity() * 2);
		    block_iter.try_append(std::move(node));
		}
		++block_iter;
		return tar::try_state::retry;
	    }
	});
    }
}

TEST(pool_test, list_parallel_use_string)
{
    tme::block_list list1(sizeof(std::string), 2U);
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
	list_user_task<std::string, std::string, 2048U> task1(list1, *input1, *actual_output1, process);
	list_user_task<std::string, std::string, 2048U> task2(list1, *input2, *actual_output2, process);
	list_user_task<std::string, std::string, 2048U> task3(list1, *input3, *actual_output3, process);
	list_user_task<std::string, std::string, 2048U> task4(list1, *input4, *actual_output4, process);
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

TEST(pool_test, list_parallel_use_octshort)
{
    tme::block_list list1(sizeof(oct_short), 2U);
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
	list_user_task<oct_short, std::uint32_t, 2048U> task1(list1, *input1, *actual_output1, process);
	list_user_task<oct_short, std::uint32_t, 2048U> task2(list1, *input2, *actual_output2, process);
	list_user_task<oct_short, std::uint32_t, 2048U> task3(list1, *input3, *actual_output3, process);
	list_user_task<oct_short, std::uint32_t, 2048U> task4(list1, *input4, *actual_output4, process);
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

TEST(pool_test, find_block_bucket_basic)
{
    tme::pool pool1(16U, { {2U, 16U}, {8U, 16U}, {32U, 16U} }, 4U);
    EXPECT_EQ(0U, pool1.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, pool1.find_block_bucket(2U)) << "Unexpected bucket with bucket size parameter of 2U";
    EXPECT_EQ(1U, pool1.find_block_bucket(3U)) << "Unexpected bucket with bucket size parameter of 3U";
    EXPECT_EQ(1U, pool1.find_block_bucket(8U)) << "Unexpected bucket with bucket size parameter of 8U";
    EXPECT_EQ(2U, pool1.find_block_bucket(9U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(2U, pool1.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";

    tme::pool pool2(16U, { {8U, 16U}, {32U, 16U}, {128U, 16U} }, 4U);
    EXPECT_EQ(0U, pool2.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, pool2.find_block_bucket(8U)) << "Unexpected bucket with bucket size parameter of 8U";
    EXPECT_EQ(1U, pool2.find_block_bucket(9U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(1U, pool2.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(2U, pool2.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";
    EXPECT_EQ(2U, pool2.find_block_bucket(128U)) << "Unexpected bucket with bucket size parameter of 128U";

    tme::pool pool3(8U, { {32U, 16U}, {64U, 16U}, {128U, 16U}, {256U, 16U} }, 2U);
    EXPECT_EQ(0U, pool3.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, pool3.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(1U, pool3.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";
    EXPECT_EQ(1U, pool3.find_block_bucket(64U)) << "Unexpected bucket with bucket size parameter of 64U";
    EXPECT_EQ(2U, pool3.find_block_bucket(65U)) << "Unexpected bucket with bucket size parameter of 65U";
    EXPECT_EQ(2U, pool3.find_block_bucket(128U)) << "Unexpected bucket with bucket size parameter of 128U";

    tme::pool pool4(8U, { {4U, 16U}, {20U, 16U}, {100U, 16U} }, 5U);
    EXPECT_EQ(0U, pool4.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, pool4.find_block_bucket(4U)) << "Unexpected bucket with bucket size parameter of 4U";
    EXPECT_EQ(1U, pool4.find_block_bucket(5U)) << "Unexpected bucket with bucket size parameter of 5U";
    EXPECT_EQ(1U, pool4.find_block_bucket(20U)) << "Unexpected bucket with bucket size parameter of 20U";
    EXPECT_EQ(2U, pool4.find_block_bucket(21U)) << "Unexpected bucket with bucket size parameter of 21U";
    EXPECT_EQ(2U, pool4.find_block_bucket(100U)) << "Unexpected bucket with bucket size parameter of 100U";
}

TEST(pool_test, find_block_bucket_invalid)
{
    tme::pool pool1(16U, { {2U, 16U}, {8U, 16U}, {32U, 16U} }, 4U);
    EXPECT_EQ(0U, pool1.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(3U, pool1.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";

    tme::pool pool2(16U, { {8U, 16U}, {32U, 16U}, {128U, 16U} }, 4U);
    EXPECT_EQ(0U, pool2.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(3U, pool2.find_block_bucket(129U)) << "Unexpected bucket with bucket size parameter of 129U";

    tme::pool pool3(8U, { {32U, 16U}, {64U, 16U}, {128U, 16U}, {256U, 16U} }, 2U);
    EXPECT_EQ(0U, pool3.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(4U, pool3.find_block_bucket(257U)) << "Unexpected bucket with bucket size parameter of 257U";

    tme::pool pool4(8U, { {4U, 16U}, {20U, 16U}, {100U, 16U} }, 5U);
    EXPECT_EQ(0U, pool4.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(3U, pool4.find_block_bucket(101U)) << "Unexpected bucket with bucket size parameter of 101U";
}

template <class value_t, std::size_t limit>
class pool_producer_task
{
public:
    typedef tco::mpmc_ring_queue<value_t*> queue;
    pool_producer_task(typename queue::producer& producer, tme::pool& pool, const std::array<value_t, limit>& input);
    ~pool_producer_task() noexcept;
    void run();
    void produce();
private:
    typename queue::producer& producer_;
    tme::pool& pool_;
    const std::array<value_t, limit>& input_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
pool_producer_task<value_t, limit>::pool_producer_task(typename queue::producer& producer, tme::pool& pool, const std::array<value_t, limit>& input)
    :
	producer_(producer),
	pool_(pool),
	input_(input),
	thread_(nullptr)
{ }

template <class value_t, std::size_t limit>
pool_producer_task<value_t, limit>::~pool_producer_task() noexcept
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
void pool_producer_task<value_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&pool_producer_task::produce, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void pool_producer_task<value_t, limit>::produce()
{
    for (auto input_iter = input_.cbegin(); input_iter != input_.cend();)
    {
	value_t* result = pool_.allocate<value_t>();
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
	    std::cerr << "ERROR: pool allocation failed" << std::endl;
	}
    }
}

template <class value_t, std::size_t limit>
class pool_consumer_task
{
public:
    typedef tco::mpmc_ring_queue<value_t*> queue;
    pool_consumer_task(typename queue::consumer& consumer, tme::pool& pool, std::array<value_t, limit>& output);
    ~pool_consumer_task() noexcept;
    void run();
    void consume();
private:
    typename queue::consumer& consumer_;
    tme::pool& pool_;
    std::array<value_t, limit>& output_;
    std::thread* thread_;
};

template <class value_t, std::size_t limit>
pool_consumer_task<value_t, limit>::pool_consumer_task(typename queue::consumer& consumer, tme::pool& pool, std::array<value_t, limit>& output)
    :
	consumer_(consumer),
	pool_(pool),
	output_(output),
	thread_(nullptr)
{ }

template <class value_t, std::size_t limit>
pool_consumer_task<value_t, limit>::~pool_consumer_task() noexcept
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
void pool_consumer_task<value_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&pool_consumer_task::consume, this));
	thread_ = new std::thread(entry);
    }
}

template <class value_t, std::size_t limit>
void pool_consumer_task<value_t, limit>::consume()
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
		pool_.deallocate<value_t>(tmp);
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

TEST(pool_test, pool_message_pass_string)
{
    typedef tco::mpmc_ring_queue<std::string*> string_queue;
    string_queue queue1(8U, 4U);
    tme::pool pool1(2U, { {sizeof(std::string), 2U} });
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
	pool_producer_task<std::string, 2048U> producer1(queue1.get_producer(), pool1, *input1);
	pool_producer_task<std::string, 2048U> producer2(queue1.get_producer(), pool1, *input2);
	pool_producer_task<std::string, 2048U> producer3(queue1.get_producer(), pool1, *input3);
	pool_producer_task<std::string, 2048U> producer4(queue1.get_producer(), pool1, *input4);
	pool_consumer_task<std::string, 2048U> consumer1(queue1.get_consumer(), pool1, *output1);
	pool_consumer_task<std::string, 2048U> consumer2(queue1.get_consumer(), pool1, *output2);
	pool_consumer_task<std::string, 2048U> consumer3(queue1.get_consumer(), pool1, *output3);
	pool_consumer_task<std::string, 2048U> consumer4(queue1.get_consumer(), pool1, *output4);
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

TEST(pool_test, pool_message_pass_record)
{
    typedef tco::mpmc_ring_queue<record*> record_queue;
    record_queue queue1(8U, 4U);
    tme::pool pool1(2U, { {sizeof(record), 2U} });
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
	pool_producer_task<record, 2048U> producer1(queue1.get_producer(), pool1, *input1);
	pool_producer_task<record, 2048U> producer2(queue1.get_producer(), pool1, *input2);
	pool_producer_task<record, 2048U> producer3(queue1.get_producer(), pool1, *input3);
	pool_producer_task<record, 2048U> producer4(queue1.get_producer(), pool1, *input4);
	pool_consumer_task<record, 2048U> consumer1(queue1.get_consumer(), pool1, *output1);
	pool_consumer_task<record, 2048U> consumer2(queue1.get_consumer(), pool1, *output2);
	pool_consumer_task<record, 2048U> consumer3(queue1.get_consumer(), pool1, *output3);
	pool_consumer_task<record, 2048U> consumer4(queue1.get_consumer(), pool1, *output4);
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
class pool_user_task
{
public:
    pool_user_task(tme::pool& pool, const std::array<input_t, limit>& input, std::array<output_t, limit>& output, const std::function<output_t (const input_t&)>& process);
    ~pool_user_task() noexcept;
    void run();
    void use();
private:
    tme::pool& pool_;
    const std::array<input_t, limit>& input_;
    std::array<output_t, limit>& output_;
    const std::function<output_t (const input_t&)>& process_;
    std::thread* thread_;
};

template <class input_t, class output_t, std::size_t limit>
pool_user_task<input_t, output_t, limit>::pool_user_task(
	tme::pool& pool,
	const std::array<input_t, limit>& input,
	std::array<output_t, limit>& output,
	const std::function<output_t (const input_t&)>& process)
    :
	pool_(pool),
	input_(input),
	output_(output),
	process_(process),
	thread_(nullptr)
{ }

template <class input_t, class output_t, std::size_t limit>
pool_user_task<input_t, output_t, limit>::~pool_user_task() noexcept
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
void pool_user_task<input_t, output_t, limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&pool_user_task::use, this));
	thread_ = new std::thread(entry);
    }
}

template <class input_t, class output_t, std::size_t limit>
void pool_user_task<input_t, output_t, limit>::use()
{
    for (auto iter = 0U; iter < limit; ++iter)
    {
	input_t* result = pool_.allocate<input_t>();
	if (result != nullptr)
	{
	    random_spin();
	    new (result) input_t(input_[iter]);
	    output_[iter] = process_(*result);
	    random_spin();
	    result->~input_t();
	    pool_.deallocate<input_t>(result);
	}
	else
	{
	    std::cerr << "ERROR: pool allocation failed" << std::endl;
	}
    }
}

TEST(pool_test, pool_parallel_use_string)
{
    tme::pool pool1(2U, { {sizeof(std::string), 2U} });
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
	pool_user_task<std::string, std::string, 2048U> task1(pool1, *input1, *actual_output1, process);
	pool_user_task<std::string, std::string, 2048U> task2(pool1, *input2, *actual_output2, process);
	pool_user_task<std::string, std::string, 2048U> task3(pool1, *input3, *actual_output3, process);
	pool_user_task<std::string, std::string, 2048U> task4(pool1, *input4, *actual_output4, process);
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

TEST(pool_test, pool_parallel_use_octshort)
{
    tme::pool pool1(2U, { {sizeof(oct_short), 2U} });
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
	pool_user_task<oct_short, std::uint32_t, 2048U> task1(pool1, *input1, *actual_output1, process);
	pool_user_task<oct_short, std::uint32_t, 2048U> task2(pool1, *input2, *actual_output2, process);
	pool_user_task<oct_short, std::uint32_t, 2048U> task3(pool1, *input3, *actual_output3, process);
	pool_user_task<oct_short, std::uint32_t, 2048U> task4(pool1, *input4, *actual_output4, process);
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

TEST(pool_test, pool_make_unique_basic)
{
    tme::pool pool1(3U, { {sizeof(std::string), 3U} });
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "String in memory pool didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "String in memory pool didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "String in memory pool didn't initialise";
    }
}

TEST(pool_test, calibrate_positive)
{
    std::vector<tme::block_config> input1{ {64U, 4U}, {32U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected1{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual1(tme::calibrate(input1, 2U));
    EXPECT_TRUE(!actual1.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected1.cbegin(), expected1.cend(), actual1.cbegin())) << "Incorrect reordering: "
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
    std::vector<tme::block_config> actual2(tme::calibrate(input2, 2U));
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
    std::vector<tme::block_config> actual3(tme::calibrate(input3, 2U));
    EXPECT_TRUE(!actual3.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected3.cbegin(), expected3.cend(), actual3.cbegin())) << "Incorrect reordering: "
	    << "expected [ "
	    << "{" << expected3[0].block_size << ", " << expected3[0].initial_capacity << "}, "
	    << "{" << expected3[1].block_size << ", " << expected3[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual3[0].block_size << ", " << actual3[0].initial_capacity << "}, "
	    << "{" << actual3[1].block_size << ", " << actual3[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input4{ {16U, 16U}, {64U, 4U} };
    std::vector<tme::block_config> expected4{ {16U, 16U}, {32U, 0U}, {64U, 4U} };
    std::vector<tme::block_config> actual4(tme::calibrate(input4, 2U));
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

TEST(pool_test, calibrate_negative)
{
    std::vector<tme::block_config> input1;
    std::vector<tme::block_config> actual1(tme::calibrate(input1, 2U));
    EXPECT_TRUE(actual1.empty()) << "Empty output from non-empty input";

    std::vector<tme::block_config> input2{ {64U, 4U}, {24U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected2{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual2(tme::calibrate(input2, 0U));
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
    std::vector<tme::block_config> actual3(tme::calibrate(input3, 1U));
    EXPECT_TRUE(!actual3.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected3.cbegin(), expected3.cend(), actual3.cbegin())) << "Incorrect reordering: "
	    << "expected [ "
	    << "{" << expected3[0].block_size << ", " << expected3[0].initial_capacity << "}, "
	    << "{" << expected3[1].block_size << ", " << expected3[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual3[0].block_size << ", " << actual3[0].initial_capacity << "}, "
	    << "{" << actual3[1].block_size << ", " << actual3[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input4{ {16U, 16U}, {64U, 4U} };
    std::vector<tme::block_config> expected4{ {16U, 16U}, {32U, 0U}, {64U, 4U} };
    std::vector<tme::block_config> actual4(tme::calibrate(input4, 1U));
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

// TODO delete all block_pool based test cases below

TEST(pool_test, make_unique_basic)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Unique pool string didn't initialise";
    }
}

TEST(pool_test, make_unique_full)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(2U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::pool_full, result3.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result3.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_unique_recycle)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Unique pool string didn't initialise";
    }
    {
	auto result4 = pool1.make_unique<std::string>("foo");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("foo"), *result4.second) << "Recycled unique pool string didn't initialise";
	auto result5 = pool1.make_unique<std::string>("bar");
	EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("bar"), *result5.second) << "Recycled unique pool string didn't initialise";
	auto result6 = pool1.make_unique<std::string>("blah");
	EXPECT_EQ(tme::make_result::success, result6.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("blah"), *result6.second) << "Recycled unique pool string didn't initialise";
	auto result7 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result7.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result7.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_unique_moved)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U);
    {
	tme::pool_unique_ptr<std::string> moved1;
	tme::pool_unique_ptr<std::string> moved2;
	tme::pool_unique_ptr<std::string> moved3;
	{
	    auto result1 = pool1.make_unique<std::string>("abc123");
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	    auto result2 = pool1.make_unique<std::string>("xyz789");
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	    auto result3 = pool1.make_unique<std::string>("lmn456");
	    EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("lmn456"), *result3.second) << "Unique pool string didn't initialise";
	    moved1 = std::move(result1.second);
	    moved2 = std::move(result2.second);
	    moved3 = std::move(result3.second);
	}
    }
    {
	auto result4 = pool1.make_unique<std::string>("foo");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("foo"), *result4.second) << "Recycled unique pool string didn't initialise";
	auto result5 = pool1.make_unique<std::string>("bar");
	EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("bar"), *result5.second) << "Recycled unique pool string didn't initialise";
	auto result6 = pool1.make_unique<std::string>("blah");
	EXPECT_EQ(tme::make_result::success, result6.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("blah"), *result6.second) << "Recycled unique pool string didn't initialise";
	auto result7 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result7.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result7.second.get()) << "Pointer returned from full pool is not null";
	tme::pool_unique_ptr<std::string> moved4(std::move(result4.second));
	tme::pool_unique_ptr<std::string> moved5(std::move(result5.second));
	tme::pool_unique_ptr<std::string> moved6(std::move(result6.second));
    }
}

constexpr std::size_t double_string()
{
    return sizeof(std::string) * 2;
}

TEST(pool_test, make_unique_large_block)
{
    typedef tme::block_pool<double_string()> string_pool;
    string_pool pool1(3U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Unique pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Unique pool string didn't initialise";
    }
    {
	auto result4 = pool1.make_unique<std::string>("foo");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("foo"), *result4.second) << "Recycled unique pool string didn't initialise";
	auto result5 = pool1.make_unique<std::string>("bar");
	EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("bar"), *result5.second) << "Recycled unique pool string didn't initialise";
	auto result6 = pool1.make_unique<std::string>("blah");
	EXPECT_EQ(tme::make_result::success, result6.first) << "Make reycled unique pool string failed";
	EXPECT_EQ(std::string("blah"), *result6.second) << "Recycled unique pool string didn't initialise";
	auto result7 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result7.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result7.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_unique_array)
{
    typedef std::array<std::uint32_t, 8> uint_array;
    typedef tme::block_pool<sizeof(uint_array)> array_pool;
    array_pool pool1(2U);
    {
	auto result1 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool array failed";
	std::fill(result1.second->begin(), result1.second->end(), 0U);
	auto result2 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool array failed";
	std::fill(result2.second->begin(), result2.second->end(), std::numeric_limits<uint_array::value_type>::max());
	for (const std::uint32_t& value: *(result1.second))
	{
	    EXPECT_EQ(0U, value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result1.second->cbegin()));
	}
	for (const std::uint32_t& value: *(result2.second))
	{
	    EXPECT_EQ(std::numeric_limits<uint_array::value_type>::max(), value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result2.second->cbegin()));
	}
    }
    {
	auto result3 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool array failed";
	std::fill(result3.second->begin(), result3.second->end(), std::numeric_limits<uint_array::value_type>::max());
	auto result4 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled unique pool array failed";
	std::fill(result4.second->begin(), result4.second->end(), 0U);
	for (const std::uint32_t& value: *(result3.second))
	{
	    EXPECT_EQ(std::numeric_limits<uint_array::value_type>::max(), value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result3.second->cbegin()));
	}
	for (const std::uint32_t& value: *(result4.second))
	{
	    EXPECT_EQ(0U, value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result4.second->cbegin()));
	}
	auto result5 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::pool_full, result5.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result5.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_shared_basic)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U);
    {
	auto result1 = pool1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	auto result2 = pool1.make_shared<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	auto result3 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared pool string didn't initialise";
    }
}

TEST(pool_test, make_shared_full)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(2U);
    {
	auto result1 = pool1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	auto result2 = pool1.make_shared<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	auto result3 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::pool_full, result3.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result3.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_shared_recycle)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U);
    {
	auto result1 = pool1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	auto result2 = pool1.make_shared<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	auto result3 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared pool string didn't initialise";
    }
    {
	auto result4 = pool1.make_shared<std::string>("foo");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make reycled shared pool string failed";
	EXPECT_EQ(std::string("foo"), *result4.second) << "Recycled shared pool string didn't initialise";
	auto result5 = pool1.make_shared<std::string>("bar");
	EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled shared pool string failed";
	EXPECT_EQ(std::string("bar"), *result5.second) << "Recycled shared pool string didn't initialise";
	auto result6 = pool1.make_shared<std::string>("blah");
	EXPECT_EQ(tme::make_result::success, result6.first) << "Make reycled shared pool string failed";
	EXPECT_EQ(std::string("blah"), *result6.second) << "Recycled shared pool string didn't initialise";
	auto result7 = pool1.make_shared<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result7.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result7.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_shared_copied)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(3U);
    {
	std::shared_ptr<std::string> copy1;
	std::shared_ptr<std::string> copy2;
	std::shared_ptr<std::string> copy3;
	{
	    auto result1 = pool1.make_shared<std::string>("abc123");
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	    auto result2 = pool1.make_shared<std::string>("xyz789");
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	    auto result3 = pool1.make_shared<std::string>("lmn456");
	    EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared pool string didn't initialise";
	    copy1 = result1.second;
	    copy2 = result2.second;
	    copy3 = result3.second;
	}
	auto result4 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result4.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result4.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result5 = pool1.make_shared<std::string>("!@#");
    EXPECT_EQ(tme::make_result::success, result5.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("!@#"), *result5.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy4 = result5.second;
    auto result6 = pool1.make_shared<std::string>("$%^");
    EXPECT_EQ(tme::make_result::success, result6.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("$%^"), *result6.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy5 = result6.second;
    auto result7 = pool1.make_shared<std::string>("&*(");
    EXPECT_EQ(tme::make_result::success, result7.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("&*("), *result7.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy6 = result7.second;
}

TEST(pool_test, make_shared_large_block)
{
    typedef tme::block_pool<double_string()> string_pool;
    string_pool pool1(3U);
    {
	std::shared_ptr<std::string> copy1;
	std::shared_ptr<std::string> copy2;
	std::shared_ptr<std::string> copy3;
	{
	    auto result1 = pool1.make_shared<std::string>("abc123");
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("abc123"), *result1.second) << "Shared pool string didn't initialise";
	    auto result2 = pool1.make_shared<std::string>("xyz789");
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("xyz789"), *result2.second) << "Shared pool string didn't initialise";
	    auto result3 = pool1.make_shared<std::string>("lmn456");
	    EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("lmn456"), *result3.second) << "Shared pool string didn't initialise";
	    copy1 = result1.second;
	    copy2 = result2.second;
	    copy3 = result3.second;
	}
	auto result4 = pool1.make_unique<std::string>("foobar");
	EXPECT_EQ(tme::make_result::pool_full, result4.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result4.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result5 = pool1.make_shared<std::string>("!@#");
    EXPECT_EQ(tme::make_result::success, result5.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("!@#"), *result5.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy4 = result5.second;
    auto result6 = pool1.make_shared<std::string>("$%^");
    EXPECT_EQ(tme::make_result::success, result6.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("$%^"), *result6.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy5 = result6.second;
    auto result7 = pool1.make_shared<std::string>("&*(");
    EXPECT_EQ(tme::make_result::success, result7.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("&*("), *result7.second) << "Shared pool string didn't initialise";
    std::shared_ptr<std::string> copy6 = result7.second;
}

TEST(pool_test, make_shared_array)
{
    typedef std::array<std::uint32_t, 8> uint_array;
    typedef tme::block_pool<sizeof(uint_array)> array_pool;
    array_pool pool1(2U);
    {
	std::shared_ptr<uint_array> copy1;
	std::shared_ptr<uint_array> copy2;
	{
	    auto result1 = pool1.make_shared<uint_array>();
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool array failed";
	    std::fill(result1.second->begin(), result1.second->end(), 15U);
	    auto result2 = pool1.make_shared<uint_array>();
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool array failed";
	    std::fill(result2.second->begin(), result2.second->end(), 256U);
	    copy1 = result1.second;
	    copy2 = result2.second;
	}
	for (std::uint32_t& value: *copy1)
	{
	    value *= 2;
	}
	for (std::uint32_t& value: *copy2)
	{
	    value *= 2;
	}
	for (const std::uint32_t& value: *copy1)
	{
	    EXPECT_EQ(15U * 2, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(copy1->cbegin()));
	}
	for (const std::uint32_t& value: *copy2)
	{
	    EXPECT_EQ(256U * 2, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(copy2->cbegin()));
	}
	auto result3 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::pool_full, result3.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result3.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result4 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::success, result4.first) << "Make shared pool array failed";
    std::fill(result4.second->begin(), result4.second->end(), 256U);
    std::shared_ptr<uint_array> copy3 = result4.second;
    auto result5 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled shared pool array failed";
    std::fill(result5.second->begin(), result5.second->end(), 15U);
    std::shared_ptr<uint_array> copy4 = result5.second;
    for (const std::uint32_t& value: *(result4.second))
    {
	EXPECT_EQ(256U, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(result4.second->cbegin()));
    }
    for (const std::uint32_t& value: *(result5.second))
    {
	EXPECT_EQ(15U, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(result5.second->cbegin()));
    }
    auto result6 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::pool_full, result6.first) << "Full pool is still allocating";
    EXPECT_EQ(nullptr, result6.second.get()) << "Pointer returned from full pool is not null";
}

TEST(pool_test, make_mixed_full)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(4U);
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	auto result2 = pool1.make_shared<std::string>("!@#");
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("!@#"), *result2.second) << "Shared pool string didn't initialise";
	auto result3 = pool1.make_unique<std::string>("xyz789");
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(std::string("xyz789"), *result3.second) << "Unique pool string didn't initialise";
	auto result4 = pool1.make_shared<std::string>("$%^");
	EXPECT_EQ(tme::make_result::success, result4.first) << "Make shared pool string failed";
	EXPECT_EQ(std::string("$%^"), *result4.second) << "Shared pool string didn't initialise";
	auto result5 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::pool_full, result5.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result5.second.get()) << "Pointer returned from full pool is not null";
    }
}

TEST(pool_test, make_mixed_transferred)
{
    typedef tme::block_pool<sizeof(std::string)> string_pool;
    string_pool pool1(4U);
    {
	tme::pool_unique_ptr<std::string> move1;
	std::shared_ptr<std::string> copy2;
	tme::pool_unique_ptr<std::string> move3;
	std::shared_ptr<std::string> copy4;
	{
	    auto result1 = pool1.make_unique<std::string>("abc123");
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("abc123"), *result1.second) << "Unique pool string didn't initialise";
	    auto result2 = pool1.make_shared<std::string>("!@#");
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("!@#"), *result2.second) << "Shared pool string didn't initialise";
	    auto result3 = pool1.make_unique<std::string>("xyz789");
	    EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	    EXPECT_EQ(std::string("xyz789"), *result3.second) << "Unique pool string didn't initialise";
	    auto result4 = pool1.make_shared<std::string>("$%^");
	    EXPECT_EQ(tme::make_result::success, result4.first) << "Make shared pool string failed";
	    EXPECT_EQ(std::string("$%^"), *result4.second) << "Shared pool string didn't initialise";
	    move1 = std::move(result1.second);
	    copy2 = result2.second;
	    move3 = std::move(result3.second);
	    copy4 = result4.second;
	}
	auto result5 = pool1.make_shared<std::string>("lmn456");
	EXPECT_EQ(tme::make_result::pool_full, result5.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result5.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result6 = pool1.make_unique<std::string>("xyz789");
    EXPECT_EQ(tme::make_result::success, result6.first) << "Make unique pool string failed";
    EXPECT_EQ(std::string("xyz789"), *result6.second) << "Unique pool string didn't initialise";
    auto result7 = pool1.make_shared<std::string>("$%^");
    EXPECT_EQ(tme::make_result::success, result7.first) << "Make shared pool string failed";
    EXPECT_EQ(std::string("$%^"), *result7.second) << "Shared pool string didn't initialise";
}

TEST(pool_test, make_mixed_array)
{
    typedef std::array<std::uint32_t, 8> uint_array;
    typedef tme::block_pool<sizeof(uint_array)> array_pool;
    array_pool pool1(2U);
    {
	std::shared_ptr<uint_array> copy1;
	tme::pool_unique_ptr<uint_array> move2;
	{
	    auto result1 = pool1.make_shared<uint_array>();
	    EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool array failed";
	    std::fill(result1.second->begin(), result1.second->end(), 15U);
	    auto result2 = pool1.make_unique<uint_array>();
	    EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool array failed";
	    std::fill(result2.second->begin(), result2.second->end(), 256U);
	    copy1 = result1.second;
	    move2 = std::move(result2.second);
	}
	for (std::uint32_t& value: *copy1)
	{
	    value *= 2;
	}
	for (std::uint32_t& value: *move2)
	{
	    value *= 2;
	}
	for (const std::uint32_t& value: *copy1)
	{
	    EXPECT_EQ(15U * 2, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(copy1->cbegin()));
	}
	for (const std::uint32_t& value: *move2)
	{
	    EXPECT_EQ(256U * 2, value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(move2->cbegin()));
	}
	auto result3 = pool1.make_unique<uint_array>();
	EXPECT_EQ(tme::make_result::pool_full, result3.first) << "Full pool is still allocating";
	EXPECT_EQ(nullptr, result3.second.get()) << "Pointer returned from full pool is not null";
    }
    auto result4 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::success, result4.first) << "Make recyced shared pool array failed";
    std::fill(result4.second->begin(), result4.second->end(), 256U);
    std::shared_ptr<uint_array> copy3 = result4.second;
    auto result5 = pool1.make_unique<uint_array>();
    EXPECT_EQ(tme::make_result::success, result5.first) << "Make reycled unique pool array failed";
    std::fill(result5.second->begin(), result5.second->end(), 15U);
    for (const std::uint32_t& value: *(result4.second))
    {
	EXPECT_EQ(256U, value) << "Shared pool array didn't initialise correctly at index " << &value - &(*(result4.second->cbegin()));
    }
    for (const std::uint32_t& value: *(result5.second))
    {
	EXPECT_EQ(15U, value) << "Unique pool array didn't initialise correctly at index " << &value - &(*(result5.second->cbegin()));
    }
    auto result6 = pool1.make_shared<uint_array>();
    EXPECT_EQ(tme::make_result::pool_full, result6.first) << "Full pool is still allocating";
    EXPECT_EQ(nullptr, result6.second.get()) << "Pointer returned from full pool is not null";
}

template <std::size_t limit>
class produce_task
{
public:
    typedef tme::pool_unique_ptr<record> unique_value;
    typedef tco::mpmc_ring_queue<unique_value> queue;
    typedef tme::block_pool<sizeof(record)> pool_type;
    produce_task(typename queue::producer& producer, pool_type& pool, const std::array<record, limit>& input);
    ~produce_task() noexcept;
    void run();
    void produce();
private:
    typename queue::producer& producer_;
    pool_type& pool_;
    const std::array<record, limit>& input_;
    std::thread* thread_;
};

template <std::size_t limit>
produce_task<limit>::produce_task(typename queue::producer& producer, pool_type& pool, const std::array<record, limit>& input)
    :
	producer_(producer),
	pool_(pool),
	input_(input),
	thread_(nullptr)
{ }

template <std::size_t limit>
produce_task<limit>::~produce_task() noexcept
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
void produce_task<limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&produce_task::produce, this));
	thread_ = new std::thread(entry);
    }
}

template <std::size_t limit>
void produce_task<limit>::produce()
{
    for (auto iter = input_.cbegin(); iter != input_.cend();)
    {
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    auto result = pool_.make_unique<record>(*iter);
	    if (result.first == tme::make_result::success)
	    {
		tar::retry_with_random_backoff([&] () -> tar::try_state
		{
		    if (producer_.try_enqueue_move(std::move(result.second)) == queue::producer::result::success)
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

template <std::size_t limit>
class consume_task
{
public:
    typedef tme::pool_unique_ptr<record> unique_value;
    typedef tco::mpmc_ring_queue<unique_value> queue;
    consume_task(typename queue::consumer& consumer, std::array<record, limit>& output);
    ~consume_task() noexcept;
    void run();
    void consume();
private:
    typename queue::consumer& consumer_;
    std::array<record, limit>& output_;
    std::thread* thread_;
};

template <std::size_t limit>
consume_task<limit>::consume_task(typename queue::consumer& consumer, std::array<record, limit>& output)
    :
	consumer_(consumer),
	output_(output),
	thread_(nullptr)
{ }

template <std::size_t limit>
consume_task<limit>::~consume_task() noexcept
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
void consume_task<limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&consume_task::consume, this));
	thread_ = new std::thread(entry);
    }
}

template <std::size_t limit>
void consume_task<limit>::consume()
{
    for (auto iter = output_.begin(); iter != output_.end();)
    {
	unique_value tmp;
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    if (consumer_.try_dequeue_move(tmp) == queue::consumer::result::success)
	    {
		*iter = *tmp;
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

TEST(pool_test, messasge_passing)
{
    typedef tme::pool_unique_ptr<record> unique_record;
    typedef tco::mpmc_ring_queue<unique_record> unique_record_queue;
    typedef tme::block_pool<sizeof(record)> record_pool;
    unique_record_queue queue1(64U, 4U);
    record_pool pool1(8192U);
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
	produce_task<2048U> producer1(queue1.get_producer(), pool1, *input1);
	produce_task<2048U> producer2(queue1.get_producer(), pool1, *input2);
	produce_task<2048U> producer3(queue1.get_producer(), pool1, *input3);
	produce_task<2048U> producer4(queue1.get_producer(), pool1, *input4);
	consume_task<2048U> consumer1(queue1.get_consumer(), *output1);
	consume_task<2048U> consumer2(queue1.get_consumer(), *output2);
	consume_task<2048U> consumer3(queue1.get_consumer(), *output3);
	consume_task<2048U> consumer4(queue1.get_consumer(), *output4);
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

template <std::size_t limit>
class use_unique_task
{
public:
    typedef tme::block_pool<sizeof(oct_short)> pool_type;
    use_unique_task(pool_type& pool, const std::array<oct_short, limit>& input, std::array<std::uint32_t, limit>& output);
    ~use_unique_task() noexcept;
    void run();
    void use();
private:
    pool_type& pool_;
    const std::array<oct_short, limit>& input_;
    std::array<std::uint32_t, limit>& output_;
    std::thread* thread_;
};

template <std::size_t limit>
use_unique_task<limit>::use_unique_task(pool_type& pool, const std::array<oct_short, limit>& input, std::array<std::uint32_t, limit>& output)
    :
	pool_(pool),
	input_(input),
	output_(output),
	thread_(nullptr)
{ }

template <std::size_t limit>
use_unique_task<limit>::~use_unique_task() noexcept
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
void use_unique_task<limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&use_unique_task::use, this));
	thread_ = new std::thread(entry);
    }
}

template <std::size_t limit>
void use_unique_task<limit>::use()
{
    for (auto iter = 0U; iter < limit; ++iter)
    {
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    auto result = pool_.make_unique<oct_short>();
	    if (result.first == tme::make_result::success)
	    {
		random_spin();
		tme::pool_unique_ptr<oct_short> tmp = std::move(result.second);
		std::copy(input_[iter].cbegin(), input_[iter].cend(), tmp->begin());
		output_[iter] = 0U;
		std::for_each(tmp->cbegin(), tmp->cend(), [&] (const std::uint16_t value) -> void
		{
		    output_[iter] += value;
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

template <std::size_t limit>
class use_shared_task
{
public:
    typedef tme::block_pool<sizeof(oct_short)> pool_type;
    use_shared_task(pool_type& pool, const std::array<oct_short, limit>& input, std::array<std::uint32_t, limit>& output);
    ~use_shared_task() noexcept;
    void run();
    void use();
private:
    pool_type& pool_;
    const std::array<oct_short, limit>& input_;
    std::array<std::uint32_t, limit>& output_;
    std::thread* thread_;
};

template <std::size_t limit>
use_shared_task<limit>::use_shared_task(pool_type& pool, const std::array<oct_short, limit>& input, std::array<std::uint32_t, limit>& output)
    :
	pool_(pool),
	input_(input),
	output_(output),
	thread_(nullptr)
{ }

template <std::size_t limit>
use_shared_task<limit>::~use_shared_task() noexcept
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
void use_shared_task<limit>::run()
{
    if (!thread_)
    {
	std::function<void ()> entry(std::bind(&use_shared_task::use, this));
	thread_ = new std::thread(entry);
    }
}

template <std::size_t limit>
void use_shared_task<limit>::use()
{
    for (auto iter = 0U; iter < limit; ++iter)
    {
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    std::shared_ptr<oct_short> copy;
	    {
		auto result = pool_.make_shared<oct_short>();
		if (result.first == tme::make_result::success)
		{
		    random_spin();
		    copy = result.second;
		}
		else
		{
		    return tar::try_state::retry;
		}
	    }
	    if (copy)
	    {
		std::copy(input_[iter].cbegin(), input_[iter].cend(), copy->begin());
		output_[iter] = 0U;
		std::for_each(copy->cbegin(), copy->cend(), [&] (const std::uint16_t value) -> void
		{
		    output_[iter] += value;
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

TEST(pool_test, parallel_use)
{
    typedef tme::block_pool<sizeof(oct_short)> oct_pool;
    oct_pool pool1(8192U);
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
	use_unique_task<2048U> unique1(pool1, *input1, *actual_output1);
	use_unique_task<2048U> unique2(pool1, *input2, *actual_output2);
	use_shared_task<2048U> shared3(pool1, *input3, *actual_output3);
	use_shared_task<2048U> shared4(pool1, *input4, *actual_output4);
	unique1.run();
	unique2.run();
	shared3.run();
	shared4.run();
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

TEST(pool_test, sanitise_positive)
{
    std::vector<tme::block_config> input1{ {64U, 4U}, {32U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected1{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual1(tme::range_pool<>::sanitise(input1, 2U));
    EXPECT_TRUE(!actual1.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected1.cbegin(), expected1.cend(), actual1.cbegin())) << "Incorrect reordering: "
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
    std::vector<tme::block_config> actual2(tme::range_pool<>::sanitise(input2, 2U));
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
    std::vector<tme::block_config> actual3(tme::range_pool<>::sanitise(input3, 2U));
    EXPECT_TRUE(!actual3.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected3.cbegin(), expected3.cend(), actual3.cbegin())) << "Incorrect reordering: "
	    << "expected [ "
	    << "{" << expected3[0].block_size << ", " << expected3[0].initial_capacity << "}, "
	    << "{" << expected3[1].block_size << ", " << expected3[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual3[0].block_size << ", " << actual3[0].initial_capacity << "}, "
	    << "{" << actual3[1].block_size << ", " << actual3[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input4{ {16U, 16U}, {64U, 4U} };
    std::vector<tme::block_config> expected4{ {16U, 16U}, {32U, 0U}, {64U, 4U} };
    std::vector<tme::block_config> actual4(tme::range_pool<>::sanitise(input4, 2U));
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

TEST(pool_test, sanitise_negative)
{
    std::vector<tme::block_config> input1;
    std::vector<tme::block_config> actual1(tme::range_pool<>::sanitise(input1, 2U));
    EXPECT_TRUE(actual1.empty()) << "Empty output from non-empty input";

    std::vector<tme::block_config> input2{ {64U, 4U}, {24U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected2{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual2(tme::range_pool<>::sanitise(input2, 0U));
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
    std::vector<tme::block_config> actual3(tme::range_pool<>::sanitise(input3, 1U));
    EXPECT_TRUE(!actual3.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected3.cbegin(), expected3.cend(), actual3.cbegin())) << "Incorrect reordering: "
	    << "expected [ "
	    << "{" << expected3[0].block_size << ", " << expected3[0].initial_capacity << "}, "
	    << "{" << expected3[1].block_size << ", " << expected3[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual3[0].block_size << ", " << actual3[0].initial_capacity << "}, "
	    << "{" << actual3[1].block_size << ", " << actual3[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input4{ {16U, 16U}, {64U, 4U} };
    std::vector<tme::block_config> expected4{ {16U, 16U}, {32U, 0U}, {64U, 4U} };
    std::vector<tme::block_config> actual4(tme::range_pool<>::sanitise(input4, 1U));
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
