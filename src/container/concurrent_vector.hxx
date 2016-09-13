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
	segments_(new std::atomic<value_t*>[max_exponent_ - initial_exponent_ + 1U])
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
	segments_[0] = new value_t[turbo::math::pow(capacity_base_, initial_exponent_)];
    }
}

template <class value_t, template <class type_t> class allocator_t>
concurrent_vector<value_t, allocator_t>::~concurrent_vector()
{
    std::size_t range = max_exponent_ - initial_exponent_ + 1U;
    for (std::size_t iter = 0U; iter < range; ++iter)
    {
	value_t* tmp = segments_[iter].load(std::memory_order_acquire);
	if (tmp != nullptr)
	{
	    delete tmp;
	}
	segments_[iter].store(nullptr, std::memory_order_release);
    }
}

template <class value_t, template <class type_t> class allocator_t>
value_t& concurrent_vector<value_t, allocator_t>::operator[](capacity_type index)
{
    const value_t& tmp = (*this)[index];
    return const_cast<value_t&>(tmp);
}

template <class value_t, template <class type_t> class allocator_t>
const value_t& concurrent_vector<value_t, allocator_t>::operator[](capacity_type index) const
{
    if (index == 0)
    {
	return segments_[0][0];
    }
    capacity_type high_bit = std::numeric_limits<capacity_type>::digits - turbo::toolset::count_leading_zero(index) - 1U;
    if (high_bit < initial_exponent_)
    {
	return segments_[0][index];
    }
    capacity_type segment = high_bit - initial_exponent_ + 1U;
    capacity_type position = index ^ (turbo::math::pow(capacity_base_, high_bit));
    return segments_[segment][position];
}

template <class value_t, template <class type_t> class allocator_t>
value_t& concurrent_vector<value_t, allocator_t>::at(capacity_type index)
{
    const value_t& tmp = at(index);
    return const_cast<value_t&>(tmp);
}

template <class value_t, template <class type_t> class allocator_t>
const value_t& concurrent_vector<value_t, allocator_t>::at(capacity_type index) const
{
    if (index == 0)
    {
	if (segments_[0] != nullptr)
	{
	    return segments_[0][0];
	}
	else
	{
	    return std::out_of_range("First segment is null");
	}
    }
    capacity_type high_bit = std::numeric_limits<capacity_type>::digits - turbo::toolset::count_leading_zero(index) - 1U;
    if (high_bit < initial_exponent_)
    {
	if (segments_[0] != nullptr)
	{
	    return segments_[0][index];
	}
	else
	{
	    return std::out_of_range("First segment is null");
	}
    }
    capacity_type segment = high_bit - initial_exponent_ + 1U;
    capacity_type position = index ^ (turbo::math::pow(capacity_base_, high_bit));
    if (segments_[segment] != nullptr)
    {
	return segments_[segment][position];
    }
    else
    {
	return std::out_of_range("Requested segment is null");
    }
}

} // namespace container
} // namespace turbo

#endif
