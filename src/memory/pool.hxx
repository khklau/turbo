#ifndef TURBO_MEMORY_POOL_HXX
#define TURBO_MEMORY_POOL_HXX

#include <turbo/memory/pool.hpp>
#include <cstring>
#include <limits>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>

namespace turbo {
namespace memory {

template <std::uint32_t block_size_c, template <class type_t> class allocator_t>
block_pool<block_size_c, allocator_t>::block_pool(std::uint32_t capacity, uint16_t user_limit)
    :
	free_list_(capacity, user_limit),
	block_list_(capacity)
{
    namespace tar = turbo::algorithm::recovery;
    memset(block_list_.data(), 0, sizeof(block_type) * capacity);
    for (std::uint32_t index = 0; index < capacity; ++index)
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

template <std::uint32_t block_size_c, template <class type_t> class allocator_t>
template <class value_t, class ...args_t>
std::pair<make_result, pool_unique_ptr<value_t>> block_pool<block_size_c, allocator_t>::make_unique(args_t&&... args)
{
    // TODO: static_assert that block_type is large enough for value_t
    namespace tar = turbo::algorithm::recovery;
    std::uint32_t reservation = 0U;
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
	return std::make_pair(
		result,
		pool_unique_ptr<value_t>(
			new (&(block_list_[reservation])) value_t(std::forward<args_t>(args)...),
			[&] (value_t* pointer) -> void
			{
			    try
			    {
				std::size_t offset = static_cast<block_type*>(static_cast<void*>(pointer)) - &(block_list_[0]);
				if (offset < block_list_.size())
				{
				    tar::retry_with_random_backoff([&] () -> tar::try_state
				    {
					switch (free_list_.try_enqueue_copy(reservation))
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
			}));
    }
    else
    {
	return std::make_pair(result, pool_unique_ptr<value_t>());
    }
}

} // namespace memory
} // namespace turbo

#endif
