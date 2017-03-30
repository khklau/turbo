#include "pool.hpp"
#include "pool.hxx"
#include <cmath>
#include <algorithm>
#include <turbo/memory/alignment.hpp>
#include <turbo/memory/alignment.hxx>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace memory {

block_config::block_config()
    :
	block_config(0U, 0U, 0U)
{ }

block_config::block_config(std::size_t size, capacity_type capacity)
    :
	block_config(size, capacity, 2U)
{ }

block_config::block_config(std::size_t size, capacity_type capacity, std::size_t growth)
    :
	block_size(size),
	initial_capacity(capacity),
	growth_factor(growth < 2U ? 2U : growth)
{ }

bool block_config::operator<(const block_config& other) const
{
    return block_size < other.block_size;
}

bool block_config::operator==(const block_config& other) const
{
    return block_size == other.block_size
	    && initial_capacity == other.initial_capacity
	    && growth_factor == other.growth_factor;
}

block_list::iterator::iterator()
    :
	base_iterator()
{ }

block_list::iterator::iterator(block_list::node* pointer)
    :
	base_iterator(pointer)
{ }

block_list::iterator::iterator(const iterator& other)
    :
	base_iterator(static_cast<const base_iterator&>(other))
{ }

block_list::iterator& block_list::iterator::operator=(const iterator& other)
{
    static_cast<base_iterator&>(*this) = static_cast<const base_iterator&>(other);
    return *this;
}

bool block_list::iterator::operator==(const iterator& other) const
{
    return static_cast<const base_iterator&>(*this) == static_cast<const base_iterator&>(other);
}

typename block_list::iterator::block_type& block_list::iterator::operator*()
{
    if (TURBO_LIKELY(is_valid()))
    {
	return pointer_->mutate_block();
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::basic_iterator");
    }
}

typename block_list::iterator::block_type* block_list::iterator::operator->()
{
    if (TURBO_LIKELY(is_valid()))
    {
	return &(pointer_->mutate_block());
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::basic_iterator");
    }
}

block_list::append_result block_list::iterator::try_append(std::unique_ptr<block_list::node> successor)
{
    if (TURBO_LIKELY(is_valid()))
    {
	successor->mutate_next().store(nullptr, std::memory_order_release);
	node* next = pointer_->get_next().load(std::memory_order_acquire);
	if (next == nullptr)
	{
	    if (pointer_->mutate_next().compare_exchange_strong(next, successor.get(), std::memory_order_release))
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

block_list::const_iterator::const_iterator()
    :
	base_iterator()
{ }

block_list::const_iterator::const_iterator(const block_list::node* pointer)
    :
	base_iterator(pointer)
{ }

block_list::const_iterator::const_iterator(const const_iterator& other)
    :
	base_iterator(static_cast<const base_iterator&>(other))
{ }

block_list::const_iterator& block_list::const_iterator::operator=(const const_iterator& other)
{
    static_cast<base_iterator&>(*this) = static_cast<const base_iterator&>(other);
    return *this;
}

bool block_list::const_iterator::operator==(const const_iterator& other) const
{
    return static_cast<const base_iterator&>(*this) == static_cast<const base_iterator&>(other);
}

typename block_list::const_iterator::block_type& block_list::const_iterator::operator*() const
{
    if (TURBO_LIKELY(is_valid()))
    {
	return pointer_->get_block();
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::basic_const_iterator");
    }
}

typename block_list::const_iterator::block_type* block_list::const_iterator::operator->() const
{
    if (TURBO_LIKELY(is_valid()))
    {
	return &(pointer_->get_block());
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::basic_const_iterator");
    }
}

block_list::node::node(std::size_t value_size, block::capacity_type capacity)
    :
	block_(value_size, capacity, value_size),
	next_(nullptr)
{ }

block_list::node::node(const node& other)
    :
	block_(other.block_),
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

bool block_list::node::operator==(const node& other) const
{
    node* this_next = this->next_.load(std::memory_order_acquire);
    node* other_next = other.next_.load(std::memory_order_acquire);
    return this->block_ == other.block_
	&& ((this_next == nullptr && other_next == nullptr) || (this_next != nullptr && other_next != nullptr));
}

block_list::block_list(std::size_t value_size, block::capacity_type capacity)
    :
	block_list(value_size, capacity, 2U)
{ }

block_list::block_list(std::size_t value_size, block::capacity_type capacity, std::size_t growth_factor)
    :
	value_size_(value_size),
	growth_factor_(growth_factor),
	first_(value_size, capacity)
{ }

block_list::block_list(const block_config& config)
    :
	value_size_(config.block_size),
	growth_factor_(config.growth_factor),
	first_(config.block_size, config.initial_capacity)
{ }

block_list::block_list(const block_list& other)
    :
	value_size_(other.value_size_),
	growth_factor_(other.growth_factor_),
	first_(other.first_)
{
    auto this_iter = this->begin();
    auto other_iter = other.cbegin();
    // first_ has already been copied, so skip it
    ++other_iter;
    for (; other_iter != other.cend(); ++other_iter)
    {
	if (other_iter.is_valid())
	{
	    this_iter.try_append(clone_node(other_iter.get_node()));
	    ++this_iter;
	}
    }
}

bool block_list::operator==(const block_list& other) const
{
    bool is_list_matching = true;
    bool is_length_matching = true;
    auto this_iter = this->cbegin();
    for (auto other_iter = other.cbegin(); other_iter != other.cend() && this_iter != this->cend();)
    {
	is_list_matching = is_list_matching && (*this_iter == *other_iter);
	++this_iter;
	++other_iter;
	is_length_matching = is_length_matching
		&& ((other_iter != other.cend() && this_iter != this->cend()) || (other_iter == other.cend() && this_iter == this->cend()));
    }
    return this->value_size_ == other.value_size_
	&& this->growth_factor_ == other.growth_factor_
	&& is_list_matching
	&& is_length_matching;
}

std::unique_ptr<block_list::node> block_list::create_node(block::capacity_type capacity) const
{
    return std::move(std::unique_ptr<block_list::node>(new block_list::node(value_size_, capacity)));
}

std::unique_ptr<block_list::node> block_list::clone_node(const node& other) const
{
    return std::move(std::unique_ptr<block_list::node>(new block_list::node(other)));
}

pool::pool(capacity_type default_capacity, const std::vector<block_config>& config)
    :
	pool(calibrate(config), default_capacity)
{ }

pool::pool(const std::vector<block_config>& config, capacity_type default_capacity)
    :
	default_capacity_(std::llround(std::pow(2U, std::ceil(std::log2(default_capacity))))),
	smallest_block_exponent_(std::llround(std::log2(config.cbegin()->block_size))),
	block_map_(config.cbegin(), config.cend())
{ }

pool::pool(const pool& other)
    :
	default_capacity_(other.default_capacity_),
	smallest_block_exponent_(other.smallest_block_exponent_),
	block_map_(other.block_map_)
{ }

bool pool::operator==(const pool& other) const
{
    return this->default_capacity_ == other.default_capacity_
	&& this->smallest_block_exponent_ == other.smallest_block_exponent_
	&& this->block_map_ == other.block_map_;
}

void* pool::allocate(std::size_t value_size, std::size_t value_alignment, capacity_type quantity, const void*)
{
    const std::size_t bucket = find_block_bucket(calc_total_aligned_size(value_size, value_alignment, quantity));
    if (TURBO_LIKELY(bucket < block_map_.size()))
    {
	void* allocation = nullptr;
	for (auto iter = block_map_[bucket].begin(); allocation == nullptr && iter != block_map_[bucket].end(); ++iter)
	{
	    allocation = iter->allocate();
	    if (allocation == nullptr && iter.is_last())
	    {
		block::capacity_type capacity = iter->is_empty() ?
			default_capacity_ :
			iter->get_capacity() * block_map_[bucket].get_growth_factor();
		iter.try_append(std::move(block_map_[bucket].create_node(capacity)));
	    }
	}
	return allocation;
    }
    else
    {
	return nullptr;
    }
}

void pool::deallocate(std::size_t value_size, std::size_t value_alignment, void* pointer, capacity_type quantity)
{
    const std::size_t bucket = find_block_bucket(calc_total_aligned_size(value_size, value_alignment, quantity));
    if (TURBO_LIKELY(bucket < block_map_.size()))
    {
	for (auto&& block : block_map_[bucket])
	{
	    if (block.in_range(pointer))
	    {
		block.free(pointer);
		break;
	    }
	}
    }
}

std::vector<block_config> calibrate(const std::vector<block_config>& config)
{
    std::vector<block_config> sorted(config);
    std::stable_sort(sorted.begin(), sorted.end());
    if (TURBO_LIKELY(!sorted.empty()))
    {
	std::size_t desired_size = std::llround(std::pow(2U, std::ceil(std::log2(sorted.cbegin()->block_size))));
	std::vector<block_config> result;
	auto current_step = sorted.cbegin();
	do
	{
	    auto next_step = std::find_if(current_step, sorted.cend(), [&] (const block_config& config) -> bool
	    {
		return desired_size < config.block_size;
	    });
	    if (next_step == current_step)
	    {
		// no configuration set for this particular desired size
		result.emplace_back(desired_size, 0U);
	    }
	    else
	    {
		std::size_t total_capacity = 0U;
		std::for_each(current_step, next_step, [&] (const block_config& config) -> void
		{
		    total_capacity += config.initial_capacity;
		});
		result.emplace_back(
			desired_size,
			total_capacity,
			std::llround(std::pow(2U, std::ceil(std::log2(current_step->growth_factor)))));
		current_step = next_step;
	    }
	    desired_size *= 2U;
	}
	while (current_step != sorted.cend());
	return std::move(result);
    }
    else
    {
	return std::move(std::vector<block_config>());
    }
}

} // namespace memory
} // namespace turbo
