#ifndef TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP
#define TURBO_CONTAINER_EMPLACING_SKIPLIST_HPP

#include <array>
#include <memory>
#include <turbo/container/emplacing_list.hpp>

namespace turbo {
namespace container {

template <class key_t, class value_t, class typed_allocator_t = turbo::memory::typed_allocator>
class emplacing_skiplist
{
private:
    struct ground;
    struct floor;
public:
    typedef key_t key_type;
    typedef value_t value_type;
    typedef typed_allocator_t typed_allocator_type;
    static constexpr std::array<std::size_t, 2U> node_sizes { sizeof(ground), sizeof(floor) };
    static constexpr std::array<std::size_t, 2U> node_alignments { alignof(ground), alignof(floor) };
    explicit emplacing_skiplist(typed_allocator_type& allocator);
private:
    struct ground
    {
	key_t key;
	value_t value;
    };
    struct floor
    {
	key_t key;
	std::weak_ptr<floor> down;
	std::weak_ptr<ground> bottom;
    };
    typed_allocator_type& allocator_;
    emplacing_list<ground, typed_allocator_type> base_;
    emplacing_list<emplacing_list<floor, typed_allocator_type>, typed_allocator_type> tower_;
};

} // namespace container
} // namespace turbo

#endif
