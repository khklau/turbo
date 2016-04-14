#ifndef TURBO_TYPE_UTILITY_ENUM_ITERATOR_HXX
#define TURBO_TYPE_UTILITY_ENUM_ITERATOR_HXX

#include <turbo/type_utility/enum_iterator.hpp>

namespace turbo {
namespace type_utility {

template <class enum_t, enum_t first, enum_t last>
template <class value_t>
enum_iterator<enum_t, first, last>::basic_iterator<value_t>::basic_iterator() :
	value_(static_cast<underlying_type>(last) + 1)
{ }

template <class enum_t, enum_t first, enum_t last>
template <class value_t>
enum_iterator<enum_t, first, last>::basic_iterator<value_t>::basic_iterator(value_t value) :
	value_(static_cast<underlying_type>(value))
{ }

template <class enum_t, enum_t first, enum_t last>
template <class value_t>
bool enum_iterator<enum_t, first, last>::basic_iterator<value_t>::operator==(const basic_iterator<value_t>& other)
{
    return value_ == other.value_;
}

template <class enum_t, enum_t first, enum_t last>
template <class value_t>
bool enum_iterator<enum_t, first, last>::basic_iterator<value_t>::operator!=(const basic_iterator<value_t>& other)
{
    return !(*this == other);
}

template <class enum_t, enum_t first, enum_t last>
template <class value_t>
enum_iterator<enum_t, first, last>::basic_iterator<value_t>& enum_iterator<enum_t, first, last>::basic_iterator<value_t>::operator++()
{
    ++value_;
    return *this;
}

template <class enum_t, enum_t first, enum_t last>
template <class value_t>
value_t enum_iterator<enum_t, first, last>::basic_iterator<value_t>::operator*()
{
    return static_cast<value_t>(value_);
}

template <class enum_t, enum_t first, enum_t last>
typename enum_iterator<enum_t, first, last>::iterator enum_iterator<enum_t, first, last>::begin()
{
    return iterator(first);
}

template <class enum_t, enum_t first, enum_t last>
typename enum_iterator<enum_t, first, last>::iterator enum_iterator<enum_t, first, last>::end()
{
    return iterator();
}

template <class enum_t, enum_t first, enum_t last>
typename enum_iterator<enum_t, first, last>::const_iterator enum_iterator<enum_t, first, last>::cbegin() const
{
    return const_iterator(first);
}

template <class enum_t, enum_t first, enum_t last>
typename enum_iterator<enum_t, first, last>::const_iterator enum_iterator<enum_t, first, last>::cend() const
{
    return const_iterator();
}

} // namespace type_utility
} // namespace turbo

#endif
