#ifndef TURBO_CONTAINER_CONCURRENT_VECTOR_HXX
#define TURBO_CONTAINER_CONCURRENT_VECTOR_HXX

#include <turbo/container/concurrent_vector.hpp>
#include <limits>
#include <turbo/math/power.hpp>
#include <turbo/math/summation.hpp>
#include <turbo/toolset/extension.hpp>
#include <turbo/toolset/intrinsic.hpp>

namespace turbo {
namespace container {

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::concurrent_vector(
	std::uint8_t initial_capacity_exponent,
	std::uint8_t max_capacity_exponent)
    :
	initial_exponent_(initial_capacity_exponent),
	max_exponent_(max_capacity_exponent),
	segments_()
{
    if (std::numeric_limits<capacity_type>::digits < max_exponent_)
    {
	throw invalid_capacity_argument("Maximum capacity exponent cannot exceed the number of digit bits in capacity_type");
    }
    else if (max_exponent_ < initial_exponent_)
    {
	throw invalid_capacity_argument("Maximum capacity cannot be less than initial capacity");
    }
    else
    {
	segments_.reset(new std::atomic<node*>[max_exponent_ - initial_exponent_]);
	std::size_t first_size = turbo::math::pow(capacity_base_, initial_exponent_);
	segments_[0] = new node[first_size];
	for (std::size_t iter = 0U; iter < first_size; ++iter)
	{
	    segments_[0][iter].guard = node::status::ready;
	}
	std::size_t range = max_exponent_ - initial_exponent_;
	for (std::size_t iter = 1U; iter < range; ++iter)
	{
	    segments_[iter] = nullptr;
	}
    }
}

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::~concurrent_vector()
{
    std::size_t range = max_exponent_ - initial_exponent_;
    for (std::size_t iter = 0U; iter < range; ++iter)
    {
	node* tmp = segments_[iter].load(std::memory_order_acquire);
	if (tmp != nullptr)
	{
	    delete[] tmp;
	    segments_[iter].store(nullptr, std::memory_order_release);
	}
    }
}

template <class value_t, template <class type_t> class allocator_t>
value_t& concurrent_vector<value_t, allocator_t>::operator[](capacity_type index)
{
    subscript_type subscript = find_subscript(index);
    return segments_[subscript.first][subscript.second].value;
}

template <class value_t, template <class type_t> class allocator_t>
const value_t& concurrent_vector<value_t, allocator_t>::operator[](capacity_type index) const
{
    subscript_type subscript = find_subscript(index);
    return segments_[subscript.first][subscript.second].value;
}

template <class value_t, template <class type_t> class allocator_t>
value_t& concurrent_vector<value_t, allocator_t>::at(capacity_type index)
{
    range_check(index);
    return (*this)[index];
}


template <class value_t, template <class type_t> class allocator_t>
const value_t& concurrent_vector<value_t, allocator_t>::at(capacity_type index) const
{
    range_check(index);
    return (*this)[index];
}

template <class value_t, template <class type_t> class allocator_t>
typename concurrent_vector<value_t, allocator_t>::subscript_type concurrent_vector<value_t, allocator_t>::find_subscript(capacity_type index) const
{
    if (index == 0)
    {
	return std::make_pair(0, 0);
    }
    capacity_type high_bit = std::numeric_limits<capacity_type>::digits - turbo::toolset::count_leading_zero(index) - 1U;
    if (high_bit < initial_exponent_)
    {
	return std::make_pair(0, index);
    }
    capacity_type segment = high_bit - initial_exponent_ + 1U;
    capacity_type position = index ^ (turbo::math::pow(capacity_base_, high_bit));
    return std::make_pair(segment, position);
}

template <class value_t, template <class type_t> class allocator_t>
void concurrent_vector<value_t, allocator_t>::range_check(capacity_type index) const
{
    subscript_type subscript = find_subscript(index);
    const std::size_t range = max_exponent_ - initial_exponent_;
    if (range < subscript.first)
    {
	throw std::out_of_range("Requested element is not in range");
    }
    else if (segments_[subscript.first] == nullptr)
    {
	throw std::out_of_range("Requested element is not in range");
    }
}

} // namespace container
} // namespace turbo

#endif
