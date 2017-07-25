#ifndef TURBO_MEMORY_SLAB_ALLOCATOR_HXX
#define TURBO_MEMORY_SLAB_ALLOCATOR_HXX

#include <turbo/memory/slab_allocator.hpp>
#include <cstring>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/memory/alignment.hpp>
#include <turbo/memory/alignment.hxx>
#include <turbo/memory/block.hxx>
#include <turbo/toolset/extension.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>

namespace turbo {
namespace memory {

inline bool concurrent_sized_slab::in_configured_range(std::size_t value_size) const
{
    return value_size != 0U && find_block_bucket(calc_total_aligned_size(value_size, value_size, 1U)) < block_map_.size();
}

inline const block_list& concurrent_sized_slab::at(std::size_t size) const
{
    const std::size_t bucket = find_block_bucket(calc_total_aligned_size(size, size, 1U));
    if (TURBO_LIKELY(bucket < block_map_.size()))
    {
	return block_map_[bucket];
    }
    else
    {
	throw std::invalid_argument("concurrent_sized_slab::at - there is no block_list matching the given arguments");
    }
}

inline block_list& concurrent_sized_slab::at(std::size_t size)
{
    const std::size_t bucket = find_block_bucket(calc_total_aligned_size(size, size, 1U));
    if (TURBO_LIKELY(bucket < block_map_.size()))
    {
	return block_map_[bucket];
    }
    else
    {
	throw std::invalid_argument("concurrent_sized_slab::at - there is no block_list matching the given arguments");
    }
}

inline std::size_t concurrent_sized_slab::find_block_bucket(std::size_t allocation_size) const
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
std::pair<make_result, slab_unique_ptr<value_t>> concurrent_sized_slab::make_unique(args_t&&... args)
{
    value_t* result = allocate<value_t>();
    if (result != nullptr)
    {
	return std::make_pair(
		make_result::success,
		slab_unique_ptr<value_t>(
			new (result) value_t(std::forward<args_t>(args)...),
			std::bind(static_cast<void (concurrent_sized_slab::*)(value_t*)>(&concurrent_sized_slab::unmake<value_t>), this, std::placeholders::_1)));
    }
    else
    {
	return std::make_pair(make_result::slab_full, slab_unique_ptr<value_t>());
    }
}

template <class value_t, class... args_t>
std::pair<make_result, std::shared_ptr<value_t>> concurrent_sized_slab::make_shared(args_t&&... args)
{
    value_t* result = allocate<value_t>();
    if (result != nullptr)
    {
	return std::make_pair(
		make_result::success,
		std::shared_ptr<value_t>(
			new (result) value_t(std::forward<args_t>(args)...),
			std::bind(static_cast<void (concurrent_sized_slab::*)(value_t*)>(&concurrent_sized_slab::unmake<value_t>), this, std::placeholders::_1)));
    }
    else
    {
	return std::make_pair(make_result::slab_full, std::shared_ptr<value_t>());
    }
}

template <class value_t>
void concurrent_sized_slab::unmake(value_t* pointer)
{
    pointer->~value_t();
    deallocate(pointer);
}

} // namespace memory
} // namespace turbo

#endif
