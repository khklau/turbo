#ifndef TURBO_CONTAINER_EMPLACING_SKIPLIST_HXX
#define TURBO_CONTAINER_EMPLACING_SKIPLIST_HXX

#include <turbo/container/emplacing_skiplist.hpp>
#include <turbo/container/emplacing_list.hxx>

namespace turbo {
namespace container {

template <class key_t, class value_t, class typed_allocator_t>
emplacing_skiplist<key_t, value_t, typed_allocator_t>::emplacing_skiplist(typed_allocator_type& allocator)
    :
	allocator_(allocator),
	base_(allocator_),
	tower_(allocator_)
{ }

} // namespace container
} // namespace turbo

#endif

