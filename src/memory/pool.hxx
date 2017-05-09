#ifndef TURBO_MEMORY_POOL_HXX
#define TURBO_MEMORY_POOL_HXX

#include <turbo/memory/pool.hpp>
#include <cstring>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/memory/alignment.hpp>
#include <turbo/memory/alignment.hxx>
#include <turbo/toolset/extension.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>

namespace turbo {
namespace memory {

template <class b, class n>
block_list::basic_iterator<b, n>::basic_iterator()
    :
	pointer_(nullptr)
{ }

template <class b, class n>
block_list::basic_iterator<b, n>::basic_iterator(typename block_list::basic_iterator<b, n>::node_type* pointer)
    :
	pointer_(pointer)
{ }

template <class b, class n>
block_list::basic_iterator<b, n>::basic_iterator(const basic_iterator& other)
    :
	pointer_(other.pointer_)
{ }

template <class b, class n>
block_list::basic_iterator<b, n>& block_list::basic_iterator<b, n>::operator=(const basic_iterator& other)
{
    if (TURBO_LIKELY(this != &other))
    {
	pointer_ = other.pointer_;
    }
    return *this;
}

template <class b, class n>
bool block_list::basic_iterator<b, n>::operator==(const basic_iterator& other) const
{
    return pointer_ == other.pointer_;
}

template <class b, class n>
block_list::basic_iterator<b, n>& block_list::basic_iterator<b, n>::operator++()
{
    if (TURBO_LIKELY(is_valid()))
    {
	node* next = pointer_->get_next().load(std::memory_order_acquire);
	pointer_ = next;
    }
    return *this;
}

template <class b, class n>
typename block_list::basic_iterator<b, n> block_list::basic_iterator<b, n>::operator++(int)
{
    if (TURBO_LIKELY(is_valid()))
    {
	basic_iterator<b, n> tmp = *this;
	++(*this);
	return tmp;
    }
    else
    {
	return *this;
    }
}

inline const block_list& pool::get_block_list(
	std::size_t value_size,
	std::size_t value_alignment,
	capacity_type quantity) const
{
    const std::size_t bucket = find_block_bucket(calc_total_aligned_size(value_size, value_alignment, quantity));
    if (TURBO_LIKELY(bucket < block_map_.size()))
    {
	return block_map_[bucket];
    }
    else
    {
	throw std::invalid_argument("pool::get_block_list - there is no block_list matching the given arguments");
    }
}

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
