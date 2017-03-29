#include <turbo/memory/pool.hpp>
#include <turbo/memory/pool.hxx>
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

TEST(pool_test, list_invalid_iterator)
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

TEST(pool_test, list_invalid_append)
{
    tme::block_list list1(sizeof(std::int64_t), 4U);
    auto iter1 = list1.end();
    auto node1 = list1.create_node(8U);
    ASSERT_THROW(iter1.try_append(std::move(node1)), tme::block_list::invalid_dereference) << "Appending to invalid iterator succeeded";
}

TEST(pool_test, list_use_first_node)
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

TEST(pool_test, list_sequential_append)
{
    tme::block_list list1(sizeof(std::int64_t), 16U);
    auto iter1 = list1.begin();
    EXPECT_TRUE(16U <= iter1->get_capacity()) << "Capacity of first block in block list is less than requested";
    EXPECT_EQ(tme::block_list::append_result::success, iter1.try_append(std::move(list1.create_node(8U)))) << "Append failed";
    EXPECT_EQ(tme::block_list::append_result::beaten, iter1.try_append(std::move(list1.create_node(16U)))) << "Appending to middle of list succeeded";
    ++iter1;
    EXPECT_TRUE(8U <= iter1->get_capacity()) << "Capacity of second block in block list is less than requested";
    EXPECT_EQ(tme::block_list::append_result::success, iter1.try_append(std::move(list1.create_node(4U)))) << "Append failed";
    EXPECT_EQ(tme::block_list::append_result::beaten, iter1.try_append(std::move(list1.create_node(16U)))) << "Appending to middle of list succeeded";
    ++iter1;
    EXPECT_TRUE(4U <= iter1->get_capacity()) << "Capacity of third block in block list is less than requested";

    tme::block_list list2(sizeof(std::int64_t), 0U);
    auto iter2 = list2.begin();
    EXPECT_EQ(0U, iter2->get_capacity()) << "Capacity of first empty block is not zero";
    EXPECT_EQ(tme::block_list::append_result::success, iter2.try_append(std::move(list2.create_node(8U)))) << "Append to empty block failed";
    EXPECT_EQ(tme::block_list::append_result::beaten, iter2.try_append(std::move(list2.create_node(16U)))) << "Appending to middle of list succeeded";
    ++iter2;
    EXPECT_TRUE(8U <= iter2->get_capacity()) << "Capacity of second block in block list is less than requested";
    EXPECT_EQ(tme::block_list::append_result::success, iter2.try_append(std::move(list2.create_node(4U)))) << "Append failed";
    EXPECT_EQ(tme::block_list::append_result::beaten, iter2.try_append(std::move(list2.create_node(16U)))) << "Appending to middle of list succeeded";
    ++iter2;
    EXPECT_TRUE(4U <= iter2->get_capacity()) << "Capacity of first block in block list is less than requested";
}

TEST(pool_test, list_copy_construction)
{
    tme::block_list list1(sizeof(std::uint64_t), 4U);
    auto iter1 = list1.begin();
    std::uint64_t* allocation1a = static_cast<std::uint64_t*>(iter1->allocate());
    *allocation1a = 17U;
    std::uint64_t* allocation1b = static_cast<std::uint64_t*>(iter1->allocate());
    *allocation1b = 432U;
    std::uint64_t* allocation1c = static_cast<std::uint64_t*>(iter1->allocate());
    *allocation1c = 1097U;
    tme::block_list list2(list1);
    EXPECT_TRUE(list1 == list2) << "Copy constructed block list is not equal to the original";
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
    tme::pool pool1(16U, { {2U, 16U}, {8U, 16U}, {32U, 16U} });
    EXPECT_EQ(0U, pool1.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, pool1.find_block_bucket(2U)) << "Unexpected bucket with bucket size parameter of 2U";
    EXPECT_EQ(1U, pool1.find_block_bucket(3U)) << "Unexpected bucket with bucket size parameter of 3U";
    EXPECT_EQ(1U, pool1.find_block_bucket(4U)) << "Unexpected bucket with bucket size parameter of 3U";
    EXPECT_EQ(2U, pool1.find_block_bucket(5U)) << "Unexpected bucket with bucket size parameter of 8U";
    EXPECT_EQ(2U, pool1.find_block_bucket(8U)) << "Unexpected bucket with bucket size parameter of 8U";
    EXPECT_EQ(3U, pool1.find_block_bucket(9U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(3U, pool1.find_block_bucket(16U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(4U, pool1.find_block_bucket(17U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(4U, pool1.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";

    tme::pool pool2(16U, { {8U, 16U}, {32U, 16U}, {128U, 16U} });
    EXPECT_EQ(0U, pool2.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, pool2.find_block_bucket(8U)) << "Unexpected bucket with bucket size parameter of 8U";
    EXPECT_EQ(1U, pool2.find_block_bucket(9U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(1U, pool2.find_block_bucket(16U)) << "Unexpected bucket with bucket size parameter of 9U";
    EXPECT_EQ(2U, pool2.find_block_bucket(17U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(2U, pool2.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(3U, pool2.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";
    EXPECT_EQ(3U, pool2.find_block_bucket(64U)) << "Unexpected bucket with bucket size parameter of 33U";
    EXPECT_EQ(4U, pool2.find_block_bucket(65U)) << "Unexpected bucket with bucket size parameter of 128U";
    EXPECT_EQ(4U, pool2.find_block_bucket(128U)) << "Unexpected bucket with bucket size parameter of 128U";

    tme::pool pool3(8U, { {32U, 16U}, {64U, 16U}, {128U, 16U}, {256U, 16U} });
    EXPECT_EQ(0U, pool3.find_block_bucket(1U)) << "Unexpected bucket with bucket size parameter of 1U";
    EXPECT_EQ(0U, pool3.find_block_bucket(32U)) << "Unexpected bucket with bucket size parameter of 32U";
    EXPECT_EQ(1U, pool3.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";
    EXPECT_EQ(1U, pool3.find_block_bucket(64U)) << "Unexpected bucket with bucket size parameter of 64U";
    EXPECT_EQ(2U, pool3.find_block_bucket(65U)) << "Unexpected bucket with bucket size parameter of 65U";
    EXPECT_EQ(2U, pool3.find_block_bucket(128U)) << "Unexpected bucket with bucket size parameter of 128U";
}

TEST(pool_test, find_block_bucket_invalid)
{
    tme::pool pool1(16U, { {2U, 16U}, {8U, 16U}, {32U, 16U} });
    EXPECT_EQ(0U, pool1.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(5U, pool1.find_block_bucket(33U)) << "Unexpected bucket with bucket size parameter of 33U";

    tme::pool pool2(16U, { {8U, 16U}, {32U, 16U}, {128U, 16U} });
    EXPECT_EQ(0U, pool2.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(5U, pool2.find_block_bucket(129U)) << "Unexpected bucket with bucket size parameter of 129U";

    tme::pool pool3(8U, { {32U, 16U}, {64U, 16U}, {128U, 16U}, {256U, 16U} });
    EXPECT_EQ(0U, pool3.find_block_bucket(0U)) << "Unexpected bucket with bucket size parameter of 0U";
    EXPECT_EQ(4U, pool3.find_block_bucket(257U)) << "Unexpected bucket with bucket size parameter of 257U";
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
    tme::pool pool2(2U, { {2U, 32U}, {32U, 32U} });
    {
	auto result1 = pool1.make_unique<std::uint64_t>(123U);
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make unique pool string failed";
	EXPECT_EQ(123U, *result1.second) << "Integer in initially empty memory pool didn't initialise";
	auto result2 = pool1.make_unique<std::uint64_t>(456U);
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make unique pool string failed";
	EXPECT_EQ(456U, *result2.second) << "Integer in initially empty memory pool didn't initialise";
	auto result3 = pool1.make_unique<std::uint64_t>(789U);
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make unique pool string failed";
	EXPECT_EQ(789U, *result3.second) << "Integer in initially empty memory pool didn't initialise";
    }
}

TEST(pool_test, pool_make_unique_invalid)
{
    tme::pool pool1(8U, { {sizeof(std::uint8_t), 8U} });
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::pool_full, result1.first) << "Making a unique_ptr for a value that is larger than the configured pool succeeded";
	EXPECT_EQ(nullptr, result1.second.get()) << "The unique_ptr returned when the requested value is larger than the configured pool is not nullptr";
    }
}

TEST(pool_test, pool_make_shared_basic)
{
    tme::pool pool1(3U, { {sizeof(std::string), 3U} });
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
    tme::pool pool2(2U, { {2U, 32U}, {32U, 32U} });
    {
	auto result1 = pool1.make_shared<std::uint64_t>(123U);
	EXPECT_EQ(tme::make_result::success, result1.first) << "Make shared pool string failed";
	EXPECT_EQ(123U, *result1.second) << "Integer in initially empty memory pool didn't initialise";
	auto result2 = pool1.make_shared<std::uint64_t>(456U);
	EXPECT_EQ(tme::make_result::success, result2.first) << "Make shared pool string failed";
	EXPECT_EQ(456U, *result2.second) << "Integer in initially empty memory pool didn't initialise";
	auto result3 = pool1.make_shared<std::uint64_t>(789U);
	EXPECT_EQ(tme::make_result::success, result3.first) << "Make shared pool string failed";
	EXPECT_EQ(789U, *result3.second) << "Integer in initially empty memory pool didn't initialise";
    }
}

TEST(pool_test, pool_make_shared_invalid)
{
    tme::pool pool1(8U, { {sizeof(std::uint8_t), 8U} });
    {
	auto result1 = pool1.make_shared<std::string>("abc123");
	EXPECT_EQ(tme::make_result::pool_full, result1.first) << "Making a shared_ptr for a value that is larger than the configured pool succeeded";
	EXPECT_EQ(nullptr, result1.second.get()) << "The shared_ptr returned when the requested value is larger than the configured pool is not nullptr";
    }
}

TEST(pool_test, pool_make_mixed_basic)
{
    tme::pool pool1(4U, { {sizeof(std::string), 4U} });
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
    }
}

TEST(pool_test, pool_make_mixed_invalid)
{
    tme::pool pool1(8U, { {sizeof(std::uint8_t), 8U} });
    {
	auto result1 = pool1.make_unique<std::string>("abc123");
	EXPECT_EQ(tme::make_result::pool_full, result1.first) << "Making a unique_ptr for a value that is larger than the configured pool succeeded";
	EXPECT_EQ(nullptr, result1.second.get()) << "The unique_ptr returned when the requested value is larger than the configured pool is not nullptr";
	auto result2 = pool1.make_shared<record>(4U, 5U, 6U);
	EXPECT_EQ(tme::make_result::pool_full, result2.first) << "Making a shared_ptr for a value that is larger than the configured pool succeeded";
	EXPECT_EQ(nullptr, result2.second.get()) << "The shared_ptr returned when the requested value is larger than the configured pool is not nullptr";
    }
}

TEST(pool_test, calibrate_positive)
{
    std::vector<tme::block_config> input1{ {64U, 4U}, {32U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected1{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual1(tme::calibrate(input1));
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
    std::vector<tme::block_config> actual2(tme::calibrate(input2));
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
    std::vector<tme::block_config> actual3(tme::calibrate(input3));
    EXPECT_TRUE(!actual3.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected3.cbegin(), expected3.cend(), actual3.cbegin())) << "Incorrect calibration for config requiring input bucket merging: "
	    << "expected [ "
	    << "{" << expected3[0].block_size << ", " << expected3[0].initial_capacity << "}, "
	    << "{" << expected3[1].block_size << ", " << expected3[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual3[0].block_size << ", " << actual3[0].initial_capacity << "}, "
	    << "{" << actual3[1].block_size << ", " << actual3[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input4{ {16U, 16U}, {64U, 4U} };
    std::vector<tme::block_config> expected4{ {16U, 16U}, {32U, 0U}, {64U, 4U} };
    std::vector<tme::block_config> actual4(tme::calibrate(input4));
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
    std::vector<tme::block_config> expected5{ {8U, 4U}, {16U, 0U}, {32U, 16U} };
    std::vector<tme::block_config> actual5(tme::calibrate(input5));
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

TEST(pool_test, calibrate_repeating)
{
    std::vector<tme::block_config> input1{ {64U, 4U}, {32U, 8U}, {64U, 16U} };
    std::vector<tme::block_config> expected1{ {32U, 8U}, {64U, 20U} };
    std::vector<tme::block_config> actual1(tme::calibrate(input1));
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
    std::vector<tme::block_config> actual2(tme::calibrate(input2));
    EXPECT_TRUE(!actual2.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected2.cbegin(), expected2.cend(), actual2.cbegin())) << "Incorrect calibration for config requiring just a reorder: "
	    << "expected [ "
	    << "{" << expected2[0].block_size << ", " << expected2[0].initial_capacity << "}, "
	    << "{" << expected2[1].block_size << ", " << expected2[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual2[0].block_size << ", " << actual2[0].initial_capacity << "}, "
	    << "{" << actual2[1].block_size << ", " << actual2[1].initial_capacity << "} ]";

    std::vector<tme::block_config> input3{ {8U, 4U}, {24U, 8U}, {24U, 16U} };
    std::vector<tme::block_config> expected3{ {8U, 4U}, {16U, 0U}, {32U, 24U} };
    std::vector<tme::block_config> actual3(tme::calibrate(input3));
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
    std::vector<tme::block_config> actual4(tme::calibrate(input4));
    EXPECT_TRUE(!actual4.empty()) << "Empty output from non-empty input";
    EXPECT_TRUE(std::equal(expected4.cbegin(), expected4.cend(), actual4.cbegin())) << "Incorrect calibration for config requiring a bucket size adjustment: "
	    << "expected [ "
	    << "{" << expected4[0].block_size << ", " << expected4[0].initial_capacity << "}, "
	    << "{" << expected4[1].block_size << ", " << expected4[1].initial_capacity << "} ] - "
	    << "actual [ "
	    << "{" << actual4[0].block_size << ", " << actual4[0].initial_capacity << "}, "
	    << "{" << actual4[1].block_size << ", " << actual4[1].initial_capacity << "} ]";
}

TEST(pool_test, calibrate_negative)
{
    std::vector<tme::block_config> input1;
    std::vector<tme::block_config> actual1(tme::calibrate(input1));
    EXPECT_TRUE(actual1.empty()) << "Empty output from non-empty input";

    std::vector<tme::block_config> input2{ {64U, 4U}, {24U, 8U}, {16U, 16U} };
    std::vector<tme::block_config> expected2{ {16U, 16U}, {32U, 8U}, {64U, 4U} };
    std::vector<tme::block_config> actual2(tme::calibrate(input2));
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
    std::vector<tme::block_config> actual3(tme::calibrate(input3));
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
    std::vector<tme::block_config> actual4(tme::calibrate(input4));
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
