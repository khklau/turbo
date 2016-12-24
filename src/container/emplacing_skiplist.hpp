#ifndef TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP
#define TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP

#include <functional>
#include <iterator>
#include <memory>
#include <tuple>
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
	record(const key_t& k, const value_t& v);
	key_t key;
	value_t value;
    };
    static constexpr std::array<std::size_t, 2U> node_sizes { sizeof(typename store::iterator::node_type), sizeof(typename floor::iterator::node_type) };
    static constexpr std::array<std::size_t, 2U> node_alignments { alignof(typename store::iterator::node_type), alignof(typename floor::iterator::node_type) };
    explicit emplacing_skiplist(typed_allocator_type& allocator);
    emplacing_skiplist(typed_allocator_type& allocator, std::size_t height_log_base);
    inline std::size_t size() const
    {
	return store_.size();
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
    struct room
    {
	room(const key_t& k, const std::shared_ptr<typename store::iterator::node_type>& b, std::shared_ptr<typename floor::iterator::node_type>& d);
	key_t key;
	std::weak_ptr<typename store::iterator::node_type> bottom;
	std::weak_ptr<typename floor::iterator::node_type> down;
    };
    store_region search(const key_type& key);
    store_region search_store(const key_type& key, const typename store::iterator& iter);
    floor_region search_floor(const key_type& key, const typename floor::iterator& iter);
    floor_region search_tower(const key_type& key, std::size_t target_floor);
    std::size_t chose_height() const;
    typed_allocator_type& allocator_;
    std::size_t height_log_base_;
    store store_;
    tower tower_;
};

} // namespace container
} // namespace turbo

#endif
