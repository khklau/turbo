#ifndef TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP
#define TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP

#include <array>
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
    static constexpr std::array<std::size_t, 2U> node_sizes { sizeof(base), sizeof(floor) };
    static constexpr std::array<std::size_t, 2U> node_alignments { alignof(base), alignof(floor) };
    explicit emplacing_skiplist(typed_allocator_type& allocator);
    inline iterator begin()
    {
	return ground_.begin();
    }
    inline iterator end()
    {
	return ground_.end();
    }
    inline iterator cbegin()
    {
	return ground_.cbegin();
    }
    inline iterator cend()
    {
	return ground_.cend();
    }
    inline iterator rbegin()
    {
	return ground_.rbegin();
    }
    inline iterator rend()
    {
	return ground_.rend();
    }
    inline iterator crbegin()
    {
	return ground_.crbegin();
    }
    inline iterator crend()
    {
	return ground_.crend();
    }
    iterator search(const key_type& key);
private:
    typedef std::tuple<typename ground::iterator, typename ground::iterator> ground_region;
    typedef std::tuple<typename level::iterator, typename level::iterator> level_region;
    struct base
    {
	key_t key;
	value_t value;
    };
    struct floor
    {
	key_t key;
	std::weak_ptr<typename level::iterator::node_type> down;
	std::weak_ptr<typename ground::iterator::node_type> bottom;
    };
    ground_region search_ground(const key_type& key, const typename ground::iterator& iter);
    level_region search_level(const key_type& key, const typename level::iterator& iter);
    level_region search_tower(const key_type& key, std::size_t target_level);
    typed_allocator_type& allocator_;
    ground ground_;
    tower tower_;
};

} // namespace container
} // namespace turbo

#endif
