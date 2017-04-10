#ifndef TURBO_CONTAINER_BITWISE_TRIE_HXX
#define TURBO_CONTAINER_BITWISE_TRIE_HXX

#include <turbo/container/bitwise_trie.hpp>
#include <cmath>
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
	throw invalid_dereference_error("cannot dereference invalid bitwise_trie iterator");
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
bitwise_trie<k, v, a>::bitwise_trie(const bitwise_trie& other, allocator_type* allocator)
    :
	allocator_(allocator != nullptr ? *allocator : other.allocator_),
	size_(other.size_),
	root_(other.root_),
	index_(root_)
{
    branch_ptr* current_branch = &root_;
    trie_key tkey;
    for (typename trie_key::iterator iter = tkey.begin(); iter.is_valid(); ++iter)
    {
	if (current_branch == nullptr || current_branch->is_empty())
	{
	    return;
	}
	else
	{
	    tkey.write(iter, 0U);
	    index_.insert(current_branch->get_ptr(), tkey, iter);
	    current_branch = &((*current_branch)->children[0U]);
	}
    }
}

template <class k, class v, class a>
bitwise_trie<k, v, a>::~bitwise_trie()
{
    trie_key tkey(0U);
    erase_recursive(
	    &root_,
	    tkey,
	    tkey.begin(),
	    [] (const typename trie_key::iterator&, key_type, key_type, const branch_ptr&) -> bool
    {
	return true;
    });
    if (!root_.is_empty())
    {
	destroy_branch(root_.get_ptr());
	root_.reset();
	index_.remove(tkey, tkey.begin());
    }
}

template <class k, class v, class a>
bool bitwise_trie<k, v ,a>::operator==(const bitwise_trie& other) const
{
    trie_key key;
    return this->size_ == other.size_
	&& is_equal(&(this->root_), &(other.root_), this->index_, other.index_, key, key.begin());
}

template <class k, class v, class a>
typename bitwise_trie<k, v ,a>::const_iterator bitwise_trie<k, v ,a>::find(key_type key) const
{
    const branch_ptr* current_branch = nullptr;
    trie_key tkey(key);
    typename trie_key::iterator iter = tkey.begin();
    std::tie(current_branch, iter) = index_.const_search(tkey);
    for (; iter.is_valid(); ++iter)
    {
	if (current_branch == nullptr || current_branch->is_empty())
	{
	    return const_iterator(*this, nullptr);
	}
	const branch_ptr& child_branch = (*current_branch)->children[std::get<1>(tkey.read(iter))];
	if (child_branch.is_empty())
	{
	    return const_iterator(*this, nullptr);
	}
	else if (child_branch.get_tag() == child_type::leaf)
	{
	    return const_iterator(*this, static_cast<leaf*>(static_cast<void*>(child_branch.get_ptr())));
	}
	else
	{
	    current_branch = &child_branch;
	}
    }
    return const_iterator(*this, nullptr);
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
		index_.insert(new_branch, tkey, iter);
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
    std::size_t leaf_erase_count = 0U;
    std::size_t child_count = 0U;
    std::tie(leaf_erase_count, child_count) = erase_recursive(
	    &root_,
	    tkey,
	    tkey.begin(),
	    [] (const typename trie_key::iterator&, key_type prefix_wanted, key_type prefix_found, const branch_ptr&) -> bool
    {
	return prefix_found == prefix_wanted;
    });
    if (child_count == 0U && !root_.is_empty())
    {
	destroy_branch(root_.get_ptr());
	root_.reset();
	index_.remove(tkey, tkey.begin());
    }
    return leaf_erase_count;
}

template <class k, class v, class a>
template <class... value_args_t>
bitwise_trie<k, v, a>::leaf::leaf(typename bitwise_trie<k, v ,a>::key_type key_arg, value_args_t&&... value_args)
    :
	key(key_arg),
	value(std::forward<value_args_t>(value_args)...)
{ }

template <class k, class v, class a>
bitwise_trie<k, v, a>::leaf::leaf(const leaf& other)
    :
	key(other.key),
	value(other.value)
{ }

template <class k, class v, class a>
bool bitwise_trie<k, v, a>::leaf::operator==(const leaf& other) const
{
    return this->key == other.key && this->value == other.value;
}

template <class k, class v, class a>
bitwise_trie<k, v, a>::branch::branch()
{
    branch_ptr empty(child_type::branch);
    std::fill_n(children.begin(), children.max_size(), empty);
}

template <class k, class v, class a>
bitwise_trie<k, v, a>::branch::branch(const branch& other)
{
    auto this_iter = this->children.begin();
    auto other_iter = other.chidren.cbegin();
    for (; this_iter != this->children.end() && other_iter != other.children.cend(); ++this_iter, ++other_iter)
    {
	if (!other_iter->is_empty())
	{
	    continue;
	}
	else if (other_iter->get_tag() == child_type::leaf)
	{
	    this_iter->reset(clone_leaf(*(static_cast<leaf*>(static_cast<void*>(other_iter->get_ptr())))), child_type::leaf);
	}
	else
	{
	    this_iter->reset(clone_branch(*(other_iter->get_ptr())), child_type::branch);
	}
    }
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
bool bitwise_trie<k, v, a>::leading_zero_index::is_defined(
	const trie_key& key,
	const typename trie_key::iterator& iter) const
{
    trie_key tmp(std::numeric_limits<key_type>::max());
    tmp.copy(key, key.begin(), iter);
    std::size_t zero_count = turbo::toolset::count_leading_zero(tmp.get_key());
    bool result = false;
    if (zero_count < index_.max_size())
    {
	result = !(index_[zero_count].is_empty());
    }
    return result;
}

template <class k, class v, class a>
std::tuple<typename bitwise_trie<k, v, a>::branch_ptr*, typename bitwise_trie<k, v, a>::trie_key::iterator> bitwise_trie<k, v, a>::leading_zero_index::search(
	const trie_key& key)
{
    key_type zero_count = turbo::toolset::count_leading_zero(key.get_key());
    branch_ptr& shortcut = index_[zero_count];
    typename trie_key::iterator iter = key.begin();
    if (shortcut.is_empty() || zero_count == trie_key::key_bit_size())
    {
	return std::make_tuple(&root_, iter);
    }
    else
    {
	constexpr std::size_t radix_bit_div_size = std::lround(std::log2(trie_key::radix_bit_size()));
	iter += zero_count >> radix_bit_div_size;
	constexpr std::size_t shift_qty = trie_key::key_bit_size() - radix_bit_div_size;
	key_type remainder = static_cast<key_type>(zero_count << shift_qty) >> shift_qty;
	if (remainder != 0U)
	{
	    // zero counts that aren't divisible by the radix size need rounding up
	    iter += 1U;
	}
	return std::make_tuple(&shortcut, iter);
    }
}

template <class k, class v, class a>
std::tuple<const typename bitwise_trie<k, v, a>::branch_ptr*, typename bitwise_trie<k, v, a>::trie_key::iterator> bitwise_trie<k, v, a>::leading_zero_index::const_search(
	const trie_key& key) const
{
    key_type zero_count = turbo::toolset::count_leading_zero(key.get_key());
    const branch_ptr& shortcut = index_[zero_count];
    typename trie_key::iterator iter = key.begin();
    if (shortcut.is_empty() || zero_count == trie_key::key_bit_size())
    {
	return std::make_tuple(&root_, iter);
    }
    else
    {
	constexpr std::size_t radix_bit_div_size = std::lround(std::log2(trie_key::radix_bit_size()));
	iter += zero_count >> radix_bit_div_size;
	constexpr std::size_t shift_qty = trie_key::key_bit_size() - radix_bit_div_size;
	key_type remainder = static_cast<key_type>(zero_count << shift_qty) >> shift_qty;
	if (remainder != 0U)
	{
	    // zero counts that aren't divisible by the radix size need rounding up
	    iter += 1U;
	}
	return std::make_tuple(&shortcut, iter);
    }
}

template <class k, class v, class a>
void bitwise_trie<k, v, a>::leading_zero_index::insert(
	branch* branch,
	const trie_key& key,
	const typename trie_key::iterator& iter)
{
    trie_key tmp(std::numeric_limits<key_type>::max());
    tmp.copy(key, key.begin(), iter);
    std::size_t zero_count = turbo::toolset::count_leading_zero(tmp.get_key());
    if (zero_count < index_.max_size())
    {
	index_[zero_count].reset(branch, child_type::branch);
    }
}

template <class k, class v, class a>
void bitwise_trie<k, v, a>::leading_zero_index::remove(
	const trie_key& key,
	const typename trie_key::iterator& iter)
{
    trie_key tmp(std::numeric_limits<key_type>::max());
    tmp.copy(key, key.begin(), iter);
    std::size_t zero_count = turbo::toolset::count_leading_zero(tmp.get_key());
    if (zero_count < index_.max_size())
    {
	index_[zero_count].reset();
    }
}

template <class k, class v, class a>
bool bitwise_trie<k, v, a>::is_equal(
	const branch_ptr* this_branch,
	const branch_ptr* other_branch,
	const leading_zero_index& this_index,
	const leading_zero_index& other_index,
	trie_key key,
	typename trie_key::iterator iter)
{
    bool both_are_null = this_branch == nullptr && other_branch == nullptr;
    bool both_are_empty = this_branch != nullptr && this_branch->is_empty() && other_branch != nullptr && other_branch->is_empty();
    bool both_not_empty = this_branch != nullptr && !this_branch->is_empty() && other_branch != nullptr && !other_branch->is_empty();
    if (!iter.is_valid())
    {
	return both_are_null || both_are_empty || both_not_empty;
    }
    else if (both_not_empty)
    {
	bool branch_result = true;
	for (std::size_t child_index = 0U; child_index < (*this_branch)->children.max_size(); ++child_index)
	{
	    const branch_ptr& this_child = (*this_branch)->children[child_index];
	    const branch_ptr& other_child = (*other_branch)->children[child_index];
	    key.write(iter, child_index);
	    if (!this_child.is_empty() && !other_child.is_empty())
	    {
		if (this_child.get_tag() == child_type::leaf && other_child.get_tag() == child_type::leaf)
		{
		    return *(static_cast<leaf*>(static_cast<void*>(this_child.get_ptr())))
			== *(static_cast<leaf*>(static_cast<void*>(other_child.get_ptr())));
		}
		else if (this_child.get_tag() == child_type::branch)
		{
		    // TODO compare leading zero index on these branches
		    bool child_result = is_equal(&this_child, &other_child, this_index, other_index, key, iter + 1U);
		    branch_result = branch_result && child_result;
		    if (!child_result)
		    {
			return branch_result;
		    }
		}
	    }
	    else if (this_child.is_empty() && other_child.is_empty())
	    {
		continue;
	    }
	    else
	    {
		return false;
	    }
	}
	return branch_result;
    }
    else
    {
	return both_are_null || both_are_empty;
    }
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
template <typename compare_t>
std::tuple<std::size_t, std::size_t> bitwise_trie<k, v, a>::erase_recursive(
	branch_ptr* branch,
	const trie_key& key,
	typename trie_key::iterator iter,
	compare_t compare_func)
{
    if (!iter.is_valid() || branch == nullptr || branch->is_empty())
    {
	return std::make_tuple(0U, 0U);
    }
    std::size_t child_count = 0U;
    std::size_t leaf_erase_count = 0U;
    for (std::size_t child_index = 0U; child_index < (*branch)->children.max_size(); ++child_index)
    {
	branch_ptr& child_branch = (*branch)->children[child_index];
	if (!child_branch.is_empty())
	{
	    if (compare_func(iter, std::get<1>(key.read(iter)), child_index, child_branch))
	    {
		if (child_branch.get_tag() == child_type::leaf)
		{
		    destroy_leaf(static_cast<leaf*>(static_cast<void*>(child_branch.get_ptr())));
		    child_branch.reset();
		    ++leaf_erase_count;
		    --size_;
		}
		else
		{
		    std::size_t grand_child_count = 0U;
		    std::size_t child_leaf_erase_count = 0U;
		    std::tie(child_leaf_erase_count, grand_child_count) = erase_recursive(&child_branch, key, iter + 1U, compare_func);
		    leaf_erase_count += child_leaf_erase_count;
		    if (grand_child_count == 0U)
		    {
			// nothing left under this child branch so destroy it
			destroy_branch(child_branch.get_ptr());
			child_branch.reset();
			// Branches are created on their level, but are destroyed at their parent's level, so we can't use the parent's iterator
			typename trie_key::iterator child_iter = iter + 1U;
			auto result = key.get_preceding_prefixes(child_iter);
			if (std::get<0>(result) == trie_key::get_result::unavailable || std::get<1>(result) == 0U)
			{
			    index_.remove(key, child_iter);
			}
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
    return std::make_tuple(leaf_erase_count, child_count);
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
typename bitwise_trie<k, v, a>::leaf* bitwise_trie<k, v, a>::clone_leaf(const leaf& other)
{
    leaf* tmp = allocator_.template allocate<leaf>();
    if (tmp != nullptr)
    {
	new (tmp) leaf(other);
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
typename bitwise_trie<k, v, a>::branch* bitwise_trie<k, v, a>::clone_branch(const branch& other)
{
    branch* tmp = allocator_.template allocate<branch>();
    if (tmp != nullptr)
    {
	new (tmp) branch(other);
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
