#include "slab_allocator.hpp"
#include "slab_allocator.hxx"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <turbo/memory/alignment.hpp>
#include <turbo/memory/alignment.hxx>
#include <turbo/memory/block.hxx>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace memory {

pool::pool(capacity_type contingency_capacity, const std::vector<block_config>& config)
    :
	pool(calibrate(contingency_capacity, config))
{ }

pool::pool(const std::vector<block_config>& config)
    :
	smallest_block_exponent_(std::llround(std::log2(config.cbegin()->block_size))),
	block_map_(config.cbegin(), config.cend())
{ }

pool::pool(const pool& other)
    :
	smallest_block_exponent_(other.smallest_block_exponent_),
	block_map_(other.block_map_)
{ }

pool& pool::operator=(const pool& other)
{
    if (this != &other
	    && this->smallest_block_exponent_ == other.smallest_block_exponent_
	    && this->block_map_.size() == other.block_map_.size())
    {
	std::copy_n(other.block_map_.cbegin(), this->block_map_.size(), this->block_map_.begin());
    }
    return *this;
}

bool pool::operator==(const pool& other) const
{
    return this->smallest_block_exponent_ == other.smallest_block_exponent_ && this->block_map_ == other.block_map_;
}

const std::vector<block_config> pool::get_block_config() const
{
    std::vector<block_config> output;
    for (const block_list& list: block_map_)
    {
	output.emplace_back(
		list.get_value_size(),
		list.cbegin()->get_capacity(),
		list.get_contingency_capacity(),
		list.get_growth_factor());
    }
    return std::move(output);
}

void* pool::allocate(std::size_t value_size, std::size_t value_alignment, capacity_type quantity, const void*)
{
    const std::size_t bucket = find_block_bucket(calc_total_aligned_size(value_size, value_alignment, quantity));
    if (TURBO_LIKELY(value_size != 0U && quantity != 0U && bucket < block_map_.size()))
    {
	return block_map_[bucket].allocate();
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

std::vector<block_config> calibrate(block::capacity_type contingency_capacity, const std::vector<block_config>& config)
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
		result.emplace_back(desired_size, 0U, contingency_capacity);
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
			contingency_capacity,
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
