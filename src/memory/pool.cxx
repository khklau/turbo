#include "pool.hpp"
#include <algorithm>
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

block_node::block_node(std::size_t value_size, block::capacity_type capacity)
    :
	block_(value_size, capacity, value_size),
	next_(nullptr)
{ }

block_list::block_list(std::size_t value_size, block::capacity_type capacity)
    :
	front_(value_size, capacity)
{ }

block_list::append_result block_list::try_append(block_node& predecessor, const block_node* successor)
{
    return block_list::append_result::beaten;
}

pool::pool(capacity_type default_capacity, const std::vector<block_config>& config)
    :
	pool(default_capacity, config, 2U)
{ }

pool::pool(capacity_type default_capacity, const std::vector<block_config>& config, std::uint8_t step_factor)
    :
	default_capacity_(default_capacity),
	step_factor_(step_factor < 2U ? 2U : step_factor),
	smallest_block_(0U)
{
    std::vector<block_config> calibrated(calibrate(config, step_factor_));
    if (!calibrated.empty())
    {
	smallest_block_ = calibrated.cbegin()->block_size;
    }
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
	return result;
    }
    else
    {
	return std::vector<block_config>();
    }
}

} // namespace memory
} // namespace turbo
