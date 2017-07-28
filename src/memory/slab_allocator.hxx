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

bool concurrent_sized_slab::in_configured_range(std::size_t value_size) const
{
    return value_size != 0U && find_block_bucket(calc_total_aligned_size(value_size, value_size, 1U)) < block_map_.size();
}

const block_list& concurrent_sized_slab::at(std::size_t size) const
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

block_list& concurrent_sized_slab::at(std::size_t size)
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

std::size_t concurrent_sized_slab::find_block_bucket(std::size_t allocation_size) const
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
			std::bind(
				static_cast<void (concurrent_sized_slab::*)(value_t*)>(&concurrent_sized_slab::unmake<value_t>),
				this,
				std::placeholders::_1)));
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
			std::bind(
				static_cast<void (concurrent_sized_slab::*)(value_t*)>(&concurrent_sized_slab::unmake<value_t>),
				this,
				std::placeholders::_1)));
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

template <class value_t>
std::size_t default_type_index_policy::get_index()
{
    static_assert(sizeof(value_t) == 0U, "users of concurrent_typed_slab need to implement their own type_index_policy");
    return 0U;
};

template <class t>
concurrent_typed_slab<t>::concurrent_typed_slab(const std::vector<block_config>& config)
    :
	block_map_(config.cbegin(), config.cend())
{ }

template <class t>
concurrent_typed_slab<t>::concurrent_typed_slab(const concurrent_typed_slab& other)
    :
	block_map_(other.block_map_)
{ }

template <class t>
concurrent_typed_slab<t>& concurrent_typed_slab<t>::operator=(const concurrent_typed_slab<t>& other)
{
    if (this != &other
	    && this->block_map_.size() == other.block_map_.size())
    {
	std::copy_n(other.block_map_.cbegin(), this->block_map_.size(), this->block_map_.begin());
    }
    return *this;
}

template <class t>
bool concurrent_typed_slab<t>::operator==(const concurrent_typed_slab<t>& other) const
{
    return this->block_map_ == other.block_map_;
}

template <class t>
template <class value_t, class ...args_t>
std::pair<make_result, slab_unique_ptr<value_t>> concurrent_typed_slab<t>::make_unique(args_t&&... args)
{
    value_t* result = allocate<value_t>();
    if (result != nullptr)
    {
	return std::make_pair(
		make_result::success,
		slab_unique_ptr<value_t>(
			new (result) value_t(std::forward<args_t>(args)...),
			std::bind(
				static_cast<void (concurrent_typed_slab<t>::*)(value_t*)>(&concurrent_typed_slab<t>::unmake<value_t>),
				this,
				std::placeholders::_1)));
    }
    else
    {
	return std::make_pair(make_result::slab_full, slab_unique_ptr<value_t>());
    }
}

template <class t>
template <class value_t, class... args_t>
std::pair<make_result, std::shared_ptr<value_t>> concurrent_typed_slab<t>::make_shared(args_t&&... args)
{
    value_t* result = allocate<value_t>();
    if (result != nullptr)
    {
	return std::make_pair(
		make_result::success,
		std::shared_ptr<value_t>(
			new (result) value_t(std::forward<args_t>(args)...),
			std::bind(
				static_cast<void (concurrent_typed_slab<t>::*)(value_t*)>(&concurrent_typed_slab<t>::unmake<value_t>),
				this,
				std::placeholders::_1)));
    }
    else
    {
	return std::make_pair(make_result::slab_full, std::shared_ptr<value_t>());
    }
}

template <class t>
template <class value_t>
void concurrent_typed_slab<t>::unmake(value_t* pointer)
{
    pointer->~value_t();
    deallocate(pointer);
}

template <class t>
template <class value_t>
const block_list& concurrent_typed_slab<t>::at() const
{
    const std::size_t bucket = type_index_policy::template get_index<value_t>();
    if (TURBO_LIKELY(bucket < block_map_.size()))
    {
	return block_map_[bucket];
    }
    else
    {
	throw std::invalid_argument("concurrent_typed_slab::at - there is no block_list matching the given arguments");
    }
}

template <class t>
template <class value_t>
block_list& concurrent_typed_slab<t>::at()
{
    const std::size_t bucket = type_index_policy::template get_index<value_t>();
    if (TURBO_LIKELY(bucket < block_map_.size()))
    {
	return block_map_[bucket];
    }
    else
    {
	throw std::invalid_argument("concurrent_typed_slab::at - there is no block_list matching the given arguments");
    }
}

template <class t>
template <class value_t>
value_t* concurrent_typed_slab<t>::allocate(const value_t*)
{
    const std::size_t bucket = type_index_policy::template get_index<value_t>();
    if (TURBO_LIKELY(bucket < block_map_.size()))
    {
	return static_cast<value_t*>(block_map_[bucket].allocate());
    }
    else
    {
	return nullptr;
    }
}

template <class t>
template <class value_t>
void concurrent_typed_slab<t>::deallocate(value_t* pointer)
{
    const std::size_t bucket = type_index_policy::template get_index<value_t>();
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

} // namespace memory
} // namespace turbo

#endif
