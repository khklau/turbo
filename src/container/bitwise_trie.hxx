#ifndef TURBO_CONTAINER_BITWISE_TRIE_HXX
#define TURBO_CONTAINER_BITWISE_TRIE_HXX

#include <turbo/container/bitwise_trie.hpp>
#include <algorithm>
#include <turbo/container/invalid_dereference_error.hpp>
#include <turbo/toolset/intrinsic.hpp>

namespace turbo {
namespace container {

namespace bitwise_trie_iterator {

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>::basic_forward(trie_type& trie) noexcept
    :
	trie_(trie),
	pointer_(nullptr)
{ }

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>::basic_forward(trie_type& trie, node_type* pointer) noexcept
    :
	trie_(trie),
	pointer_(pointer)
{ }

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>::basic_forward(const basic_forward& other)
    :
	trie_(other.trie_),
	pointer_(other.pointer_)
{ }

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>::basic_forward(basic_forward&& other)
    :
	trie_(other.trie_),
	pointer_(std::move(other.pointer_))
{
    other.pointer_ = nullptr;
}

template <class t, class k, class v, class n>
template <class other_value_t>
basic_forward<t, k, v, n>::basic_forward(const basic_forward<t, k, other_value_t, n>& other)
    :
	trie_(other.get_trie()),
	pointer_(other.get_ptr())
{ }

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>& basic_forward<t, k, v, n>::operator=(const basic_forward& other)
{
    if (this != &other && &trie_ == &other.trie_)
    {
	pointer_ = other.pointer_;
    }
    return *this;
}

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>& basic_forward<t, k, v, n>::operator=(basic_forward&& other)
{
    if (this != &other && &trie_ == &other.trie_)
    {
	pointer_ = std::move(other.pointer_);
	other.pointer_ = nullptr;
    }
    return *this;
}

template <class t, class k, class v, class n>
template <class other_value_t>
basic_forward<t, k, v, n>& basic_forward<t, k, v, n>::operator=(const basic_forward<t, k, other_value_t, n>& other)
{
    if (&trie_ == &other.trie_)
    {
	pointer_ = other.get_ptr();
    }
    return *this;
}

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>& basic_forward<t, k, v, n>::operator=(node_type* other)
{
    pointer_ = other;
    return *this;
}

template <class t, class k, class v, class n>
bool basic_forward<t, k, v, n>::operator==(const basic_forward& other) const
{
    return &trie_ == &other.trie_ && pointer_ == other.pointer_;
}

template <class t, class k, class v, class n>
typename basic_forward<t, k, v, n>::value_type& basic_forward<t, k, v, n>::operator*()
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

template <class t, class k, class v, class n>
typename basic_forward<t, k, v, n>::value_type* basic_forward<t, k, v, n>::operator->()
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

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>& basic_forward<t, k, v, n>::operator++()
{
    // TODO
    return *this;
}

template <class t, class k, class v, class n>
basic_forward<t, k, v, n> basic_forward<t, k, v, n>::operator++(int)
{
    basic_forward<t, k, v, n> tmp = *this;
    ++(*this);
    return tmp;
}

template <class t, class k, class v, class n>
basic_forward<t, k, v, n>& basic_forward<t, k, v, n>::operator--()
{
    // TODO
    return *this;
}

template <class t, class k, class v, class n>
basic_forward<t, k, v, n> basic_forward<t, k, v, n>::operator--(int)
{
    basic_forward<t, k, v, n> tmp = *this;
    --(*this);
    return tmp;
}

template <class t, class k, class v, class n>
typename basic_forward<t, k, v, n>::key_type basic_forward<t, k, v, n>::get_key() const
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
    trie_key tkey(key);
    typename trie_key::iterator iter = tkey.begin();
    std::tie(current_branch, iter) = index_.search(tkey);
    for (; iter.is_valid(); ++iter)
    {
	if (current_branch->is_empty())
	{
	    branch* new_branch = create_branch();
	    current_branch->reset(new_branch, child_type::branch);
	    auto result = tkey.get_preceding_prefixes(iter);
	    if (std::get<0>(result) == trie_key::get_result::unavailable || std::get<1>(result) == 0U)
	    {
		index_.insert(new_branch, iter);
	    }
	}
	current_branch = &((*current_branch)->children[std::get<1>(tkey.read(iter))]);
    }
    if (current_branch->is_empty())
    {
	leaf* new_leaf = create_leaf(key, std::forward<value_args_t>(value_args)...);
	current_branch->reset(static_cast<branch*>(static_cast<void*>(new_leaf)), child_type::leaf);
	++size_;
	return std::make_tuple(iterator(*this, new_leaf), true);
    }
    else if (current_branch->get_tag() == child_type::leaf)
    {
	return std::make_tuple(iterator(*this, static_cast<leaf*>(static_cast<void*>(current_branch->get_ptr()))), false);
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
std::tuple<typename bitwise_trie<k, v, a>::branch_ptr*, typename bitwise_trie<k, v, a>::trie_key::iterator> bitwise_trie<k, v, a>::leading_zero_index::search(
	const trie_key& key)
{
    std::size_t zero_count = turbo::toolset::count_leading_zero(key.get_key());
    branch_ptr& shortcut = index_[zero_count];
    typename trie_key::iterator iter = key.begin();
    if (shortcut.is_empty() || zero_count == trie_key::key_bit_size())
    {
	return std::make_tuple(&root_, iter);
    }
    else
    {
	iter += zero_count;
	return std::make_tuple(&shortcut, iter);
    }
}

template <class k, class v, class a>
std::tuple<const typename bitwise_trie<k, v, a>::branch_ptr*, typename bitwise_trie<k, v, a>::trie_key::iterator> bitwise_trie<k, v, a>::leading_zero_index::const_search(
	const trie_key& key) const
{
    std::size_t zero_count = turbo::toolset::count_leading_zero(key.get_key());
    const branch_ptr& shortcut = index_[zero_count];
    typename trie_key::iterator iter = key.begin();
    if (shortcut.is_empty() || zero_count == trie_key::key_bit_size())
    {
	return std::make_tuple(&root_, iter);
    }
    else
    {
	iter += zero_count;
	return std::make_tuple(&shortcut, iter);
    }
}

template <class k, class v, class a>
void bitwise_trie<k, v, a>::leading_zero_index::insert(
	branch* branch,
	const typename trie_key::iterator& iter)
{
    index_[iter.get_index()].reset(branch, child_type::branch);
}

template <class k, class v, class a>
template <typename compare_t>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::least_search(
	typename bitwise_trie<k, v ,a>::key_type key,
	compare_t compare_func) const
{
    const branch_ptr* current_branch = nullptr;
    trie_key key_wanted(key);
    trie_key key_found(key);
    typename trie_key::iterator iter = key_wanted.begin();
    std::tie(current_branch, iter) = index_.const_search(key_wanted);
    for (; iter.is_valid(); ++iter)
    {
	if (current_branch == nullptr || current_branch->is_empty())
	{
	    return nullptr;
	}
	std::size_t child_index = 0U;
	std::size_t index_max = (*current_branch)->children.max_size();
	for (; child_index < index_max; ++child_index)
	{
	    const branch_ptr& child_branch = (*current_branch)->children[child_index];
	    key_found.write(iter, child_index);
	    if (!child_branch.is_empty() && compare_func(iter, key_wanted, key_found, child_branch))
	    {
		if (child_branch.get_tag() == child_type::leaf)
		{
		    return static_cast<leaf*>(static_cast<void*>(child_branch.get_ptr()));
		}
		else if (child_branch.get_tag() == child_type::branch)
		{
		    current_branch = &child_branch;
		    break;
		}
	    }
	}
	if (child_index == index_max)
	{
	    // every branch should have at least 1 child so this means the trie is invalid
	    throw invalid_bitwise_trie_error("branch has no children");
	}
    }
    return nullptr;
}

template <class k, class v, class a>
template <typename compare_t>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::most_search(
	typename bitwise_trie<k, v ,a>::key_type key,
	compare_t compare_func) const
{
    const branch_ptr* current_branch = nullptr;
    trie_key key_wanted(key);
    trie_key key_found(key);
    typename trie_key::iterator iter = key_wanted.begin();
    std::tie(current_branch, iter) = index_.const_search(key_wanted);
    for (; iter.is_valid(); ++iter)
    {
	if (current_branch == nullptr || current_branch->is_empty())
	{
	    return nullptr;
	}
	std::int64_t child_index = (*current_branch)->children.max_size() - 1U;
	for (; 0 <= child_index; --child_index)
	{
	    const branch_ptr& child_branch = (*current_branch)->children[child_index];
	    key_found.write(iter, child_index);
	    if (!child_branch.is_empty() && compare_func(iter, key_wanted, key_found, child_branch))
	    {
		if (child_branch.get_tag() == child_type::leaf)
		{
		    return static_cast<leaf*>(static_cast<void*>(child_branch.get_ptr()));
		}
		else if (child_branch.get_tag() == child_type::branch)
		{
		    current_branch = &child_branch;
		    break;
		}
	    }
	}
	if (child_index < 0)
	{
	    // every branch should have at least 1 child so this means the trie is invalid
	    throw invalid_bitwise_trie_error("branch has no children");
	}
    }
    return nullptr;
}

template <class k, class v, class a>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::min() const
{
    return least_search(std::numeric_limits<key_type>::min(),
	    [] (const typename trie_key::iterator&, const trie_key&, const trie_key&, const branch_ptr&) -> bool
    {
	return true;
    });
}

template <class k, class v, class a>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::max() const
{
    return most_search(std::numeric_limits<key_type>::min(),
	    [] (const typename trie_key::iterator&, const trie_key&, const trie_key&, const branch_ptr&) -> bool
    {
	return true;
    });
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
