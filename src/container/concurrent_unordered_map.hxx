#ifndef TURBO_CONTAINER_CONCURRENT_UNORDERED_MAP_HXX
#define TURBO_CONTAINER_CONCURRENT_UNORDERED_MAP_HXX

#include <turbo/container/concurrent_unordered_map.hpp>
#include <algorithm>
#include <turbo/threading/shared_lock.hpp>
#include <turbo/threading/shared_lock.hxx>

namespace turbo {
namespace container {

namespace detail {

constexpr std::uint64_t power_of_2_ceil(std::uint64_t input)
{
    return std::llround(std::pow(2U, std::ceil(std::log2(input))));
}

} // namespace detail

namespace tth = turbo::threading;

template <typename k, typename e, typename h, class a>
concurrent_unordered_map<k, e, h, a>::concurrent_unordered_map(
	allocator_type& allocator,
	std::size_t min_buckets,
	const hasher& hash_func)
    :
	allocator_(allocator),
	hash_func_(hash_func),
	group_(detail::power_of_2_ceil(min_buckets)),
	mutex_()
{ }

template <typename k, typename e, typename h, class a>
typename concurrent_unordered_map<k, e, h, a>::const_iterator concurrent_unordered_map<k, e, h, a>::find(const key_type& key) const
{
    std::size_t bucket_id = hasher(key) & (group_.size() - 1);
    tth::shared_lock<tth::shared_mutex> group_lock(mutex_);
    tth::shared_lock<tth::shared_mutex> storage_lock(group_[bucket_id].mutex());
    const_storage_iterator iter = group_[bucket_id].find(key);
    if (iter != group_[bucket_id].cend())
    {
	return const_iterator(group_, mutex_, bucket_id, iter, group_[bucket_id].mutex());
    }
    else
    {
	return cend();
    }
}

template <typename k, typename e, typename h, class a>
typename concurrent_unordered_map<k, e, h, a>::iterator concurrent_unordered_map<k, e, h, a>::find(const key_type& key)
{
    std::size_t bucket_id = hasher(key) & (group_.size() - 1);
    std::lock(mutex_, group_[bucket_id].mutex());
    storage_iterator iter = group_[bucket_id].find(key);
    if (iter != group_[bucket_id].end())
    {
	return iterator(group_, mutex_, bucket_id, iter, group_[bucket_id].mutex());
    }
    else
    {
	return end();
    }
}

template <typename k, typename e, typename h, class a>
template <class value_t, class storage_iterator_t, class group_iterator_t, class bound_t>
concurrent_unordered_map<k, e, h, a>::basic_iterator<value_t, storage_iterator_t, group_iterator_t, bound_t>::basic_iterator(
	bucket_group_type& bucket_group,
	tth::shared_mutex& group_mutex)
    :
	bucket_group_(&bucket_group),
	group_mutex_(&group_mutex),
	bucket_id_(bucket_group.size()),
	storage_iter_(),
	storage_mutex_(nullptr)
{ }

template <typename k, typename e, typename h, class a>
template <class value_t, class storage_iterator_t, class group_iterator_t, class bound_t>
concurrent_unordered_map<k, e, h, a>::basic_iterator<value_t, storage_iterator_t, group_iterator_t, bound_t>::basic_iterator(
	bucket_group_type& bucket_group,
	tth::shared_mutex& group_mutex,
	std::size_t bucket_id,
	const storage_iterator_t& storage_iter,
	tth::shared_mutex& storage_mutex)
    :
	bucket_group_(&bucket_group),
	group_mutex_(&group_mutex),
	bucket_id_(bucket_id),
	storage_iter_(storage_iter),
	storage_mutex_(&storage_mutex)
{ }

template <typename k, typename e, typename h, class a>
template <class v, class s, class g, class b>
concurrent_unordered_map<k, e, h, a>::basic_iterator<v, s, g, b>& concurrent_unordered_map<k, e, h, a>::basic_iterator<v, s, g, b>::operator++()
{
    if (storage_iter_ == storage_iterator_type() || bucket_id_ == bucket_group_->size() || !storage_mutex_)
    {
	return *this;
    }
    tth::shared_lock<tth::shared_mutex> group_lock(*group_mutex_);
    tth::shared_lock<tth::shared_mutex> storage_lock(*storage_mutex_);
    ++storage_iter_;
    if (storage_iter_ == bound::end(bucket_group_, bucket_id_))
    {
	++bucket_id_;
	if (bucket_id_ < bucket_group_->size())
	{
	    storage_iter_ = bound::begin(bucket_group_, bucket_id_);
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
