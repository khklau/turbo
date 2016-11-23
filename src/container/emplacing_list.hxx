#ifndef TURBO_CONTAINER_EMPLACING_LIST_HXX
#define TURBO_CONTAINER_EMPLACING_LIST_HXX

#include <turbo/container/emplacing_list.hpp>
#include <stdexcept>
#include <utility>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {

namespace emplacing_list_iterator {

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward()
    :
	pointer_()
{ }

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward(const std::shared_ptr<node_t>& pointer)
    :
	pointer_(pointer)
{ }

template <class value_t, class node_t>
basic_forward<value_t, node_t>::basic_forward(const basic_forward& other)
    :
	pointer_(other.pointer_)
{ }

template <class value_t, class node_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator=(const basic_forward& other)
{
    if (TURBO_LIKELY(this != &other))
    {
	pointer_ = other.pointer_;
    }
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
    if (TURBO_LIKELY(is_valid()))
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
    if (TURBO_LIKELY(is_valid()))
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
    if (TURBO_LIKELY(is_valid()))
    {
	pointer_ = pointer_->next;
    }
    return *this;
}

template <class value_t, class node_t>
basic_forward<value_t, node_t> basic_forward<value_t, node_t>::operator++(int)
{
    if (TURBO_LIKELY(is_valid()))
    {
	basic_forward<value_t, node_t> tmp = *this;
	++(*this);
	return tmp;
    }
    else
    {
	return *this;
    }
}

template <class value_t, class node_t>
basic_forward<value_t, node_t>& basic_forward<value_t, node_t>::operator--()
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
basic_forward<value_t, node_t> basic_forward<value_t, node_t>::operator--(int)
{
    if (TURBO_LIKELY(is_valid()))
    {
	basic_forward<value_t, node_t> tmp = *this;
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
	next(),
	previous()
{ }

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::emplacing_list(typed_allocator_type& allocator)
    :
	allocator_(allocator),
	size_(0U),
	front_(),
	back_()
{ }

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::~emplacing_list()
{
    std::shared_ptr<node> current = front_;
    for (front_.reset(); current.use_count() != 0; current = current->next)
    {
	current->previous.reset();
    }
    back_.reset();
}

template <class value_t, class typed_allocator_t>
template <class... args_t>
void emplacing_list<value_t, typed_allocator_t>::emplace_front(args_t&&... args)
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
}

template <class value_t, class typed_allocator_t>
template <class... args_t>
void emplacing_list<value_t, typed_allocator_t>::emplace_back(args_t&&... args)
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
}

template <class value_t, class typed_allocator_t>
template <class... args_t>
std::shared_ptr<typename emplacing_list<value_t, typed_allocator_t>::node> emplacing_list<value_t, typed_allocator_t>::create_node(args_t&&... args)
{
    node* tmp = allocator_.template allocate<node>();
    if (TURBO_LIKELY(tmp != nullptr))
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
