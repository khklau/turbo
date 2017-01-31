#ifndef TURBO_CONTAINER_BITWISE_TRIE_HXX
#define TURBO_CONTAINER_BITWISE_TRIE_HXX

#include <turbo/container/bitwise_trie.hpp>
#include <turbo/container/invalid_dereference_error.hpp>

namespace turbo {
namespace container {

namespace bitwise_trie_iterator {

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward() noexcept
    :
	pointer_(nullptr)
{ }

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward(node_type* pointer) noexcept
    :
	pointer_(pointer)
{ }

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward(const basic_forward& other)
    :
	pointer_(other.pointer_)
{ }

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward(basic_forward&& other)
    :
	pointer_(std::move(other.pointer_))
{
    other.pointer_ = nullptr;
}

template <class value_t, class node_t>
template <class other_value_t>
basic_forward<value_t, node_t>::basic_forward(const basic_forward<other_value_t, node_t>& other)
    :
	pointer_(other.ptr())
{ }

template <class value_t, class node_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator=(const basic_forward& other)
{
    if (this != &other)
    {
	pointer_ = other.pointer_;
    }
    return *this;
}

template <class value_t, class node_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator=(basic_forward&& other)
{
    if (this != &other)
    {
	pointer_ = std::move(other.pointer_);
	other.pointer_ = nullptr;
    }
    return *this;
}

template <class value_t, class node_t>
template <class other_value_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator=(const basic_forward<other_value_t, node_t>& other)
{
    pointer_ = other.ptr();
    return *this;
}

template <class value_t, class node_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator=(node_type* other)
{
    pointer_ = other;
    return *this;
}

template <class value_t, class node_t>
bool basic_forward<value_t, node_t>::operator==(const basic_forward& other) const
{
    return pointer_ == other.pointer_;
}

template <class value_t, class node_t>
value_t& basic_forward<value_t, node_t>::operator*()
{
    if (pointer_ != nullptr)
    {
	return pointer_->value;
    }
    else
    {
	throw invalid_dereference_error("cannot dereference invalid bitwise_trie iterator");
    }
}

template <class value_t, class node_t>
value_t* basic_forward<value_t, node_t>::operator->()
{
    if (pointer_ != nullptr)
    {
	return &(pointer_->value);
    }
    else
    {
	throw invalid_dereference_error("cannot dereference bitwise_trie iterator");
    }
}

template <class value_t, class node_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator++()
{
    // TODO
    return *this;
}

template <class value_t, class node_t>
basic_forward<value_t, node_t> basic_forward<value_t, node_t>::operator++(int)
{
    basic_forward<value_t, node_t> tmp = *this;
    ++(*this);
    return tmp;
}

template <class value_t, class node_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator--()
{
    // TODO
    return *this;
}

template <class value_t, class node_t>
basic_forward<value_t, node_t> basic_forward<value_t, node_t>::operator--(int)
{
    basic_forward<value_t, node_t> tmp = *this;
    --(*this);
    return tmp;
}

} // namespace bitwise_trie_iterator

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
bitwise_trie<k, v, a>::record::record(const key_arg_t& key_arg, value_args_t&&... value_args)
    :
	key(key_arg),
	value(std::forward<value_args_t>(value_args)...)
{ }

template <class k, class v, class a>
template <class key_arg_t, class... value_args_t>
bitwise_trie<k, v, a>::leaf::leaf(branch* a_branch, const key_arg_t& key_arg, value_args_t&&... value_args)
    :
	parent(a_branch),
	value(std::forward<key_arg_t>(key_arg), std::forward<value_args_t>(value_args)...)
{ }

template <class k, class v, class a>
bitwise_trie<k, v, a>::branch::branch()
{
    for (auto&& child_ptr: children)
    {
	child_ptr = branch_ptr();
    }
}

template <class k, class v, class a>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::min()
{
    if (root_ == nullptr)
    {
	return nullptr;
    }
    branch_ptr current(root_);
    while (!current.is_empty())
    {
	auto child_iter = current.get_ptr()->children.begin();
	while (child_iter != current.get_ptr()->children.end())
	{
	    if (!child_iter->is_empty() && child_iter->get_tag() == child_type::leaf)
	    {
		return static_cast<leaf*>(static_cast<void*>(child_iter->get_ptr()));
	    }
	    else if (!child_iter->is_empty() && child_iter->get_tag() == child_type::branch)
	    {
		current = *child_iter;
		break;
	    }
	}
	if (child_iter == current.get_ptr()->children.end())
	{
	    // every branch should have at least 1 child so this means the trie is invalid
	    throw invalid_bitwise_trie_error("branch has no children");
	}
    }
    return nullptr;
}

template <class k, class v, class a>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::max()
{
    if (root_ == nullptr)
    {
	return nullptr;
    }
    branch_ptr current(root_);
    while (!current.is_empty())
    {
	auto child_iter = current.get_ptr()->children.rbegin();
	while (child_iter != current.get_ptr()->children.rend())
	{
	    if (!child_iter->is_empty() && child_iter->get_tag() == child_type::leaf)
	    {
		return static_cast<leaf*>(static_cast<void*>(child_iter->get_ptr()));
	    }
	    else if (!child_iter->is_empty() && child_iter->get_tag() == child_type::branch)
	    {
		current = *child_iter;
		break;
	    }
	}
	if (child_iter == current.get_ptr()->children.rend())
	{
	    // every branch should have at least 1 child so this means the trie is invalid
	    throw invalid_bitwise_trie_error("branch has no children");
	}
    }
    return nullptr;
}

template <class k, class v, class a>
template <class key_arg_t, class... value_args_t>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::create_leaf(
	branch* a_branch,
	const key_arg_t& key_arg,
	value_args_t&&... value_args)
{
    leaf* tmp = allocator_.template allocate<leaf>();
    if (tmp != nullptr)
    {
	new (tmp) leaf(a_branch, key_arg, std::forward<value_args_t>(value_args)...);
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
