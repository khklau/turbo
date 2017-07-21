#ifndef TURBO_MEMORY_BLOCK_HXX
#define TURBO_MEMORY_BLOCK_HXX

#include <turbo/memory/block.hpp>
#include <cstring>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/memory/alignment.hpp>
#include <turbo/memory/alignment.hxx>
#include <turbo/toolset/extension.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>

namespace turbo {
namespace memory {

template <class b, class n>
block_list::basic_iterator<b, n>::basic_iterator()
    :
	pointer_(nullptr)
{ }

template <class b, class n>
block_list::basic_iterator<b, n>::basic_iterator(typename block_list::basic_iterator<b, n>::node_type* pointer)
    :
	pointer_(pointer)
{ }

template <class b, class n>
block_list::basic_iterator<b, n>::basic_iterator(const basic_iterator& other)
    :
	pointer_(other.pointer_)
{ }

template <class b, class n>
block_list::basic_iterator<b, n>& block_list::basic_iterator<b, n>::operator=(const basic_iterator& other)
{
    if (TURBO_LIKELY(this != &other))
    {
	pointer_ = other.pointer_;
    }
    return *this;
}

template <class b, class n>
bool block_list::basic_iterator<b, n>::operator==(const basic_iterator& other) const
{
    return pointer_ == other.pointer_;
}

template <class b, class n>
block_list::basic_iterator<b, n>& block_list::basic_iterator<b, n>::operator++()
{
    if (TURBO_LIKELY(is_valid()))
    {
	node* next = pointer_->get_next().load(std::memory_order_acquire);
	pointer_ = next;
    }
    return *this;
}

template <class b, class n>
typename block_list::basic_iterator<b, n> block_list::basic_iterator<b, n>::operator++(int)
{
    if (TURBO_LIKELY(is_valid()))
    {
	basic_iterator<b, n> tmp = *this;
	++(*this);
	return tmp;
    }
    else
    {
	return *this;
    }
}

} // namespace memory
} // namespace turbo

#endif
