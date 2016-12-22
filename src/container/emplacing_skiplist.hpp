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
private:
    struct base;
    struct floor;
    typedef emplacing_list<base, allocator_t> ground;
    typedef emplacing_list<floor, allocator_t> level;
    typedef emplacing_list<level, allocator_t> tower;
public:
    typedef key_t key_type;
    typedef value_t value_type;
    typedef compare_f compare_func;
    typedef allocator_t typed_allocator_type;
    typedef typename ground::const_iterator const_iterator;
    typedef typename ground::iterator iterator;
    typedef typename ground::const_reverse_iterator const_reverse_iterator;
    typedef typename ground::reverse_iterator reverse_iterator;
    static constexpr std::array<std::size_t, 2U> node_sizes { sizeof(typename ground::iterator::node_type), sizeof(typename level::iterator::node_type) };
    static constexpr std::array<std::size_t, 2U> node_alignments { alignof(typename ground::iterator::node_type), alignof(typename level::iterator::node_type) };
    explicit emplacing_skiplist(typed_allocator_type& allocator);
    inline iterator begin()
    {
	return ground_.begin();
    }
    inline iterator end()
    {
	return ground_.end();
    }
    inline const_iterator cbegin()
    {
	return ground_.cbegin();
    }
    inline const_iterator cend()
    {
	return ground_.cend();
    }
    inline reverse_iterator rbegin()
    {
	return ground_.rbegin();
    }
    inline reverse_iterator rend()
    {
	return ground_.rend();
    }
    inline const_reverse_iterator crbegin()
    {
	return ground_.crbegin();
    }
    inline const_reverse_iterator crend()
    {
	return ground_.crend();
    }
    iterator find(const key_type& key);
    template <class key_arg_t, class... value_args_t>
    std::tuple<iterator, bool> emplace(const key_arg_t& key_arg, value_args_t&&... value_args);
private:
    typedef std::tuple<typename ground::iterator, typename ground::iterator> ground_region;
    typedef std::tuple<typename level::iterator, typename level::iterator> level_region;
    struct base
    {
	base(const key_t& k, const value_t& v);
	key_t key;
	value_t value;
    };
    struct floor
    {
	floor(const key_t& k, const std::shared_ptr<typename ground::iterator::node_type>& b, std::shared_ptr<typename level::iterator::node_type>& d);
	key_t key;
	std::weak_ptr<typename ground::iterator::node_type> bottom;
	std::weak_ptr<typename level::iterator::node_type> down;
    };
    ground_region search(const key_type& key);
    ground_region search_ground(const key_type& key, const typename ground::iterator& iter);
    level_region search_level(const key_type& key, const typename level::iterator& iter);
    level_region search_tower(const key_type& key, std::size_t target_level);
    std::size_t chose_height() const;
    typed_allocator_type& allocator_;
    ground ground_;
    tower tower_;
};

} // namespace container
} // namespace turbo

#endif
