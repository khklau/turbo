#ifndef TURBO_CONTAINER_HEAP_HXX
#define TURBO_CONTAINER_HEAP_HXX

#include <turbo/container/heap.hpp>
#include <queue>
#include <utility>

namespace turbo {
namespace container {

template <class e, class c, class a>
template <class... args_t>
void heap<e, c, a>::emplace_back(args_t&&... args)
{
    data_.emplace_back(std::forward<args_t>(args)...);
    sift_leaf(data_.back());
}

template <class e, class c, class a>
template <class vector_t, class node_t>
std::size_t heap<e, c, a>::index(vector_t& data, node_t& node)
{
    return &(node) - &(data[0U]);
}

template <class e, class c, class a>
typename heap<e, c, a>::designation heap<e, c, a>::which_child(const element_type& node) const
{
    if ((index(data_, node) & 1U) == 1U)
    {
	return designation::left;
    }
    else
    {
	return designation::right;
    }
}

template <class e, class c, class a>
bool heap<e, c, a>::is_root(const element_type& node) const
{
    return &(front()) == &node;
}

template <class e, class c, class a>
bool heap<e, c, a>::is_leaf(const element_type& node) const
{
    return left_child(data_, node) == nullptr && right_child(data_, node) == nullptr;
}

template <class e, class c, class a>
typename heap<e, c, a>::element_type* heap<e, c, a>::parent(element_type& node)
{
    if (is_root(node))
    {
	return nullptr;
    }
    else
    {
	switch (which_child(node))
	{
	    case designation::left:
	    {
		return &(data_[(index(data_, node) - 1U) >> 1U]);
	    }
	    case designation::right:
	    {
		return &(data_[(index(data_, node) - 2U) >> 1U]);
	    }
	}
	return nullptr;
    }
}

template <class e, class c, class a>
template <class vector_t, class node_t>
node_t* heap<e, c, a>::left_child(vector_t& data, node_t& node)
{
    std::size_t result = (index(data, node) << 1U) + 1U;
    if (result < data.size())
    {
	return &(data[result]);
    }
    else
    {
	return nullptr;
    }
}

template <class e, class c, class a>
template <class vector_t, class node_t>
node_t* heap<e, c, a>::right_child(vector_t& data, node_t& node)
{
    std::size_t result = (index(data, node) << 1U) + 2U;
    if (result < data.size())
    {
	return &(data[result]);
    }
    else
    {
	return nullptr;
    }
}

template <class e, class c, class a>
typename heap<e, c, a>::element_type* heap<e, c, a>::sibling(element_type& node)
{
    element_type* parent_node = parent(node);
    if (parent_node == nullptr)
    {
	return nullptr;
    }
    else
    {
	switch (which_child(node))
	{
	    case designation::left:
	    {
		return right_child(data_, *parent_node);
	    }
	    case designation::right:
	    {
		return left_child(data_, *parent_node);
	    }
	}
	return nullptr;
    }
}

template <class e, class c, class a>
bool heap<e, c, a>::has_heap_property() const
{
    bool result = true;
    const element_type* left = nullptr;
    const element_type* right = nullptr;
    std::queue<const element_type*> work;
    work.emplace(&(front()));
    while (!work.empty())
    {
	left = left_child(data_, *(work.front()));
	if (left != nullptr)
	{
	    if (compare_func()(*(work.front()), *left))
	    {
		work.emplace(left);
	    }
	    else
	    {
		return false;
	    }
	}
	right = right_child(data_, *(work.front()));
	if (right != nullptr)
	{
	    if (compare_func()(*(work.front()), *right))
	    {
		work.emplace(right);
	    }
	    else
	    {
		return false;
	    }
	}
	work.pop();
    }
    return result;
}

template <class e, class c, class a>
void heap<e, c, a>::sift_root(element_type& node)
{
    const element_type* current = &node;
    bool at_correct_position = false;
    while (!is_leaf(*current) && !at_correct_position)
    {
	designation preference = designation::left;
	const element_type* left = left_child(data_, *current);
	const element_type* right = right_child(data_, *current);
	if (left != nullptr && right != nullptr)
	{
	    preference = (compare_func()(*left, *right))
		    ? left
		    : right;
	}
	if (preference == designation::left && left != nullptr && compare_func()(*left, *current))
	{
	    std::swap(*current, *left);
	    current = left;
	}
	else if (preference == designation::right && right != nullptr && compare_func()(*right, *current))
	{
	    std::swap(*current, *right);
	    current = right;
	}
	else
	{
	    // current node compares better than both left & right child
	    at_correct_position = true;
	}
    }
}

template <class e, class c, class a>
void heap<e, c, a>::sift_leaf(element_type& node)
{
    element_type* current = &node;
    bool at_correct_position = false;
    while (!is_root(*current) && !at_correct_position)
    {
	element_type* parent_node = parent(*current);
	if (parent_node != nullptr && compare_func()(*current, *parent_node))
	{
	    element_type* sibling_node = sibling(*current);
	    if (sibling_node == nullptr || compare_func()(*current, *sibling_node))
	    {
		std::swap(*current, *parent_node);
		current = parent_node;
	    }
	    else
	    {
		at_correct_position = true;
	    }
	}
	else
	{
	    at_correct_position = true;
	}
    }
}

} // namespace container
} // namespace turbo

#endif
