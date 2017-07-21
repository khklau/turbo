#ifndef TURBO_CONTAINER_CONCURRENT_LIST_HPP
#define TURBO_CONTAINER_CONCURRENT_LIST_HPP

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <memory>
#include <turbo/memory/tagged_ptr.hpp>
#include <turbo/memory/cstdlib_allocator.hpp>

namespace turbo {
namespace container {

template <class value_t>
using list_unique_ptr = std::unique_ptr<value_t, std::function<void (value_t*)>>;


template <class value_t, class typed_allocator_t = turbo::memory::cstdlib_typed_allocator>
class concurrent_list
{
private:
    struct node;
public:
    typedef value_t value_type;
    typedef typed_allocator_t typed_allocator_type;
    explicit concurrent_list(typed_allocator_type& allocator);
    static constexpr std::size_t node_size() { return sizeof(node); }
    static constexpr std::size_t node_alignment() { return alignof(node); }
    template <class... args_t>
    node* create_node(args_t&&... args);
private:
    enum class demand : std::uint8_t
    {
	wanted = 0U,
	for_deletion
    };
    struct node
    {
	template <class... args_t>
	node(args_t&&... args);
	value_t value;
	std::atomic<turbo::memory::tagged_ptr<node, demand>> next;
	std::atomic<turbo::memory::tagged_ptr<node, demand>> previous;
    };
    typed_allocator_type& allocator_;
    std::atomic<node*> list_;
};

} // namespace container
} // namespace turbo

#endif
