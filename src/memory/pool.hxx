#ifndef TURBO_MEMORY_POOL_HXX
#define TURBO_MEMORY_POOL_HXX

#include <turbo/memory/pool.hpp>
#include <cstring>
#include <algorithm>
#include <limits>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/toolset/extension.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>

namespace turbo {
namespace memory {

template <std::size_t block_size_c, template <class type_t> class allocator_t>
block_pool<block_size_c, allocator_t>::block_pool(capacity_type capacity)
    :
	free_list_(capacity, 0U),
	block_list_(capacity)
{
    namespace tar = turbo::algorithm::recovery;
    memset(block_list_.data(), 0, sizeof(block_type) * capacity);
    for (capacity_type index = 0; index < capacity; ++index)
    {
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    if (free_list_.try_enqueue_copy(index) == free_list_type::producer::result::success)
	    {
		return tar::try_state::done;
	    }
	    else
	    {
		return tar::try_state::retry;
	    }
	});
    }
}

template <std::size_t block_size_c, template <class type_t> class allocator_t>
std::size_t block_pool<block_size_c, allocator_t>::get_byte_size() const
{
    return sizeof(block_type) * block_list_.size();
}

template <std::size_t block_size_c, template <class type_t> class allocator_t>
const void* block_pool<block_size_c, allocator_t>::get_base_address() const
{
    return block_list_.data();
}

template <std::size_t block_size_c, template <class type_t> class allocator_t>
std::pair<make_result, void*> block_pool<block_size_c, allocator_t>::allocate()
{
    namespace tar = turbo::algorithm::recovery;
    capacity_type reservation = 0U;
    make_result result = make_result::pool_full;
    tar::retry_with_random_backoff([&] () -> tar::try_state
    {
	switch (free_list_.try_dequeue_copy(reservation))
	{
	    case free_list_type::consumer::result::queue_empty:
	    {
		// no free blocks available
		result = make_result::pool_full;
		return tar::try_state::done;
	    }
	    case free_list_type::consumer::result::success:
	    {
		result = make_result::success;
		return tar::try_state::done;
	    }
	    default:
	    {
		return tar::try_state::retry;
	    }
	}
    });
    if (result == make_result::success)
    {
	return std::make_pair(result, &(block_list_[reservation]));
    }
    else
    {
	return std::make_pair(result, nullptr);
    }
}

template <std::size_t block_size_c, template <class type_t> class allocator_t>
void block_pool<block_size_c, allocator_t>::free(void* pointer)
{
    namespace tar = turbo::algorithm::recovery;
    try
    {
	std::size_t offset = static_cast<block_type*>(pointer) - &(block_list_[0]);
	if (offset < block_list_.size())
	{
	    tar::retry_with_random_backoff([&] () -> tar::try_state
	    {
		switch (free_list_.try_enqueue_copy(offset))
		{
		    case free_list_type::producer::result::queue_full:
		    {
			// log a warning?
			return tar::try_state::done;
		    }
		    case free_list_type::producer::result::success:
		    {
			return tar::try_state::done;
		    }
		    default:
		    {
			return tar::try_state::retry;
		    }
		}
	    });
	}
	// else log a warning?
    }
    catch (...)
    {
	// do nothing since destructor can't fail
    }
}

template <std::size_t block_size_c, template <class type_t> class allocator_t>
template <class value_t, class ...args_t>
std::pair<make_result, pool_unique_ptr<value_t>> block_pool<block_size_c, allocator_t>::make_unique(args_t&&... args)
{
    static_assert(sizeof(value_t) <= sizeof(block_type), "Requested value type is larger than the memory pool's block size");
    std::pair<make_result, void*> result = allocate();
    if (result.first == make_result::success)
    {
	return std::make_pair(
		result.first,
		pool_unique_ptr<value_t>(
			new (result.second) value_t(std::forward<args_t>(args)...),
			std::bind(&block_pool<block_size_c, allocator_t>::free, this, std::placeholders::_1)));
    }
    else
    {
	return std::make_pair(result.first, pool_unique_ptr<value_t>());
    }
}

template <std::size_t block_size_c, template <class type_t> class allocator_t>
template <class value_t, class ...args_t>
std::pair<make_result, std::shared_ptr<value_t>> block_pool<block_size_c, allocator_t>::make_shared(args_t&&... args)
{
    static_assert(sizeof(value_t) <= sizeof(block_type), "Requested value type is larger than the memory pool's block size");
    std::pair<make_result, void*> result = allocate();
    if (result.first == make_result::success)
    {
	return std::make_pair(
		result.first,
		std::shared_ptr<value_t>(
			new (result.second) value_t(std::forward<args_t>(args)...),
			std::bind(&block_pool<block_size_c, allocator_t>::free, this, std::placeholders::_1)));
    }
    else
    {
	return std::make_pair(result.first, std::shared_ptr<value_t>());
    }
}

template <class block_pool_t>
class node
{
public:
    enum class status : std::uint8_t
    {
	unused,
	initialising,
	ready
    };
    node();
private:
    std::atomic<status> guard;
    std::unique_ptr<block_pool_t> pool;
    std::unique_ptr<node> next;
};

template <template <class type_t> class allocator_t>
range_pool<allocator_t>::range_pool(capacity_type default_capacity, const std::vector<block_config>& config)
    :
	range_pool(default_capacity, config, 2U)
{ }


template <template <class type_t> class allocator_t>
range_pool<allocator_t>::range_pool(capacity_type default_capacity, const std::vector<block_config>& config, std::uint8_t step_factor)
    :
	default_capacity_(default_capacity),
	step_factor_(step_factor < 2U ? 2U : step_factor),
	smallest_block_(0U)
{
    std::vector<block_config> sanitised(sanitise(config, step_factor_));
    if (!sanitised.empty())
    {
	smallest_block_ = sanitised.cbegin()->block_size;
    }
}

template <template <class type_t> class allocator_t>
std::vector<block_config> range_pool<allocator_t>::sanitise(const std::vector<block_config>& config, std::uint8_t step_factor)
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

#endif
