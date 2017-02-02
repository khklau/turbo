#ifndef TURBO_CONTAINER_BITWISE_TRIE_HXX
#define TURBO_CONTAINER_BITWISE_TRIE_HXX

#include <turbo/container/bitwise_trie.hpp>
#include <turbo/container/invalid_dereference_error.hpp>

namespace turbo {
namespace container {

namespace bitwise_trie_iterator {

template <class k, class v, class n>
basic_forward<k, v, n>::basic_forward() noexcept
    :
	pointer_(nullptr)
{ }

template <class k, class v, class n>
basic_forward<k, v, n>::basic_forward(node_type* pointer) noexcept
    :
	pointer_(pointer)
{ }

template <class k, class v, class n>
basic_forward<k, v, n>::basic_forward(const basic_forward& other)
    :
	pointer_(other.pointer_)
{ }

template <class k, class v, class n>
basic_forward<k, v, n>::basic_forward(basic_forward&& other)
    :
	pointer_(std::move(other.pointer_))
{
    other.pointer_ = nullptr;
}

template <class k, class v, class n>
template <class other_value_t>
basic_forward<k, v, n>::basic_forward(const basic_forward<k, other_value_t, n>& other)
    :
	pointer_(other.ptr())
{ }

template <class k, class v, class n>
basic_forward<k, v, n>& basic_forward<k, v, n>::operator=(const basic_forward& other)
{
    if (this != &other)
    {
	pointer_ = other.pointer_;
    }
    return *this;
}

template <class k, class v, class n>
basic_forward<k, v, n>& basic_forward<k, v, n>::operator=(basic_forward&& other)
{
    if (this != &other)
    {
	pointer_ = std::move(other.pointer_);
	other.pointer_ = nullptr;
    }
    return *this;
}

template <class k, class v, class n>
template <class other_value_t>
basic_forward<k, v, n>& basic_forward<k, v, n>::operator=(const basic_forward<k, other_value_t, n>& other)
{
    pointer_ = other.ptr();
    return *this;
}

template <class k, class v, class n>
basic_forward<k, v, n>& basic_forward<k, v, n>::operator=(node_type* other)
{
    pointer_ = other;
    return *this;
}

template <class k, class v, class n>
bool basic_forward<k, v, n>::operator==(const basic_forward& other) const
{
    return pointer_ == other.pointer_;
}

template <class k, class v, class n>
typename basic_forward<k, v, n>::value_type& basic_forward<k, v, n>::operator*()
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

template <class k, class v, class n>
typename basic_forward<k, v, n>::value_type* basic_forward<k, v, n>::operator->()
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

template <class k, class v, class n>
basic_forward<k, v, n>& basic_forward<k, v, n>::operator++()
{
    // TODO
    return *this;
}

template <class k, class v, class n>
basic_forward<k, v, n> basic_forward<k, v, n>::operator++(int)
{
    basic_forward<k, v, n> tmp = *this;
    ++(*this);
    return tmp;
}

template <class k, class v, class n>
basic_forward<k, v, n>& basic_forward<k, v, n>::operator--()
{
    // TODO
    return *this;
}

template <class k, class v, class n>
basic_forward<k, v, n> basic_forward<k, v, n>::operator--(int)
{
    basic_forward<k, v, n> tmp = *this;
    --(*this);
    return tmp;
}

template <class k, class v, class n>
typename basic_forward<k, v, n>::key_type basic_forward<k, v, n>::get_key() const
{
    if (pointer_ != nullptr)
    {
	return pointer_->key;
    }
    else
    {
	throw invalid_dereference_error("cannot dereference invalid bitwise_trie iterator");
    }
}

} // namespace bitwise_trie_iterator

template <class k, class v, class a>
constexpr std::array<std::size_t, 2U> bitwise_trie<k, v, a>::node_sizes;

template <class k, class v, class a>
constexpr std::array<std::size_t, 2U> bitwise_trie<k, v, a>::node_alignments;

template <class k, class v, class a>
bitwise_trie<k, v, a>::bitwise_trie(allocator_type& allocator)
    :
	allocator_(allocator),
	size_(0U),
	root_()
{
    for (auto&& index_ptr: leading_zero_index_)
    {
	index_ptr = nullptr;
    }
}

template <class k, class v, class a>
template <class... value_args_t>
std::tuple<typename bitwise_trie<k, v ,a>::iterator, bool> bitwise_trie<k, v ,a>::emplace(
	typename bitwise_trie<k, v ,a>::key_type key,
	value_args_t&&... value_args)
{
    branch_ptr* current_branch = &root_;
    key_type current_key = key;
    key_type branch_prefix = key;
    for (std::size_t branch_level = 1U; branch_level < depth() ; ++branch_level)
    {
	if (current_branch->is_empty())
	{
	    current_branch->reset(create_branch());
	    current_branch->set_tag(child_type::branch);
	}
	branch_prefix = get_prefix(current_key);
	current_branch = &((*current_branch)->children[branch_prefix]);
	current_key = current_key << radix_bit_size();
    }
    if (current_branch->is_empty())
    {
	leaf* new_leaf = create_leaf(key, std::forward<value_args_t>(value_args)...);
	current_branch->reset(static_cast<branch*>(static_cast<void*>(new_leaf)));
	current_branch->set_tag(child_type::leaf);
	++size_;
	return std::make_tuple(iterator(new_leaf), true);
    }
    else if (current_branch->get_tag() == child_type::leaf)
    {
	return std::make_tuple(iterator(static_cast<leaf*>(static_cast<void*>(current_branch->get_ptr()))), false);
    }
    else
    {
	throw invalid_bitwise_trie_error("encountered a branch where a leaf was expected");
    }
}

template <class k, class v, class a>
template <class... value_args_t>
bitwise_trie<k, v, a>::leaf::leaf(typename bitwise_trie<k, v ,a>::key_type key_arg, value_args_t&&... value_args)
    :
	key(key_arg),
	value(std::forward<value_args_t>(value_args)...)
{ }

template <class k, class v, class a>
bitwise_trie<k, v, a>::branch::branch()
{
    for (auto&& child_ptr: children)
    {
	child_ptr.reset();
	child_ptr.set_tag(child_type::branch);
    }
}

template <class k, class v, class a>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::min() const
{
    if (root_.is_empty())
    {
	return nullptr;
    }
    branch_ptr current(root_);
    while (!current.is_empty())
    {
	auto child_iter = current->children.begin();
	while (child_iter != current->children.end())
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
	    else
	    {
		++child_iter;
	    }
	}
	if (child_iter == current->children.end())
	{
	    // every branch should have at least 1 child so this means the trie is invalid
	    throw invalid_bitwise_trie_error("branch has no children");
	}
    }
    return nullptr;
}

template <class k, class v, class a>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::max() const
{
    if (root_.is_empty())
    {
	return nullptr;
    }
    branch_ptr current(root_);
    while (!current.is_empty())
    {
	auto child_iter = current->children.rbegin();
	while (child_iter != current->children.rend())
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
	    else
	    {
		++child_iter;
	    }
	}
	if (child_iter == current->children.rend())
	{
	    // every branch should have at least 1 child so this means the trie is invalid
	    throw invalid_bitwise_trie_error("branch has no children");
	}
    }
    return nullptr;
}

template <class k, class v, class a>
template <class... value_args_t>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::create_leaf(
	typename bitwise_trie<k, v, a>::key_type key,
	value_args_t&&... value_args)
{
    leaf* tmp = allocator_.template allocate<leaf>();
    if (tmp != nullptr)
    {
	new (tmp) leaf(key, std::forward<value_args_t>(value_args)...);
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
