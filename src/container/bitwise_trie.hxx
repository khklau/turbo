#ifndef TURBO_CONTAINER_BITWISE_TRIE_HXX
#define TURBO_CONTAINER_BITWISE_TRIE_HXX

#include <turbo/container/bitwise_trie.hpp>
#include <turbo/container/emplacing_list.hxx>

namespace turbo {
namespace container {

template <class k, class v, class a>
bitwise_trie<k, v, a>::bitwise_trie(allocator_type& allocator)
    :
	allocator_(allocator),
	size_(0U),
	root_(nullptr)
{
    for (auto&& index_ptr: leading_zero_index_)
    {
	index_ptr = nullptr;
    }
}

template <class k, class v, class a>
template <class key_arg_t, class... value_args_t>
bitwise_trie<k, v, a>::leaf::leaf(const key_arg_t& key_arg, value_args_t&&... value_args)
    :
	key(key_arg),
	value(std::forward<value_args_t>(value_args)...)
{ }

template <class k, class v, class a>
bitwise_trie<k, v, a>::branch::branch()
{
    for (auto&& child_ptr: child)
    {
	child_ptr = nullptr;
    }
}

template <class k, class v, class a>
template <class key_arg_t, class... value_args_t>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::create_leaf(const key_arg_t& key_arg, value_args_t&&... value_args)
{
    leaf* tmp = allocator_.template allocate<leaf>();
    if (tmp != nullptr)
    {
	new (tmp) leaf(key_arg, std::forward<value_args_t>(value_args)...);
	return tmp;
    }
    else
    {
	throw std::runtime_error("Out of memory");
    }
}

template <class k, class v, class a>
void bitwise_trie<k, v, a>::destroy_leaf(leaf* pointer)
{
    pointer->~leaf();
    allocator_.template deallocate<leaf>(pointer);
}

template <class k, class v, class a>
typename bitwise_trie<k, v, a>::branch* bitwise_trie<k, v, a>::create_branch()
{
    branch* tmp = allocator_.template allocate<branch>();
    if (tmp != nullptr)
    {
	new (tmp) branch();
	return tmp;
    }
    else
    {
	throw std::runtime_error("Out of memory");
    }
}

template <class k, class v, class a>
void bitwise_trie<k, v, a>::destroy_branch(branch* pointer)
{
    pointer->~branch();
    allocator_.template deallocate<branch>(pointer);
}

} // namespace container
} // namespace turbo

#endif
