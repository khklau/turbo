#ifndef TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP
#define TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP

#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <tuple>
#include <vector>
#include <turbo/container/emplacing_list.hpp>

namespace turbo {
namespace container {

template <class key_t, class value_t, class allocator_t = turbo::memory::typed_allocator, class compare_f = std::less_equal<key_t>>
class emplacing_skiplist
{
public:
    struct record;
private:
    struct room;
    typedef emplacing_list<record, allocator_t> store;
    typedef emplacing_list<room, allocator_t> floor;
    typedef emplacing_list<floor, allocator_t> tower;
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
    static constexpr std::array<std::size_t, 3U> node_sizes
    {
	sizeof(typename store::iterator::node_type),
	sizeof(typename floor::iterator::node_type),
	sizeof(typename tower::iterator::node_type)
    };
    static constexpr std::array<std::size_t, 3U> node_alignments
    {
	alignof(typename store::iterator::node_type),
	alignof(typename floor::iterator::node_type),
	alignof(typename tower::iterator::node_type)
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
private:
    typedef std::tuple<typename store::iterator, typename store::iterator> store_region;
    typedef std::tuple<typename floor::iterator, typename floor::iterator> floor_region;
    typedef std::uint32_t floor_id;
    struct room
    {
	room(
		const key_t& a_key,
		const std::weak_ptr<typename store::iterator::node_type>& a_bottom,
		const std::weak_ptr<typename floor::iterator::node_type>& a_down);
	key_t key;
	std::weak_ptr<typename store::iterator::node_type> bottom;
	std::weak_ptr<typename floor::iterator::node_type> down;
    };
    struct trace
    {
	trace(
		const floor_id& a_id,
		const typename tower::reverse_iterator& a_floor,
		const typename floor::iterator& a_nearest,
		const typename floor::iterator& a_next);
	floor_id id;
	typename tower::reverse_iterator floor;
	typename floor::iterator nearest;
	typename floor::iterator next;
    };
    store_region search(const key_type& key);
    store_region search_store(const key_type& key, const typename store::iterator& iter);
    floor_region search_floor(const key_type& key, const typename floor::iterator& iter);
    floor_region search_tower(const key_type& key, floor_id target_floor);
    std::vector<trace> trace_tower(const key_type& key);
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
