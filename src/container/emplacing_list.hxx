#ifndef TURBO_CONTAINER_EMPLACING_LIST_HXX
#define TURBO_CONTAINER_EMPLACING_LIST_HXX

#include <turbo/container/emplacing_list.hpp>
#include <stdexcept>
#include <utility>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::iterator::iterator()
    :
	pointer_()
{ }

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::iterator::iterator(const std::shared_ptr<node>& pointer)
    :
	pointer_(pointer)
{ }

template <class value_t, class typed_allocator_t>
emplacing_list<value_t, typed_allocator_t>::iterator::iterator(const iterator& other)
    :
	pointer_(other.pointer_)
{ }

template <class value_t, class typed_allocator_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator& emplacing_list<value_t, typed_allocator_t>::iterator::operator=(const iterator& other)
{
    if (TURBO_LIKELY(this != &other))
    {
	pointer_ = other.pointer_;
    }
    return *this;
}

template <class value_t, class typed_allocator_t>
bool emplacing_list<value_t, typed_allocator_t>::iterator::operator==(const iterator& other) const
{
    return pointer_ == other.pointer_;
}

template <class value_t, class typed_allocator_t>
value_t& emplacing_list<value_t, typed_allocator_t>::iterator::operator*()
{
    if (TURBO_LIKELY(is_valid()))
    {
	return pointer_->value;
    }
    else
    {
	throw invalid_dereference("cannot dereference invalid emplacing_list::iterator");
    }
}

template <class value_t, class typed_allocator_t>
value_t* emplacing_list<value_t, typed_allocator_t>::iterator::operator->()
{
    if (TURBO_LIKELY(is_valid()))
    {
	return &(pointer_->value);
    }
    else
    {
	throw invalid_dereference("cannot dereference invalid emplacing_list::iterator");
    }
}

template <class value_t, class typed_allocator_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator& emplacing_list<value_t, typed_allocator_t>::iterator::operator++()
{
    if (TURBO_LIKELY(is_valid()))
    {
	pointer_ = pointer_->next;
    }
    return *this;
}

template <class value_t, class typed_allocator_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator emplacing_list<value_t, typed_allocator_t>::iterator::operator++(int)
{
    if (TURBO_LIKELY(is_valid()))
    {
	iterator tmp = *this;
	++(*this);
	return tmp;
    }
    else
    {
	return *this;
    }
}

template <class value_t, class typed_allocator_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator& emplacing_list<value_t, typed_allocator_t>::iterator::operator--()
{
    if (is_valid() && !pointer_.expired())
    {
	pointer_ = pointer_->previous.lock();
    }
    return *this;
}

template <class value_t, class typed_allocator_t>
typename emplacing_list<value_t, typed_allocator_t>::iterator emplacing_list<value_t, typed_allocator_t>::iterator::operator--(int)
{
    if (TURBO_LIKELY(is_valid()))
    {
	iterator tmp = *this;
	--(*this);
	return tmp;
    }
    else
    {
	return *this;
    }
}

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
	head_(),
	tail_()
{ }

template <class value_t, class typed_allocator_t>
template <class... args_t>
typename emplacing_list<value_t, typed_allocator_t>::node* emplacing_list<value_t, typed_allocator_t>::create_node(args_t&&... args)
{
    node* tmp = allocator_.template allocate<node>();
    if (TURBO_LIKELY(tmp != nullptr))
    {
	return new (tmp) node(std::forward<args_t>(args)...);
    }
    else
    {
	throw std::runtime_error("Out of memory");
    }
}

} // namespace container
} // namespace turbo

#endif
