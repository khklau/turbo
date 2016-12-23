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
	ground_(allocator_),
	tower_(allocator_)
{ }

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::iterator emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::find(const key_type& key)
{
    iterator target;
    std::tie(target, std::ignore) = search(key);
    if (target != ground_.end() && compare_func()(target->key, key) && compare_func()(key, target->key))
    {
	return target;
    }
    else
    {
	return ground_.end();
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
    if (nearest_record != ground_.end() && compare_func()(nearest_record->key, key) && compare_func()(key, nearest_record->key))
    {
	return std::make_tuple(nearest_record, false);
    }
    else
    {
	iterator target = ground_.emplace(next_record, key_arg, std::forward<value_args_t>(value_args)...);
	std::size_t chosen_height = chose_height();
	const typename level::iterator empty;
	if (tower_.begin() == tower_.end())
	{
	    tower_.emplace_back(allocator_);
	}
	typename tower::iterator tower_iter = tower_.begin();
	typename level::iterator current_floor;
	typename level::iterator next_floor;
	std::shared_ptr<typename ground::iterator::node_type> bottom_ptr(target.strong_share());
	std::shared_ptr<typename level::iterator::node_type> down_ptr;
	// update the tower
	for (std::size_t level_counter = 0U; level_counter <= chosen_height && tower_iter != tower_.end(); ++level_counter, ++tower_iter)
	{
	    if (tower_iter->cbegin() == tower_iter->cend())
	    {
		// level is empty
		tower_iter->emplace_front(key, bottom_ptr, down_ptr);
		current_floor = tower_iter->begin();
	    }
	    else
	    {
		current_floor = tower_iter->begin();
		std::tie(current_floor, next_floor) = search_level(key, current_floor);
		if (current_floor != empty)
		{
		    if (!(compare_func()(current_floor->key, key) && compare_func()(key, current_floor->key)))
		    {
			current_floor = tower_iter->emplace(next_floor, key, bottom_ptr, down_ptr);
		    }
		    // else the floor for this key already exists on this level so nothing to do
		}
	    }
	    down_ptr = current_floor.strong_share();
	    if (tower_iter.is_last() && 0U < (chosen_height - level_counter))
	    {
		// need to grow the tower
		tower_.emplace_back(allocator_);
	    }
	}
	return std::make_tuple(target, true);
    }
}

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::ground_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search(const key_type& key)
{
    typename level::iterator nearest_floor;
    const typename level::iterator empty;
    std::tie(nearest_floor, std::ignore) = search_tower(key, 0U);
    // TODO: in concurrent skiplist bottom might be expired
    iterator nearest_record((nearest_floor != empty)
	    ? nearest_floor->bottom.lock()
	    : ground_.begin());
    return search_ground(key, nearest_record);
}

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::ground_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search_ground(
	const key_type& key,
	const typename ground::iterator& start)
{
    if (start == ground_.end() || !start.is_valid())
    {
	return std::make_tuple(ground_.end(), ground_.end());
    }
    typename ground::iterator current(start);
    typename ground::iterator next(current++);
    while (next != ground_.end() && next.is_valid() && compare_func()(next->key, key))
    {
	current = next;
	++next;
    }
    return std::make_tuple(current, next);
}

template <class key_t, class value_t, class allocator_t, class compare_f>
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::level_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search_level(
	const key_type& key,
	const typename level::iterator& start)
{
    const typename level::iterator empty;
    if (start == empty || !start.is_valid())
    {
	return std::make_tuple(empty, empty);
    }
    typename level::iterator current(start);
    typename level::iterator next(current++);
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
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::level_region emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search_tower(
	const key_type& key,
	std::size_t target_level)
{
    if (tower_.cbegin() == tower_.cend())
    {
	return std::make_tuple(typename level::iterator(), typename level::iterator());
    }
    std::size_t level_number = tower_.size() - 1U;
    typename tower::reverse_iterator tower_iter = tower_.rbegin();
    typename level::iterator current = tower_iter->begin();
    typename level::iterator next = tower_iter->end();
    while (target_level < level_number && tower_iter != tower_.rend())
    {
	--level_number;
	++tower_iter;
	std::tie(current, next) = search_level(key, current);
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
    std::tie(current, next) = search_level(key, current);
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
emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::floor::floor(
	const key_t& k,
	const std::shared_ptr<typename ground::iterator::node_type>& b,
	std::shared_ptr<typename level::iterator::node_type>& d)
    :
	key(k),
	bottom(b),
	down(d)
{ }

} // namespace container
} // namespace turbo

#endif

