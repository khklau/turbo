#ifndef TURBO_CONTAINER_EMPLACING_LIST_HPP
#define TURBO_CONTAINER_EMPLACING_LIST_HPP

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>
#include <turbo/memory/typed_allocator.hpp>

namespace turbo {
namespace container {

namespace emplacing_list_iterator {

template <class value_t, class node_t>
class basic_forward : public std::bidirectional_iterator_tag
{
public:
    typedef value_t value_type;
    typedef value_t* pointer;
    typedef value_t& reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef node_t node_type;
    basic_forward();
    basic_forward(node_type* pointer);
    basic_forward(const basic_forward& other);
    basic_forward(basic_forward&& other);
    template <class other_value_t>
    basic_forward(const basic_forward<other_value_t, node_t>& other);
    basic_forward& operator=(const basic_forward& other);
    basic_forward& operator=(basic_forward&& other);
    template <class other_value_t>
    basic_forward& operator=(const basic_forward<other_value_t, node_t>& other);
    basic_forward& operator=(node_type* other);
    ~basic_forward() = default;
    bool operator==(const basic_forward& other) const;
    inline bool operator!=(const basic_forward& other) const { return !(*this == other); }
    value_t& operator*();
    value_t* operator->();
    basic_forward& operator++();
    basic_forward operator++(int);
    basic_forward& operator--();
    basic_forward operator--(int);
    inline node_type* ptr() const { return pointer_; }
    inline bool is_valid() const { return pointer_ != nullptr; }
    inline bool is_first() const { return is_valid() && pointer_->is_first(); }
    inline bool is_last() const { return is_valid() && pointer_->is_last(); }
private:
    node_type* pointer_;
};

template <class value_t, class node_t>
class basic_reverse : private basic_forward<value_t, node_t>
{
public:
    typedef basic_forward<value_t, node_t> base_iterator;
    typedef value_t value_type;
    typedef value_t* pointer;
    typedef value_t& reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef node_t node_type;
    basic_reverse() = default;
    inline basic_reverse(node_type* pointer)
	:
	    base_iterator(pointer)
    { }
    inline basic_reverse(const basic_reverse& other)
	:
	    base_iterator(static_cast<const base_iterator&>(other))
    { }
    inline basic_reverse(basic_reverse&& other)
	:
	    base_iterator(std::forward<base_iterator>(other))
    { }
    template <class other_value_t>
    inline basic_reverse(const basic_reverse<other_value_t, node_t>& other)
	:
	    base_iterator(static_cast<const typename basic_reverse<other_value_t, node_t>::base_iterator&>(other))
    { }
    basic_reverse& operator=(const basic_reverse& other)
    {
	return static_cast<const base_iterator&>(*this) = static_cast<const base_iterator&>(other);
    }
    basic_reverse& operator=(basic_reverse&& other)
    {
	base_iterator::operator=(std::forward<base_iterator>(other));
	return *this;
    }
    template <class other_value_t>
    basic_reverse& operator=(const basic_reverse<other_value_t, node_t>& other)
    {
	return static_cast<const base_iterator&>(*this) = static_cast<const typename basic_reverse<other_value_t, node_t>::base_iterator&>(other);
    }
    basic_reverse& operator=(node_type* other)
    {
	return static_cast<const base_iterator&>(*this) = other;
    }
    ~basic_reverse() = default;
    inline bool operator==(const basic_reverse& other) const
    {
	return static_cast<const base_iterator&>(*this) == static_cast<const base_iterator&>(other);
    }
    inline bool operator!=(const basic_reverse& other) const
    {
	return !(*this == other);
    }
    using base_iterator::operator*;
    using base_iterator::operator->;
    inline basic_reverse& operator++()
    {
	base_iterator::operator--();
	return *this;
    }
    inline basic_reverse operator++(int)
    {
	base_iterator::operator--(0);
	return *this;
    }
    inline basic_reverse& operator--()
    {
	base_iterator::operator++();
	return *this;
    }
    inline basic_reverse operator--(int)
    {
	base_iterator::operator++(0);
	return *this;
    }
    using base_iterator::is_valid;
    inline bool is_first() const
    {
	return base_iterator::is_last();
    }
    inline bool is_last() const
    {
	return base_iterator::is_first();
    }
};

} // namespace emplacing_list_iterator

template <class value_t, class typed_allocator_t = turbo::memory::typed_allocator>
class emplacing_list
{
private:
    struct node;
public:
    typedef value_t value_type;
    typedef typed_allocator_t typed_allocator_type;
    typedef emplacing_list_iterator::basic_forward<const value_t, node> const_iterator;
    typedef emplacing_list_iterator::basic_forward<value_t, node> iterator;
    typedef emplacing_list_iterator::basic_reverse<const value_t, node> const_reverse_iterator;
    typedef emplacing_list_iterator::basic_reverse<value_t, node> reverse_iterator;
    static constexpr std::size_t allocation_size()
    {
	return sizeof(node);
    }
    static constexpr std::size_t allocation_alignment()
    {
	return alignof(node);
    }
    explicit emplacing_list(typed_allocator_type& allocator);
    ~emplacing_list();
    inline std::size_t size() const noexcept
    {
	return size_;
    }
    inline iterator begin() noexcept
    {
	return iterator(front_);
    }
    inline iterator end() noexcept
    {
	return iterator();
    }
    inline const_iterator cbegin() noexcept
    {
	return const_iterator(front_);
    }
    inline const_iterator cend() noexcept
    {
	return const_iterator();
    }
    inline reverse_iterator rbegin() noexcept
    {
	return reverse_iterator(back_);
    }
    inline reverse_iterator rend() noexcept
    {
	return reverse_iterator();
    }
    inline const_reverse_iterator crbegin() noexcept
    {
	return const_reverse_iterator(back_);
    }
    inline const_reverse_iterator crend() noexcept
    {
	return const_reverse_iterator();
    }
    template <class... args_t>
    iterator emplace_front(args_t&&... args);
    template <class... args_t>
    iterator emplace_back(args_t&&... args);
    template <class... args_t>
    iterator emplace(const_iterator new_next, args_t&&... args);
    void pop_front();
    void pop_back();
    iterator erase(const_iterator position);
private:
    struct node
    {
	template <class... args_t>
	node(args_t&&... args);
	inline bool is_first() const { return previous == nullptr; }
	inline bool is_last() const { return next == nullptr; }
	value_t value;
	bool alive;
	node* next;
	node* previous;
    };
    template <class... args_t>
    node* create_node(args_t&&... args);
    void destroy_node(node* pointer);
    typed_allocator_type& allocator_;
    std::size_t size_;
    node* front_;
    node* back_;
};

} // namespace container
} // namespace turbo

#endif
