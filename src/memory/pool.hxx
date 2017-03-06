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

inline std::size_t pool::find_block_bucket(std::size_t allocation_size) const
{
    std::size_t allocation_exponent = std::llround(std::ceil(std::log2(allocation_size)));
    if (allocation_size == 0U || allocation_exponent < smallest_block_exponent_)
    {
	// to prevent underflow error
	return 0U;
    }
    else
    {
	return allocation_exponent - smallest_block_exponent_;
    }
}

template <class value_t, class ...args_t>
std::pair<make_result, pool_unique_ptr<value_t>> pool::make_unique(args_t&&... args)
{
    value_t* result = allocate<value_t>();
    if (result != nullptr)
    {
	return std::make_pair(
		make_result::success,
		pool_unique_ptr<value_t>(
			new (result) value_t(std::forward<args_t>(args)...),
			std::bind(static_cast<void (pool::*)(value_t*)>(&pool::unmake<value_t>), this, std::placeholders::_1)));
    }
    else
    {
	return std::make_pair(make_result::pool_full, pool_unique_ptr<value_t>());
    }
}

template <class value_t, class... args_t>
std::pair<make_result, std::shared_ptr<value_t>> pool::make_shared(args_t&&... args)
{
    value_t* result = allocate<value_t>();
    if (result != nullptr)
    {
	return std::make_pair(
		make_result::success,
		std::shared_ptr<value_t>(
			new (result) value_t(std::forward<args_t>(args)...),
			std::bind(static_cast<void (pool::*)(value_t*)>(&pool::unmake<value_t>), this, std::placeholders::_1)));
    }
    else
    {
	return std::make_pair(make_result::pool_full, std::shared_ptr<value_t>());
    }
}

template <class value_t>
void pool::unmake(value_t* pointer)
{
    pointer->~value_t();
    deallocate(pointer);
}

} // namespace memory
} // namespace turbo

#endif
