#ifndef TURBO_CONTAINER_BITWISE_TRIE_HXX
#define TURBO_CONTAINER_BITWISE_TRIE_HXX

#include <turbo/container/bitwise_trie.hpp>
#include <algorithm>
#include <limits>
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
    if (is_valid())
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
    if (is_valid())
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
    if (TURBO_LIKELY(is_valid()))
    {
	pointer_ = trie_.find_successor(*this).get_ptr();
    }
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
    if (TURBO_LIKELY(is_valid()))
    {
	pointer_ = trie_.find_predecessor(*this).get_ptr();
    }
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
    if (is_valid())
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
typename bitwise_trie<k, v ,a>::const_iterator bitwise_trie<k, v ,a>::find(key_type key) const
{
    trie_key key_wanted(key);
    trie_key key_found;
    auto result = index_.const_search(key_wanted);
    if (key < (std::numeric_limits<key_type>::max() - key))
    {
	return const_iterator(*this, least_first_search(
		std::get<0>(result),
		key_wanted,
		key_found,
		std::get<1>(result),
		[] (const typename trie_key::iterator& iter, const trie_key& wanted, const trie_key& found, const branch_ptr&) -> bool
	{
	    return iter.is_valid() && (std::get<1>(wanted.read(iter)) == std::get<1>(found.read(iter)));
	}));
    }
    else
    {
	return const_iterator(*this, most_first_search(
		std::get<0>(result),
		key_wanted,
		key_found,
		std::get<1>(result),
		[] (const typename trie_key::iterator& iter, const trie_key& wanted, const trie_key& found, const branch_ptr&) -> bool
	{
	    return iter.is_valid() && (std::get<1>(wanted.read(iter)) == std::get<1>(found.read(iter)));
	}));
    }
}

template <class k, class v, class a>
typename bitwise_trie<k, v ,a>::const_iterator bitwise_trie<k, v ,a>::find_successor(const_iterator iter) const
{
    trie_key key_ref_point(iter.get_key());
    trie_key key_found;
    return const_iterator(*this, least_first_search(
	    &root_,
	    key_ref_point,
	    key_found,
	    key_ref_point.begin(),
	    [] (const typename trie_key::iterator& iter, const trie_key& ref_point, const trie_key& found, const branch_ptr& branch) -> bool
    {
	if (branch.get_tag() == child_type::leaf)
	{
	    return ref_point.get_key() < found.get_key();
	}
	else
	{
	    return ref_point.get_preceding_prefixes(iter) <= found.get_preceding_prefixes(iter);
	}
    }));
}

template <class k, class v, class a>
typename bitwise_trie<k, v ,a>::const_iterator bitwise_trie<k, v ,a>::find_predecessor(const_iterator iter) const
{
    trie_key key_ref_point(iter.get_key());
    trie_key key_found;
    return const_iterator(*this, most_first_search(
	    &root_,
	    key_ref_point,
	    key_found,
	    key_ref_point.begin(),
	    [] (const typename trie_key::iterator& iter, const trie_key& ref_point, const trie_key& found, const branch_ptr& branch) -> bool
    {
	if (branch.get_tag() == child_type::leaf)
	{
	    return found.get_key() < ref_point.get_key();
	}
	else
	{
	    return found.get_preceding_prefixes(iter) <= ref_point.get_preceding_prefixes(iter);
	}
    }));
}

template <class k, class v, class a>
typename bitwise_trie<k, v ,a>::const_iterator bitwise_trie<k, v ,a>::find_less_equal(key_type key) const
{
    trie_key key_wanted(key);
    trie_key key_found;
    return const_iterator(*this, most_first_search(
	    &root_,
	    key_wanted,
	    key_found,
	    key_wanted.begin(),
	    [] (const typename trie_key::iterator& iter, const trie_key& wanted, const trie_key& found, const branch_ptr& branch) -> bool
    {
	if (branch.get_tag() == child_type::leaf)
	{
	    return found.get_key() <= wanted.get_key();
	}
	else if (!iter.is_valid())
	{
	    return false;
	}
	else if (iter == wanted.begin())
	{
	    return std::get<1>(found.read(iter)) <= std::get<1>(wanted.read(iter));
	}
	else
	{
	    return std::get<1>(found.get_preceding_prefixes(iter)) <= std::get<1>(wanted.get_preceding_prefixes(iter));
	}
    }));
}

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
std::size_t bitwise_trie<k, v ,a>::erase(key_type key)
{
    trie_key tkey(key);
    return std::get<0>(erase_recursive(&root_, tkey, tkey.begin()));
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
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::min() const
{
    trie_key key_wanted(std::numeric_limits<key_type>::min());
    trie_key key_found;
    return least_first_search(
	    &root_,
	    key_wanted,
	    key_found,
	    key_wanted.begin(),
	    [] (const typename trie_key::iterator&, const trie_key&, const trie_key&, const branch_ptr&) -> bool
    {
	return true;
    });
}

template <class k, class v, class a>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::max() const
{
    trie_key key_wanted(std::numeric_limits<key_type>::max());
    trie_key key_found;
    return most_first_search(
	    &root_,
	    key_wanted,
	    key_found,
	    key_wanted.begin(),
	    [] (const typename trie_key::iterator&, const trie_key&, const trie_key&, const branch_ptr&) -> bool
    {
	return true;
    });
}

template <class k, class v, class a>
template <typename compare_t>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::least_first_search(
	const branch_ptr* branch,
	trie_key key_wanted,
	trie_key key_found,
	typename trie_key::iterator iter,
	compare_t compare_func) const
{
    if (!iter.is_valid() || branch == nullptr || branch->is_empty())
    {
	return nullptr;
    }
    for (std::size_t child_index = 0U; child_index < (*branch)->children.max_size(); ++child_index)
    {
	const branch_ptr& child_branch = (*branch)->children[child_index];
	key_found.write(iter, child_index);
	if (!child_branch.is_empty() && compare_func(iter, key_wanted, key_found, child_branch))
	{
	    if (child_branch.get_tag() == child_type::leaf)
	    {
		return static_cast<leaf*>(static_cast<void*>(child_branch.get_ptr()));
	    }
	    else if (child_branch.get_tag() == child_type::branch)
	    {
		leaf* child_result = least_first_search(&child_branch, key_wanted, key_found, iter + 1U, compare_func);
		if (child_result != nullptr)
		{
		    return child_result;
		}
	    }
	}
    }
    return nullptr;
}

template <class k, class v, class a>
template <typename compare_t>
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::most_first_search(
	const branch_ptr* branch,
	trie_key key_wanted,
	trie_key key_found,
	typename trie_key::iterator iter,
	compare_t compare_func) const
{
    if (!iter.is_valid() || branch == nullptr || branch->is_empty())
    {
	return nullptr;
    }
    std::int64_t child_index = (*branch)->children.max_size() - 1U;
    for (; 0 <= child_index; --child_index)
    {
	const branch_ptr& child_branch = (*branch)->children[child_index];
	key_found.write(iter, child_index);
	if (!child_branch.is_empty() && compare_func(iter, key_wanted, key_found, child_branch))
	{
	    if (child_branch.get_tag() == child_type::leaf)
	    {
		return static_cast<leaf*>(static_cast<void*>(child_branch.get_ptr()));
	    }
	    else if (child_branch.get_tag() == child_type::branch)
	    {
		leaf* child_result = most_first_search(&child_branch, key_wanted, key_found, iter + 1U, compare_func);
		if (child_result != nullptr)
		{
		    return child_result;
		}
	    }
	}
    }
    return nullptr;
}

template <class k, class v, class a>
std::tuple<std::size_t, std::size_t> bitwise_trie<k, v, a>::erase_recursive(
	const branch_ptr* branch,
	const trie_key& key,
	typename trie_key::iterator iter)
{
    if (!iter.is_valid() || branch == nullptr || branch->is_empty())
    {
	return std::make_tuple(0U, 0U);
    }
    std::size_t child_count = 0U;
    std::size_t leaf_destroy_count = 0U;
    for (std::size_t child_index = 0U; child_index < (*branch)->children.max_size(); ++child_index)
    {
	const branch_ptr& child_branch = (*branch)->children[child_index];
	if (!child_branch.is_empty())
	{
	    if (child_index == std::get<1>(key.read(iter)))
	    {
		if (child_branch.get_tag() == child_type::leaf)
		{
		    destroy_leaf(static_cast<leaf*>(static_cast<void*>(child_branch.get_ptr())));
		    ++leaf_destroy_count;
		    --size_;
		}
		else
		{
		    std::size_t grand_child_count = 0U;
		    std::tie(leaf_destroy_count, grand_child_count) = erase_recursive(&child_branch, key, iter + 1U);
		    if (grand_child_count == 0U)
		    {
			// nothing left under this child branch so destroy it
			destroy_branch(child_branch.get_ptr());
		    }
		    else
		    {
			// the child branch contains values other than the one currently wanted
			++child_count;
		    }
		}
	    }
	    else
	    {
		// the child branch contains other values
		++child_count;
	    }
	}
    }
    return std::make_tuple(leaf_destroy_count, child_count);
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
