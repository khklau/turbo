#ifndef TURBO_TYPE_UTILITY_ENUM_ITERATOR_HPP
#define TURBO_TYPE_UTILITY_ENUM_ITERATOR_HPP

#include <iterator>
#include <type_traits>

namespace turbo {
namespace type_utility {

template <class enum_t, enum_t first, enum_t last>
class enum_iterator
{
public:
    template <class value_t>
    class basic_iterator : public std::iterator<std::forward_iterator_tag, value_t>
    {
    public:
	typedef value_t value_type;
	basic_iterator();
	basic_iterator(value_t value);
	bool operator==(const basic_iterator<value_t>& other);
	bool operator!=(const basic_iterator<value_t>& other);
	basic_iterator<value_t>& operator++();
	value_t operator*();
    private:
	typedef typename std::add_const<value_t>::type const_type;
	typedef typename std::underlying_type<value_t>::type underlying_type;
	const_type first_ = first;
	const_type last_ = last;
	underlying_type value_;
    };
    typedef basic_iterator<enum_t> iterator;
    typedef basic_iterator<const enum_t> const_iterator;
    iterator begin();
    iterator end();
    const_iterator cbegin() const;
    const_iterator cend() const;
};

} // namespace type_utility
} // namespace turbo

#endif
