#ifndef TURBO_CONTAINER_CONCURRENT_UNORDERED_MAP_HXX
#define TURBO_CONTAINER_CONCURRENT_UNORDERED_MAP_HXX

#include <turbo/container/concurrent_unordered_map.hpp>
#include <algorithm>
#include <turbo/threading/shared_lock.hpp>
#include <turbo/threading/shared_lock.hxx>

namespace turbo {
namespace container {

namespace tth = turbo::threading;

template <typename k, typename e, typename h, class a>
concurrent_unordered_map<k, e, h, a>::concurrent_unordered_map(
	allocator_type& allocator,
	std::size_t min_buckets,
	const hasher& hash_func)
    :
	allocator_(allocator),
	hash_func_(hash_func),
	group_(min_buckets)
{ }

template <typename k, typename e, typename h, class a>
template <class value_t, class storage_iterator_t, class group_iterator_t, class bound_t>
concurrent_unordered_map<k, e, h, a>::basic_iterator<value_t, storage_iterator_t, group_iterator_t, bound_t>::basic_iterator(
	const group_iterator_t& group_end,
	const group_iterator_t& group_iter)
    :
	group_end_(group_end),
	group_iter_(group_iter),
	storage_iter_(),
	storage_mutex_(nullptr)
{ }

template <typename k, typename e, typename h, class a>
template <class value_t, class storage_iterator_t, class group_iterator_t, class bound_t>
concurrent_unordered_map<k, e, h, a>::basic_iterator<value_t, storage_iterator_t, group_iterator_t, bound_t>::basic_iterator(
	const group_iterator_t& group_end,
	const group_iterator_t& group_iter,
	const storage_iterator_t& storage_iter,
	tth::shared_mutex& storage_mutex)
    :
	group_end_(group_end),
	group_iter_(group_iter),
	storage_iter_(storage_iter),
	storage_mutex_(&storage_mutex)
{ }

template <typename k, typename e, typename h, class a>
template <class v, class s, class g, class b>
concurrent_unordered_map<k, e, h, a>::basic_iterator<v, s, g, b>& concurrent_unordered_map<k, e, h, a>::basic_iterator<v, s, g, b>::operator++()
{
    if (storage_iter_ == storage_iterator_type() || group_iter_ == group_end_ || !storage_mutex_)
    {
	return *this;
    }
    tth::shared_lock<tth::shared_mutex> lock(*storage_mutex_);
    ++storage_iter_;
    if (storage_iter_ == bound::end(group_iter_, group_end_))
    {
	++group_iter_;
	if (group_iter_ != group_end_)
	{
	    storage_iter_ = bound::begin(group_iter_, group_end_);
	}
	else
	{
	    storage_iter_ = storage_iterator_type();
	}
    }
    return *this;
}

template <typename k, typename e, typename h, class a>
template <class v, class s, class g, class b>
concurrent_unordered_map<k, e, h, a>::basic_iterator<v, s, g, b>& concurrent_unordered_map<k, e, h, a>::basic_iterator<v, s, g, b>::operator++(int)
{
    basic_iterator<v, s, g, b> tmp(*this);
    ++(*this);
    return tmp;
}

template <typename k, typename e, typename h, class a>
typename concurrent_unordered_map<k, e, h, a>::const_storage_iterator concurrent_unordered_map<k, e, h, a>::bucket::find(const key_type& key) const
{
    return std::find_if(storage_.begin(), storage_.end(), [&](const value_type& value) -> bool
    {
	return key == value.first;
    });
}

template <typename k, typename e, typename h, class a>
typename concurrent_unordered_map<k, e, h, a>::storage_iterator concurrent_unordered_map<k, e, h, a>::bucket::find(const key_type& key)
{
    return std::find_if(storage_.begin(), storage_.end(), [&](const value_type& value) -> bool
    {
	return key == value.first;
    });
}

template <typename k, typename e, typename h, class a>
typename concurrent_unordered_map<k, e, h, a>::storage_iterator concurrent_unordered_map<k, e, h, a>::bucket::erase(const_storage_iterator& position)
{
    auto source_iter = storage_.erase(position);
    std::move(source_iter, storage_.end(), position);
}

template <typename k, typename e, typename h, class a>
typename concurrent_unordered_map<k, e, h, a>::storage_iterator concurrent_unordered_map<k, e, h, a>::bucket::push_back(value_type&& value)
{
    storage_.push_back(std::move(value));
    return (storage_.rbegin() + 1).base();
}

} // namespace container
} // namespace turbo

#endif
