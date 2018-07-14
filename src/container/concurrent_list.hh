#ifndef TURBO_CONTAINER_CONCURRENT_LIST_HXX
#define TURBO_CONTAINER_CONCURRENT_LIST_HXX

#include <stdexcept>
#include <utility>
#include <turbo/container/concurrent_list.hpp>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {

template <class value_t, class typed_allocator_t>
template <class... args_t>
concurrent_list<value_t, typed_allocator_t>::node::node(args_t&&... args)
    :
	value(std::forward<args_t>(args)...),
	next(),
	previous()
{ }

template <class value_t, class typed_allocator_t>
concurrent_list<value_t, typed_allocator_t>::concurrent_list(typed_allocator_type& allocator)
    :
	allocator_(allocator)
{ }

template <class value_t, class typed_allocator_t>
template <class... args_t>
typename concurrent_list<value_t, typed_allocator_t>::node* concurrent_list<value_t, typed_allocator_t>::create_node(args_t&&... args)
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
