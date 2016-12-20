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
typename emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::iterator emplacing_skiplist<key_t, value_t, allocator_t, compare_f>::search(const key_type& key)
{
    typename level::iterator nearest_floor;
    typename level::iterator empty;
    std::tie(nearest_floor, std::ignore) = search_tower(key, 0U);
    // TODO: in concurrent skiplist bottom might be expired
    iterator nearest_base((nearest_floor != empty)
	    ? nearest_floor->bottom.lock()
	    : ground_.begin());
    iterator target;
    std::tie(target, std::ignore) = search_ground(key, nearest_base);
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
    typename level::iterator empty;
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

} // namespace container
} // namespace turbo

#endif

