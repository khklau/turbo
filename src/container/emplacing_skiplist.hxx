#ifndef TURBO_CONTAINER_EMPLACING_SKIPLIST_HXX
#define TURBO_CONTAINER_EMPLACING_SKIPLIST_HXX

#include <turbo/container/emplacing_skiplist.hpp>
#include <cmath>
#include <chrono>
#include <random>
#include <turbo/container/emplacing_list.hxx>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {

template <class key_t, class value_t, class allocator_t, class compare_f>
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::emplacing_skiplist(typed_allocator_type& allocator)
    :
	emplacing_skiplist(allocator, 2U)
{ }

template <class key_t, class value_t, class allocator_t, class compare_f>
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::emplacing_skiplist(typed_allocator_type& allocator, std::uint16_t height_log_base)
    :
	allocator_(allocator),
	height_log_base_(height_log_base),
	store_(allocator_),
	tower_(allocator_)
{ }

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::iterator emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::find(const key_type& key)
{
    iterator target;
    std::tie(target, std::ignore) = search(key);
    if (target != store_.end() && compare_func()(target->key, key) && compare_func()(key, target->key))
    {
	return target;
    }
    else
    {
	return store_.end();
    }
}

template <class k, class v, class a, class c>
template <class key_arg_t, class... value_args_t>
std::tuple<typename emplacing_skiplist<k, v, a, c>::iterator, bool> emplacing_skiplist<k, v, a, c>::emplace(
	const key_arg_t& key_arg,
	value_args_t&&... value_args)
{
    const std::int64_t chosen_height = chose_height();
    if (0 <= chosen_height)
    {
	grow_tower(chosen_height);
    }
    const key_type key(key_arg);
    const typename floor::iterator empty;
    std::vector<trace> tower_trace(trace_tower(key));
    typename store::iterator nearest_record;
    typename store::iterator next_record;
    if (tower_trace.size() == 0U || tower_trace.back().nearest == empty)
    {
	// tower is empty or all keys in the tower are greater than the requested key
	std::tie(nearest_record, next_record) = search_store(key, store_.begin());
    }
    else
    {
	nearest_record = tower_trace.back().nearest->bottom.lock();
	std::tie(nearest_record, next_record) = search_store(key, nearest_record);
    }
    if (nearest_record != store_.end() && compare_func()(nearest_record->key, key) && compare_func()(key, nearest_record->key))
    {
	// record already exists
	return std::make_tuple(nearest_record, false);
    }
    else
    {
	iterator target = store_.emplace(next_record, key_arg, std::forward<value_args_t>(value_args)...);
	if (chosen_height < 0)
	{
	    return std::make_tuple(target, true);
	}
	std::weak_ptr<typename store::iterator::node_type> bottom_ptr(target.share());
	std::weak_ptr<typename floor::iterator::node_type> down_ptr;
	for (auto iter = tower_trace.rbegin(); iter != tower_trace.rend() && iter->id <= chosen_height; ++iter)
	{
	    if (iter->nearest != empty && compare_func()(iter->nearest->key, key) && compare_func()(key, iter->nearest->key))
	    {
		// room with requested key already exists
		down_ptr = iter->nearest.share();
	    }
	    else
	    {
		down_ptr = iter->floor->emplace(iter->next, key, bottom_ptr, down_ptr).share();
	    }
	}
	return std::make_tuple(target, true);
    }
}

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::store_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search(
	const key_type& key)
{
    const typename floor::iterator empty;
    std::vector<trace> tower_trace(trace_tower(key));
    // TODO: in concurrent skiplist bottom might be expired
    if (tower_trace.size() == 0U || tower_trace.back().nearest == empty)
    {
	// tower is empty or all keys in the tower are greater than the requested key
	return search_store(key, store_.begin());
    }
    else
    {
	typename store::iterator nearest_record = tower_trace.back().nearest->bottom.lock();
	return search_store(key, nearest_record);
    }
}

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::store_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search_store(
	const key_type& key,
	const typename store::iterator& start)
{
    const typename store::iterator empty;
    if (start == store_.end() || !start.is_valid())
    {
	return std::make_tuple(store_.end(), store_.end());
    }
    typename store::iterator current(start);
    typename store::iterator next(start);
    ++next;
    if (compare_func()(current->key, key) && compare_func()(key, current->key))
    {
	// start already points to the room we want
	return std::make_tuple(current, next);
    }
    else if (compare_func()(key, current->key))
    {
	// key is less than start
	return std::make_tuple(empty, current);
    }
    while (next != store_.end() && next.is_valid() && compare_func()(next->key, key))
    {
	current = next;
	++next;
    }
    return std::make_tuple(current, next);
}

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::floor_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search_floor(
	const key_type& key,
	const typename floor::iterator& start)
{
    const typename floor::iterator empty;
    if (start == empty || !start.is_valid())
    {
	return std::make_tuple(empty, empty);
    }
    typename floor::iterator current(start);
    typename floor::iterator next(start);
    ++next;
    if (compare_func()(current->key, key) && compare_func()(key, current->key))
    {
	// start already points to the room we want
	return std::make_tuple(current, next);
    }
    else if (compare_func()(key, current->key))
    {
	// key is less than start
	return std::make_tuple(empty, current);
    }
    while (next != empty && next.is_valid())
    {
	if (compare_func()(next->key, key))
	{
	    current = next;
	    ++next;
	}
	else
	{
	    break;
	}
    }
    return std::make_tuple(current, next);
}

template <class k, class v, class a, class c>
std::vector<typename emplacing_skiplist<k, v, a, c>::trace> emplacing_skiplist<k, v, a, c>::trace_tower(const key_type& key)
{
    const typename floor::iterator empty;
    std::vector<trace> result;
    typename floor::iterator above = empty;
    typename floor::iterator current;
    typename floor::iterator next;
    floor_id id = tower_.size() - 1U;
    for (typename tower::reverse_iterator tower_iter = tower_.rbegin(); tower_iter != tower_.rend(); --id, ++tower_iter)
    {
	if (above != empty)
	{
	    current = above->down.lock();
	}
	else
	{
	    current = tower_iter->begin();
	}
	std::tie(current, next) = search_floor(key, current);
	result.emplace_back(id, tower_iter, current, next);
	if (current != tower_iter->end())
	{
	    if (compare_func()(current->key, key) && compare_func()(key, current->key))
	    {
		// found room matching the key
		break;
	    }
	    else
	    {
		above = current;
	    }
	}
	else
	{
	    // all rooms on this floor have a key greater than the requested key
	    above = empty;
	}
    }
    return result;
}

template <class k, class v, class a, class c>
void emplacing_skiplist<k, v, a, c>::grow_tower(floor_id new_maximum)
{
    if (tower_.cbegin() == tower_.cend())
    {
	tower_.emplace_back(allocator_);
    }
    typename tower::iterator iter = tower_.begin();
    for (floor_id floor_counter = 0U; floor_counter < new_maximum; ++iter, ++floor_counter)
    {
	if (iter.is_last())
	{
	    tower_.emplace_back(allocator_);
	}
    }
}

template <class key_t, class value_t, class allocator_t, class compare_f>
std::int64_t emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::chose_height() const
{
    if (size() <= 2)
    {
	return -1;
    }
    std::int64_t ideal_height = static_cast<std::int64_t>(std::ceil(
	    std::log(static_cast<double>(size()) /
	    std::log(static_cast<double>(height_log_base_)))));
    std::uniform_int_distribution<std::int64_t> device(-1, ideal_height);
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    return device(generator);
}

template <class key_t, class value_t, class allocator_t, class compare_f>
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::record::record(const key_t& a_key, const value_t& a_value)
    :
	key(a_key),
	value(a_value)
{ }

template <class key_t, class value_t, class allocator_t, class compare_f>
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::room::room(
	const key_t& a_key,
	const std::weak_ptr<typename store::iterator::node_type>& a_bottom,
	const std::weak_ptr<typename floor::iterator::node_type>& a_down)
    :
	key(a_key),
	bottom(a_bottom),
	down(a_down)
{ }

template <class key_t, class value_t, class allocator_t, class compare_f>
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::trace::trace(
	const floor_id& a_id,
	const typename tower::reverse_iterator& a_floor,
	const typename floor::iterator& a_nearest,
	const typename floor::iterator& a_next)
    :
	id(a_id),
	floor(a_floor),
	nearest(a_nearest),
	next(a_next)
{ }

} // namespace container
} // namespace turbo

#endif

