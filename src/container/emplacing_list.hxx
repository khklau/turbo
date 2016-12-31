#ifndef TURBO_CONTAINER_EMPLACING_LIST_HXX
#define TURBO_CONTAINER_EMPLACING_LIST_HXX

#include <turbo/container/emplacing_list.hpp>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {

namespace emplacing_list_iterator {

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>::basic_safe_forward()
    :
	pointer_(nullptr)
{ }

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>::basic_safe_forward(const std::shared_ptr<node_t>& pointer)
    :
	pointer_(pointer)
{ }

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>::basic_safe_forward(const basic_safe_forward& other)
    :
	pointer_(other.pointer_)
{ }

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>::basic_safe_forward(basic_safe_forward&& other)
    :
	pointer_(std::move(other.pointer_))
{
    other.pointer_.reset();
}

template <class value_t, class node_t>
template <class other_value_t>
basic_safe_forward<value_t, node_t>::basic_safe_forward(const basic_safe_forward<other_value_t, node_t>& other)
    :
	pointer_(other.ptr())
{ }

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>& basic_safe_forward<value_t, node_t>::operator=(const basic_safe_forward& other)
{
    if (this != &other)
    {
	pointer_ = other.pointer_;
    }
    return *this;
}

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>& basic_safe_forward<value_t, node_t>::operator=(basic_safe_forward&& other)
{
    if (this != &other)
    {
	pointer_ = std::move(other.pointer_);
	other.pointer_.reset();
    }
    return *this;
}

template <class value_t, class node_t>
template <class other_value_t>
basic_safe_forward<value_t, node_t>& basic_safe_forward<value_t, node_t>::operator=(const basic_safe_forward<other_value_t, node_t>& other)
{
    pointer_ = other.ptr();
    return *this;
}

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>& basic_safe_forward<value_t, node_t>::operator=(const std::shared_ptr<node_t>& other)
{
    pointer_ = other;
    return *this;
}

template <class value_t, class node_t>
bool basic_safe_forward<value_t, node_t>::operator==(const basic_safe_forward& other) const
{
    return pointer_ == other.pointer_;
}

template <class value_t, class node_t>
value_t& basic_safe_forward<value_t, node_t>::operator*()
{
    if (is_valid())
    {
	return pointer_->value;
    }
    else
    {
	throw invalid_dereference("cannot dereference invalid emplacing_list iterator");
    }
}

template <class value_t, class node_t>
value_t* basic_safe_forward<value_t, node_t>::operator->()
{
    if (is_valid())
    {
	return &(pointer_->value);
    }
    else
    {
	throw invalid_dereference("cannot dereference emplacing_list iterator");
    }
}

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>& basic_safe_forward<value_t, node_t>::operator++()
{
    if (!is_valid())
    {
	return *this;
    }
    if (pointer_->alive)
    {
	pointer_ = pointer_->next;
    }
    else if (!pointer_->previous.expired())
    {
	std::shared_ptr<node_t> previous_node = pointer_->previous.lock();
	pointer_ = previous_node->next;
    }
    return *this;
}

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t> basic_safe_forward<value_t, node_t>::operator++(int)
{
    basic_safe_forward<value_t, node_t> tmp = *this;
    ++(*this);
    return tmp;
}

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t>& basic_safe_forward<value_t, node_t>::operator--()
{
    if (is_valid())
    {
	if (!pointer_->previous.expired())
	{
	    pointer_ = pointer_->previous.lock();
	}
	else
	{
	    pointer_.reset();
	}
    }
    return *this;
}

template <class value_t, class node_t>
basic_safe_forward<value_t, node_t> basic_safe_forward<value_t, node_t>::operator--(int)
{
    if (is_valid())
    {
	basic_safe_forward<value_t, node_t> tmp = *this;
	--(*this);
	return tmp;
    }
    else
    {
	return *this;
    }
}

} // namespace emplacing_list_iterator

template <class value_t, class typed_allocator_t>
template <class... args_t>
emplacing_list<value_t, typed_allocator_t>::node::node(args_t&&... args)
    :
	value(std::forward<args_t>(args)...),
	alive(true),
	next(nullptr),
	previous()
{ }

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::emplacing_list(typed_allocator_type& allocator)
    :
	allocator_(allocator),
	size_(0U),
	front_(nullptr),
	back_()
{ }

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::~emplacing_list()
{
    for (auto iter = cbegin(); iter != cend(); iter = erase(iter)) { }
    front_.reset();
    back_.reset();
}

template <class value_t, class typed_allocator_t>
template <class... args_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator emplacing_list<value_t, typed_allocator_t>::emplace_front(args_t&&... args)
{
    std::shared_ptr<node> old_front = front_;
    std::shared_ptr<node> new_front = create_node(std::forward<args_t>(args)...);
    if (front_.use_count() != 0)
    {
	new_front->next = front_;
    }
    front_ = new_front;
    ++size_;
    if (old_front.use_count() != 0)
    {
	old_front->previous = new_front;
    }
    if (back_.expired())
    {
	back_ = new_front;
    }
    return new_front;
}

template <class value_t, class typed_allocator_t>
template <class... args_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator emplacing_list<value_t, typed_allocator_t>::emplace_back(args_t&&... args)
{
    std::weak_ptr<node> old_back = back_;
    std::shared_ptr<node> new_back = create_node(std::forward<args_t>(args)...);
    if (!back_.expired())
    {
	new_back->previous = back_.lock();
    }
    back_ = new_back;
    ++size_;
    if (!old_back.expired())
    {
	old_back.lock()->next = new_back;
    }
    if (front_.use_count() == 0)
    {
	front_ = new_back;
    }
    return new_back;
}

template <class value_t, class typed_allocator_t>
template <class... args_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator emplacing_list<value_t, typed_allocator_t>::emplace(const_iterator new_next, args_t&&... args)
{
    if (!new_next.ptr())
    {
	if (size_ == 0U)
	{
	    emplace_front(std::forward<args_t>(args)...);
	    return front_;
	}
	else
	{
	    emplace_back(std::forward<args_t>(args)...);
	    return back_.lock();
	}
    }
    else
    {
	if (new_next.ptr()->previous.expired())
	{
	    emplace_front(std::forward<args_t>(args)...);
	    return front_;
	}
	else
	{
	    std::shared_ptr<node> new_previous = new_next.ptr()->previous.lock();
	    std::shared_ptr<node> new_node = create_node(std::forward<args_t>(args)...);
	    new_node->next = new_next.ptr();
	    new_node->previous = new_previous;
	    new_previous->next = new_node;
	    new_next.ptr()->previous = new_node;
	    ++size_;
	    return new_node;
	}
    }
}

template <class value_t, class typed_allocator_t>
void emplacing_list<value_t, typed_allocator_t>::pop_front()
{
    if (front_.use_count() == 0)
    {
	return;
    }
    std::shared_ptr<node> old_front = front_;
    old_front->alive = false;
    std::shared_ptr<node> next_node = old_front->next;
    front_ = old_front->next;
    old_front->next.reset();
    if (next_node.use_count() != 0)
    {
	next_node->previous.reset();
    }
    old_front->previous = next_node;
    --size_;
    if (size_ == 0U)
    {
	back_.reset();
    }
}

template <class value_t, class typed_allocator_t>
void emplacing_list<value_t, typed_allocator_t>::pop_back()
{
    if (back_.expired())
    {
	return;
    }
    std::shared_ptr<node> old_back = back_.lock();
    old_back->alive = false;
    if (!old_back->previous.expired())
    {
	std::shared_ptr<node> previous_node = old_back->previous.lock();
	previous_node->next.reset();
	old_back->next.reset();
	back_ = previous_node;
    }
    else
    {
	old_back->next.reset();
	back_.reset();
    }
    --size_;
    if (size_ == 0U)
    {
	front_.reset();
    }
}

template <class value_t, class typed_allocator_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator emplacing_list<value_t, typed_allocator_t>::erase(const_iterator position)
{
    std::shared_ptr<node> erase_node = position.ptr();
    if (size_ == 0U || !erase_node)
    {
	return end();
    }
    else
    {
	if (erase_node->previous.expired())
	{
	    pop_front();
	    return begin();
	}
	else if (!erase_node->next)
	{
	    pop_back();
	    return end();
	}
	else
	{
	    erase_node->alive = false;
	    std::shared_ptr<node> previous_node = erase_node->previous.lock();
	    std::shared_ptr<node> next_node = erase_node->next;
	    erase_node->next.reset();
	    previous_node->next = next_node;
	    next_node->previous = previous_node;
	    --size_;
	    return next_node;
	}
    }
}

template <class value_t, class typed_allocator_t>
template <class... args_t>
std::shared_ptr<typename emplacing_list<value_t, typed_allocator_t>::node> emplacing_list<value_t, typed_allocator_t>::create_node(args_t&&... args)
{
    node* tmp = allocator_.template allocate<node>();
    if (tmp != nullptr)
    {
	return std::shared_ptr<node>(
		new (tmp) node(std::forward<args_t>(args)...),
		std::bind(&emplacing_list<value_t, typed_allocator_t>::destroy_node, this, std::placeholders::_1));
    }
    else
    {
	throw std::runtime_error("Out of memory");
    }
}

template <class value_t, class typed_allocator_t>
void emplacing_list<value_t, typed_allocator_t>::destroy_node(node* pointer)
{
    pointer->~node();
    allocator_.template deallocate<node>(pointer);
}

} // namespace container
} // namespace turbo

#endif
