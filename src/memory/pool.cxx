#include "pool.hpp"
#include <cmath>
#include <algorithm>
#include <turbo/memory/alignment.hpp>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace memory {

block_config::block_config()
    :
	block_size(0U),
	initial_capacity(0U)
{ }

block_config::block_config(std::size_t size, capacity_type capacity)
    :
	block_size(size),
	initial_capacity(capacity)
{ }

bool block_config::operator<(const block_config& other) const
{
    return block_size < other.block_size;
}

bool block_config::operator==(const block_config& other) const
{
    return block_size == other.block_size && initial_capacity == other.initial_capacity;
}

block_list::invalid_dereference::invalid_dereference(const std::string& what)
    :
	out_of_range(what)
{ }

block_list::invalid_dereference::invalid_dereference(const char* what)
    :
	out_of_range(what)
{ }

block_list::iterator::iterator()
    :
	pointer_(nullptr)
{ }

block_list::iterator::iterator(block_list::node* pointer)
    :
	pointer_(pointer)
{ }

block_list::iterator::iterator(const iterator& other)
    :
	pointer_(other.pointer_)
{ }

block_list::iterator& block_list::iterator::operator=(const iterator& other)
{
    if (TURBO_LIKELY(this != &other))
    {
	pointer_ = other.pointer_;
    }
    return *this;
}

bool block_list::iterator::operator==(const iterator& other) const
{
    return pointer_ == other.pointer_;
}

block& block_list::iterator::operator*()
{
    if (TURBO_LIKELY(is_valid()))
    {
	return pointer_->get_block();
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::iterator");
    }
}

block* block_list::iterator::operator->()
{
    if (TURBO_LIKELY(is_valid()))
    {
	return &(pointer_->get_block());
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::iterator");
    }
}

block_list::iterator& block_list::iterator::operator++()
{
    if (TURBO_LIKELY(is_valid()))
    {
	block_list::node* next = pointer_->get_next().load(std::memory_order_acquire);
	pointer_ = next;
    }
    return *this;
}

block_list::iterator block_list::iterator::operator++(int)
{
    if (TURBO_LIKELY(is_valid()))
    {
	iterator tmp = *this;
	++(*this);
	return tmp;
    }
    else
    {
	return *this;
    }
}

block_list::append_result block_list::iterator::try_append(std::unique_ptr<block_list::node> successor)
{
    if (TURBO_LIKELY(is_valid()))
    {
	successor->get_next().store(nullptr, std::memory_order_release);
	node* next = pointer_->get_next().load(std::memory_order_acquire);
	if (next == nullptr)
	{
	    if (pointer_->get_next().compare_exchange_strong(next, successor.get(), std::memory_order_release))
	    {
		successor.release();
		return block_list::append_result::success;
	    }
	}
	return block_list::append_result::beaten;
    }
    else
    {
	throw block_list::invalid_dereference("cannot append to invalid block_list::iterator");
    }
}

block_list::node::node(std::size_t value_size, block::capacity_type capacity)
    :
	block_(value_size, capacity, value_size),
	next_(nullptr)
{ }

block_list::node::~node() noexcept
{
    try
    {
	node* next = next_.load(std::memory_order_acquire);
	if (next != nullptr)
	{
	    delete next;
	}
    }
    catch (...)
    {
	// Do nothing
    }
}

block_list::block_list(std::size_t value_size, block::capacity_type capacity)
    :
	value_size_(value_size),
	first_(value_size, capacity)
{ }

block_list::block_list(const block_config& config)
    :
	value_size_(config.block_size),
	first_(config.block_size, config.initial_capacity)
{ }

std::unique_ptr<block_list::node> block_list::create_node(block::capacity_type capacity)
{
    return std::move(std::unique_ptr<block_list::node>(new block_list::node(value_size_, capacity)));
}

pool::pool(capacity_type default_capacity, const std::vector<block_config>& config)
    :
	pool(default_capacity, 2U, calibrate(config, 2U))
{ }

pool::pool(capacity_type default_capacity, const std::vector<block_config>& config, std::uint8_t step_factor)
    :
	pool(default_capacity, step_factor, calibrate(config, step_factor))
{ }

pool::pool(capacity_type default_capacity, std::uint8_t step_factor, const std::vector<block_config>& config)
    :
	default_capacity_(default_capacity),
	step_factor_(step_factor < 2U ? 2U : step_factor),
	smallest_block_(config.cbegin()->block_size),
	block_map_(config.cbegin(), config.cend())
{ }

std::size_t pool::find_block_bucket(std::size_t allocation_size) const
{
    if (allocation_size <= smallest_block_)
    {
	// to prevent underflow error
	return 0U;
    }
    else
    {
	return static_cast<std::size_t>(std::ceil(std::log(static_cast<double>(allocation_size) / smallest_block_) / std::log(step_factor_)));
    }
}

void* pool::allocate(std::size_t value_size, std::size_t value_alignment, capacity_type quantity, const void*)
{
    const std::size_t total_size = calc_total_aligned_size(value_size, value_alignment, quantity);
    return nullptr;
}

std::vector<block_config> calibrate(const std::vector<block_config>& config, std::uint8_t step_factor)
{
    std::vector<block_config> sorted(config);
    std::stable_sort(sorted.begin(), sorted.end());
    if (TURBO_LIKELY(!sorted.empty()))
    {
	if (TURBO_UNLIKELY(step_factor < 2))
	{
	    step_factor = 2;
	}
	std::size_t desired_size = sorted.cbegin()->block_size;
	std::vector<block_config> result;
	auto this_step = sorted.cbegin();
	do
	{
	    auto next_step = std::find_if(this_step, sorted.cend(), [&] (const block_config& config) -> bool
	    {
		return desired_size < config.block_size;
	    });
	    if (next_step == this_step)
	    {
		result.emplace_back(desired_size, 0U);
	    }
	    else
	    {
		std::size_t total_capacity = 0U;
		std::for_each(this_step, next_step, [&] (const block_config& config) -> void
		{
		    total_capacity += config.initial_capacity;
		});
		result.emplace_back(desired_size, total_capacity);
	    }
	    desired_size *= step_factor;
	    this_step = next_step;
	}
	while (this_step != sorted.cend());
	return std::move(result);
    }
    else
    {
	return std::move(std::vector<block_config>());
    }
}

} // namespace memory
} // namespace turbo
