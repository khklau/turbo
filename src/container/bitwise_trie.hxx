#ifndef TURBO_CONTAINER_BITWISE_TRIE_HXX
#define TURBO_CONTAINER_BITWISE_TRIE_HXX

#include <turbo/container/bitwise_trie.hpp>
#include <algorithm>
#include <turbo/container/invalid_dereference_error.hpp>
#include <turbo/toolset/intrinsic.hpp>

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
	root_(),
	index_(root_)
{ }

template <class k, class v, class a>
template <class... value_args_t>
std::tuple<typename bitwise_trie<k, v ,a>::iterator, bool> bitwise_trie<k, v ,a>::emplace(
	typename bitwise_trie<k, v ,a>::key_type key,
	value_args_t&&... value_args)
{
    branch_ptr* current_branch = nullptr;
    trie_prefix prefix(key);
    std::tie(current_branch, prefix) = index_.search(key);
    for (; prefix.get_usage_count() < trie_prefix::max_usage(); prefix = prefix << 1U)
    {
	if (current_branch->is_empty())
	{
	    branch* new_branch = create_branch();
	    current_branch->reset(new_branch, child_type::branch);
	    if (prefix.get_common_prefix() == 0U)
	    {
		index_.insert(new_branch, prefix);
	    }
	}
	current_branch = &((*current_branch)->children[prefix.get_next_prefix()]);
    }
    if (current_branch->is_empty())
    {
	leaf* new_leaf = create_leaf(key, std::forward<value_args_t>(value_args)...);
	current_branch->reset(static_cast<branch*>(static_cast<void*>(new_leaf)), child_type::leaf);
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
    branch_ptr empty(child_type::branch);
    std::fill_n(children.begin(), children.max_size(), empty);
}

template <class k, class v, class a>
bitwise_trie<k, v, a>::leading_zero_index::leading_zero_index(branch_ptr& root)
    :
	root_(root)
{
    branch_ptr empty(child_type::branch);
    std::fill_n(index_.begin(), index_.max_size(), empty);
}

template <class k, class v, class a>
std::tuple<typename bitwise_trie<k, v, a>::branch_ptr*, typename bitwise_trie<k, v, a>::trie_prefix> bitwise_trie<k, v, a>::leading_zero_index::search(
	key_type key)
{
    std::size_t zero_count = turbo::toolset::count_leading_zero(key);
    branch_ptr& shortcut = index_[zero_count];
    trie_prefix prefix(key);
    if (shortcut.is_empty() || zero_count == trie_prefix::key_bit_size())
    {
	return std::make_tuple(&root_, prefix);
    }
    else
    {
	return std::make_tuple(&shortcut, prefix << zero_count);
    }
}

template <class k, class v, class a>
void bitwise_trie<k, v, a>::leading_zero_index::insert(
	branch* branch,
	const trie_prefix& prefix)
{
    index_[prefix.get_usage_count()].reset(branch, child_type::branch);
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
	auto end = current->children.end();
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
	if (child_iter == end)
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
	auto end = current->children.rend();
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
	if (child_iter == end)
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
