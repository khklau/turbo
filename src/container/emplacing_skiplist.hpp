#ifndef TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP
#define TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP

#include <cstdint>
#include <functional>
#include <iterator>
#include <tuple>
#include <vector>
#include <turbo/container/emplacing_list.hpp>
#include <turbo/memory/cstdlib_allocator.hpp>

namespace turbo {
namespace container {

template <class key_t, class value_t, class allocator_t = turbo::memory::cstdlib_typed_allocator, class compare_f = std::less_equal<key_t>>
class emplacing_skiplist_tester;

template <class key_t, class value_t, class allocator_t = turbo::memory::cstdlib_typed_allocator, class compare_f = std::less_equal<key_t>>
class emplacing_skiplist
{
public:
    struct record;
private:
    struct room;
    typedef emplacing_list<record, allocator_t> store;
    typedef emplacing_list<room, allocator_t> floor;
    typedef emplacing_list<floor, allocator_t> tower;
    typedef typename store::iterator store_iterator;
    typedef typename store::reverse_iterator store_reverse_iterator;
    typedef typename floor::iterator floor_iterator;
    typedef typename floor::reverse_iterator floor_reverse_iterator;
    typedef typename tower::iterator tower_iterator;
    typedef typename tower::reverse_iterator tower_reverse_iterator;
public:
    typedef key_t key_type;
    typedef value_t value_type;
    typedef compare_f compare_func;
    typedef allocator_t typed_allocator_type;
    typedef typename store::const_iterator const_iterator;
    typedef typename store::iterator iterator;
    typedef typename store::const_reverse_iterator const_reverse_iterator;
    typedef typename store::reverse_iterator reverse_iterator;
    struct record
    {
	record(const key_t& a_key, const value_t& a_value);
	key_t key;
	value_t value;
    };
    friend class emplacing_skiplist_tester<key_t, value_t, allocator_t, compare_f>;
    static constexpr std::array<std::size_t, 3U> node_sizes
    {
	sizeof(typename store_iterator::node_type),
	sizeof(typename floor_iterator::node_type),
	sizeof(typename tower_iterator::node_type)
    };
    static constexpr std::array<std::size_t, 3U> node_alignments
    {
	alignof(typename store_iterator::node_type),
	alignof(typename floor_iterator::node_type),
	alignof(typename tower_iterator::node_type)
    };
    explicit emplacing_skiplist(typed_allocator_type& allocator);
    emplacing_skiplist(typed_allocator_type& allocator, std::uint16_t height_log_base);
    inline std::size_t size() const
    {
	return store_.size();
    }
    inline std::size_t height() const
    {
	return tower_.size();
    }
    inline iterator begin()
    {
	return store_.begin();
    }
    inline iterator end()
    {
	return store_.end();
    }
    inline const_iterator cbegin()
    {
	return store_.cbegin();
    }
    inline const_iterator cend()
    {
	return store_.cend();
    }
    inline reverse_iterator rbegin()
    {
	return store_.rbegin();
    }
    inline reverse_iterator rend()
    {
	return store_.rend();
    }
    inline const_reverse_iterator crbegin()
    {
	return store_.crbegin();
    }
    inline const_reverse_iterator crend()
    {
	return store_.crend();
    }
    iterator find(const key_type& key);
    template <class key_arg_t, class... value_args_t>
    std::tuple<iterator, bool> emplace(const key_arg_t& key_arg, value_args_t&&... value_args);
    iterator erase(const key_type& key);
private:
    typedef std::tuple<store_iterator, store_iterator> store_region;
    typedef std::tuple<floor_iterator, floor_iterator> floor_region;
    typedef std::uint32_t floor_id;
    struct room
    {
	room(
		const key_t& a_key,
		typename store_iterator::node_type* a_bottom,
		typename floor_iterator::node_type* a_down);
	key_t key;
	typename store_iterator::node_type* bottom;
	typename floor_iterator::node_type* down;
    };
    struct trace
    {
	trace(
		const floor_id& a_id,
		const tower_reverse_iterator& a_floor,
		const floor_iterator& a_nearest,
		const floor_iterator& a_next);
	floor_id id;
	tower_reverse_iterator floor;
	floor_iterator nearest;
	floor_iterator next;
    };
    enum class trace_depth
    {
	first_match,
	all_matches
    };
    template <class key_arg_t, class... value_args_t>
    std::tuple<iterator, bool> emplace(std::int64_t chosen_height, const key_arg_t& key_arg, value_args_t&&... value_args);
    store_region search(const key_type& key);
    store_region search_store(const key_type& key, const store_iterator& iter);
    floor_region search_floor(const key_type& key, const floor_iterator& iter);
    std::vector<trace> trace_tower(const key_type& key, const trace_depth& depth);
    inline std::vector<trace> trace_tower(const key_type& key)
    {
	return trace_tower(key, trace_depth::first_match);
    }
    void grow_tower(floor_id new_maximum);
    std::int64_t chose_height() const;
    typed_allocator_type& allocator_;
    std::uint16_t height_log_base_;
    store store_;
    tower tower_;
};

} // namespace container
} // namespace turbo

#endif
