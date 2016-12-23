#ifndef TURBO_CONTAINER_EMPLACING_SKIPLIST_HXX
#define TURBO_CONTAINER_EMPLACING_SKIPLIST_HXX

#include <turbo/container/emplacing_skiplist.hpp>
#include <turbo/container/emplacing_list.hxx>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {

template <class key_t, class value_t, class allocator_t, class compare_f>
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::emplacing_skiplist(typed_allocator_type& allocator)
    :
	allocator_(allocator),
	size_(0U),
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

template <class key_t, class value_t, class allocator_t, class compare_f>
template <class key_arg_t, class... value_args_t>
std::tuple<typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::iterator, bool> emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::emplace(
	const key_arg_t& key_arg,
	value_args_t&&... value_args)
{
    iterator nearest_record;
    iterator next_record;
    std::tie(nearest_record, next_record) = search(key_arg);
    key_t key(key_arg);
    if (nearest_record != store_.end() && compare_func()(nearest_record->key, key) && compare_func()(key, nearest_record->key))
    {
	return std::make_tuple(nearest_record, false);
    }
    else
    {
	iterator target = store_.emplace(next_record, key_arg, std::forward<value_args_t>(value_args)...);
	++size_;
	std::size_t chosen_height = chose_height();
	const typename floor::iterator empty;
	if (tower_.begin() == tower_.end())
	{
	    tower_.emplace_back(allocator_);
	}
	typename tower::iterator tower_iter = tower_.begin();
	typename floor::iterator current_room;
	typename floor::iterator next_room;
	std::shared_ptr<typename store::iterator::node_type> bottom_ptr(target.strong_share());
	std::shared_ptr<typename floor::iterator::node_type> down_ptr;
	// update the tower
	for (std::size_t floor_counter = 0U; floor_counter <= chosen_height && tower_iter != tower_.end(); ++floor_counter, ++tower_iter)
	{
	    if (tower_iter->cbegin() == tower_iter->cend())
	    {
		// floor is empty
		tower_iter->emplace_front(key, bottom_ptr, down_ptr);
		current_room = tower_iter->begin();
	    }
	    else
	    {
		current_room = tower_iter->begin();
		std::tie(current_room, next_room) = search_floor(key, current_room);
		if (current_room != empty)
		{
		    if (!(compare_func()(current_room->key, key) && compare_func()(key, current_room->key)))
		    {
			current_room = tower_iter->emplace(next_room, key, bottom_ptr, down_ptr);
		    }
		    // else the room for this key already exists on this floor so nothing to do
		}
	    }
	    down_ptr = current_room.strong_share();
	    if (tower_iter.is_last() && 0U < (chosen_height - floor_counter))
	    {
		// need to grow the tower
		tower_.emplace_back(allocator_);
	    }
	}
	return std::make_tuple(target, true);
    }
}

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::store_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search(const key_type& key)
{
    typename floor::iterator nearest_room;
    const typename floor::iterator empty;
    std::tie(nearest_room, std::ignore) = search_tower(key, 0U);
    // TODO: in concurrent skiplist bottom might be expired
    iterator nearest_record((nearest_room != empty)
	    ? nearest_room->bottom.lock()
	    : store_.begin());
    return search_store(key, nearest_record);
}

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::store_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search_store(
	const key_type& key,
	const typename store::iterator& start)
{
    if (start == store_.end() || !start.is_valid())
    {
	return std::make_tuple(store_.end(), store_.end());
    }
    typename store::iterator current(start);
    typename store::iterator next(current++);
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
    typename floor::iterator next(current++);
    while (next != empty && next.is_valid())
    {
	if (next->bottom.expired() || next->down.expired())
	{
	    // TODO: erase the tower
	    ++next;
	}
	else if (compare_func()(next->key, key))
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

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::floor_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search_tower(
	const key_type& key,
	std::size_t target_floor)
{
    if (tower_.cbegin() == tower_.cend())
    {
	return std::make_tuple(typename floor::iterator(), typename floor::iterator());
    }
    std::size_t floor_number = tower_.size() - 1U;
    typename tower::reverse_iterator tower_iter = tower_.rbegin();
    typename floor::iterator current = tower_iter->begin();
    typename floor::iterator next = tower_iter->end();
    while (target_floor < floor_number && tower_iter != tower_.rend())
    {
	--floor_number;
	++tower_iter;
	std::tie(current, next) = search_floor(key, current);
	// TODO: in concurrent skiplist down might be expired
	if (current != tower_iter->end())
	{
	    current = current->down.lock();
	}
	else
	{
	    current = tower_iter->begin();
	}
    }
    std::tie(current, next) = search_floor(key, current);
    return std::make_tuple(current, next);
}

template <class key_t, class value_t, class allocator_t, class compare_f>
std::size_t emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::chose_height() const
{
    return 1U;
}

template <class key_t, class value_t, class allocator_t, class compare_f>
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::record::record(const key_t& k, const value_t& v)
    :
	key(k),
	value(v)
{ }

template <class key_t, class value_t, class allocator_t, class compare_f>
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::room::room(
	const key_t& k,
	const std::shared_ptr<typename store::iterator::node_type>& b,
	std::shared_ptr<typename floor::iterator::node_type>& d)
    :
	key(k),
	bottom(b),
	down(d)
{ }

} // namespace container
} // namespace turbo

#endif

