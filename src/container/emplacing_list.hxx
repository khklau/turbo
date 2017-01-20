#ifndef TURBO_CONTAINER_EMPLACING_LIST_HXX
#define TURBO_CONTAINER_EMPLACING_LIST_HXX

#include <turbo/container/emplacing_list.hpp>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {

namespace emplacing_list_iterator {

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward()
    :
	pointer_(nullptr)
{ }

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward(node_type* pointer)
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
	throw invalid_dereference("cannot dereference invalid emplacing_list iterator");
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
	throw invalid_dereference("cannot dereference emplacing_list iterator");
    }
}

template <class value_t, class node_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator++()
{
    if (pointer_ != nullptr)
    {
	pointer_ = pointer_->next;
    }
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
    if (pointer_ != nullptr)
    {
	pointer_ = pointer_->previous;
    }
    return *this;
}

template <class value_t, class node_t>
basic_forward<value_t, node_t> basic_forward<value_t, node_t>::operator--(int)
{
    basic_forward<value_t, node_t> tmp = *this;
    --(*this);
    return tmp;
}

} // namespace emplacing_list_iterator

template <class value_t, class typed_allocator_t>
template <class... args_t>
emplacing_list<value_t, typed_allocator_t>::node::node(args_t&&... args)
    :
	value(std::forward<args_t>(args)...),
	alive(true),
	next(nullptr),
	previous(nullptr)
{ }

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::emplacing_list(typed_allocator_type& allocator)
    :
	allocator_(allocator),
	size_(0U),
	front_(nullptr),
	back_(nullptr)
{ }

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::~emplacing_list()
{
    for (auto iter = cbegin(); iter != cend(); iter = erase(iter)) { }
    front_ = nullptr;
    back_ = nullptr;
}

template <class v, class a>
template <class... args_t>
typename emplacing_list<v, a>::iterator emplacing_list<v, a>::emplace_front(args_t&&... args)
{
    node* old_front = front_;
    node* new_front = create_node(std::forward<args_t>(args)...);
    new_front->next = front_;
    front_ = new_front;
    ++size_;
    if (old_front != nullptr)
    {
	old_front->previous = new_front;
    }
    if (back_ == nullptr)
    {
	back_ = new_front;
    }
    return new_front;
}

template <class v, class a>
template <class... args_t>
typename emplacing_list<v, a>::iterator emplacing_list<v, a>::emplace_back(args_t&&... args)
{
    node* old_back = back_;
    node* new_back = create_node(std::forward<args_t>(args)...);
    new_back->previous = back_;
    back_ = new_back;
    ++size_;
    if (old_back != nullptr)
    {
	old_back->next = new_back;
    }
    if (front_ == nullptr)
    {
	front_ = new_back;
    }
    return new_back;
}

template <class v, class a>
template <class... args_t>
typename emplacing_list<v, a>::iterator emplacing_list<v, a>::emplace(const_iterator new_next, args_t&&... args)
{
    if (new_next.ptr() == nullptr)
    {
	if (size_ == 0U)
	{
	    emplace_front(std::forward<args_t>(args)...);
	    return front_;
	}
	else
	{
	    emplace_back(std::forward<args_t>(args)...);
	    return back_;
	}
    }
    else
    {
	if (new_next.ptr()->previous == nullptr)
	{
	    emplace_front(std::forward<args_t>(args)...);
	    return front_;
	}
	else
	{
	    node* new_previous = new_next.ptr()->previous;
	    node* new_node = create_node(std::forward<args_t>(args)...);
	    new_node->next = new_next.ptr();
	    new_node->previous = new_previous;
	    new_previous->next = new_node;
	    new_next.ptr()->previous = new_node;
	    ++size_;
	    return new_node;
	}
    }
}

template <class v, class a>
void emplacing_list<v, a>::pop_front()
{
    if (front_ == nullptr)
    {
	return;
    }
    node* old_front = front_;
    old_front->alive = false;
    node* next_node = old_front->next;
    front_ = old_front->next;
    old_front->next = nullptr;
    if (next_node != nullptr)
    {
	next_node->previous = nullptr;
    }
    old_front->previous = nullptr;
    destroy_node(old_front);
    --size_;
    if (size_ == 0U)
    {
	back_ = nullptr;
    }
}

template <class v, class a>
void emplacing_list<v, a>::pop_back()
{
    if (back_ == nullptr)
    {
	return;
    }
    node* old_back = back_;
    old_back->alive = false;
    if (old_back->previous != nullptr)
    {
	node* previous_node = old_back->previous;
	previous_node->next = nullptr;
	old_back->next = nullptr;
	back_ = previous_node;
    }
    else
    {
	old_back->next = nullptr;
	back_ = nullptr;
    }
    destroy_node(old_back);
    --size_;
    if (size_ == 0U)
    {
	front_ = nullptr;
    }
}

template <class v, class a>
typename emplacing_list<v, a>::iterator emplacing_list<v, a>::erase(const_iterator position)
{
    node* erase_node = position.ptr();
    if (size_ == 0U || erase_node == nullptr)
    {
	return end();
    }
    else
    {
	if (erase_node->previous == nullptr)
	{
	    pop_front();
	    return begin();
	}
	else if (erase_node->next == nullptr)
	{
	    pop_back();
	    return end();
	}
	else
	{
	    erase_node->alive = false;
	    node* previous_node = erase_node->previous;
	    node* next_node = erase_node->next;
	    erase_node->next = nullptr;
	    previous_node->next = next_node;
	    next_node->previous = previous_node;
	    destroy_node(erase_node);
	    --size_;
	    return next_node;
	}
    }
}

template <class v, class a>
template <class... args_t>
typename emplacing_list<v, a>::node* emplacing_list<v, a>::create_node(args_t&&... args)
{
    node* tmp = allocator_.template allocate<node>();
    if (tmp != nullptr)
    {
	new (tmp) node(std::forward<args_t>(args)...);
	return tmp;
    }
    else
    {
	throw std::runtime_error("Out of memory");
    }
    //return new node(std::forward<args_t>(args)...);
}

template <class v, class a>
void emplacing_list<v, a>::destroy_node(node* pointer)
{
    pointer->~node();
    allocator_.template deallocate<node>(pointer);
}

} // namespace container
} // namespace turbo

#endif
