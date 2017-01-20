#ifndef TURBO_CONTAINER_EMPLACING_LIST_HPP
#define TURBO_CONTAINER_EMPLACING_LIST_HPP

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <turbo/memory/typed_allocator.hpp>

namespace turbo {
namespace container {

namespace emplacing_list_iterator {

template <class value_t, class node_t>
class basic_safe_forward : public std::bidirectional_iterator_tag
{
public:
    typedef value_t value_type;
    typedef value_t* pointer;
    typedef value_t& reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef node_t node_type;
    basic_safe_forward();
    basic_safe_forward(const std::shared_ptr<node_t>& pointer);
    basic_safe_forward(const basic_safe_forward& other);
    basic_safe_forward(basic_safe_forward&& other);
    template <class other_value_t>
    basic_safe_forward(const basic_safe_forward<other_value_t, node_t>& other);
    basic_safe_forward& operator=(const basic_safe_forward& other);
    basic_safe_forward& operator=(basic_safe_forward&& other);
    template <class other_value_t>
    basic_safe_forward& operator=(const basic_safe_forward<other_value_t, node_t>& other);
    basic_safe_forward& operator=(const std::shared_ptr<node_t>& other);
    ~basic_safe_forward() = default;
    bool operator==(const basic_safe_forward& other) const;
    inline bool operator!=(const basic_safe_forward& other) const { return !(*this == other); }
    value_t& operator*();
    value_t* operator->();
    basic_safe_forward& operator++();
    basic_safe_forward operator++(int);
    basic_safe_forward& operator--();
    basic_safe_forward operator--(int);
    inline const std::shared_ptr<node_t>& ptr() const { return pointer_; }
    inline std::weak_ptr<node_t> share() const { return pointer_; }
    inline bool is_valid() const { return pointer_.use_count() != 0; }
    inline bool is_first() const { return is_valid() && pointer_->is_first(); }
    inline bool is_last() const { return is_valid() && pointer_->is_last(); }
private:
    std::shared_ptr<node_t> pointer_;
};

template <class value_t, class node_t>
class basic_safe_reverse : private basic_safe_forward<value_t, node_t>
{
public:
    typedef basic_safe_forward<value_t, node_t> base_iterator;
    basic_safe_reverse() = default;
    inline basic_safe_reverse(const std::shared_ptr<node_t>& pointer)
	:
	    base_iterator(pointer)
    { }
    inline basic_safe_reverse(const basic_safe_reverse& other)
	:
	    base_iterator(static_cast<const base_iterator&>(other))
    { }
    inline basic_safe_reverse(basic_safe_reverse&& other)
	:
	    base_iterator(std::forward<base_iterator>(other))
    { }
    template <class other_value_t>
    inline basic_safe_reverse(const basic_safe_reverse<other_value_t, node_t>& other)
	:
	    base_iterator(static_cast<const typename basic_safe_reverse<other_value_t, node_t>::base_iterator&>(other))
    { }
    basic_safe_reverse& operator=(const basic_safe_reverse& other)
    {
	return static_cast<const base_iterator&>(*this) = static_cast<const base_iterator&>(other);
    }
    basic_safe_reverse& operator=(basic_safe_reverse&& other)
    {
	base_iterator::operator=(std::forward<base_iterator>(other));
	return *this;
    }
    template <class other_value_t>
    basic_safe_reverse& operator=(const basic_safe_reverse<other_value_t, node_t>& other)
    {
	return static_cast<const base_iterator&>(*this) = static_cast<const typename basic_safe_reverse<other_value_t, node_t>::base_iterator&>(other);
    }
    basic_safe_reverse& operator=(const std::shared_ptr<node_t>& other)
    {
	return static_cast<const base_iterator&>(*this) = other;
    }
    ~basic_safe_reverse() = default;
    inline bool operator==(const basic_safe_reverse& other) const
    {
	return static_cast<const base_iterator&>(*this) == static_cast<const base_iterator&>(other);
    }
    inline bool operator!=(const basic_safe_reverse& other) const
    {
	return !(*this == other);
    }
    using base_iterator::operator*;
    using base_iterator::operator->;
    inline basic_safe_reverse& operator++()
    {
	base_iterator::operator--();
	return *this;
    }
    inline basic_safe_reverse operator++(int)
    {
	base_iterator::operator--(0);
	return *this;
    }
    inline basic_safe_reverse& operator--()
    {
	base_iterator::operator++();
	return *this;
    }
    inline basic_safe_reverse operator--(int)
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

template <class value_t, class node_t>
class basic_unsafe_forward : public std::bidirectional_iterator_tag
{
public:
    typedef value_t value_type;
    typedef value_t* pointer;
    typedef value_t& reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef node_t node_type;
    basic_unsafe_forward();
    basic_unsafe_forward(node_type* pointer);
    basic_unsafe_forward(const basic_unsafe_forward& other);
    basic_unsafe_forward(basic_unsafe_forward&& other);
    template <class other_value_t>
    basic_unsafe_forward(const basic_unsafe_forward<other_value_t, node_t>& other);
    basic_unsafe_forward& operator=(const basic_unsafe_forward& other);
    basic_unsafe_forward& operator=(basic_unsafe_forward&& other);
    template <class other_value_t>
    basic_unsafe_forward& operator=(const basic_unsafe_forward<other_value_t, node_t>& other);
    basic_unsafe_forward& operator=(node_type* other);
    ~basic_unsafe_forward() = default;
    bool operator==(const basic_unsafe_forward& other) const;
    inline bool operator!=(const basic_unsafe_forward& other) const { return !(*this == other); }
    value_t& operator*();
    value_t* operator->();
    basic_unsafe_forward& operator++();
    basic_unsafe_forward operator++(int);
    basic_unsafe_forward& operator--();
    basic_unsafe_forward operator--(int);
    inline node_type* ptr() const { return pointer_; }
    inline bool is_valid() const { return pointer_ != nullptr; }
    inline bool is_first() const { return is_valid() && pointer_->is_first(); }
    inline bool is_last() const { return is_valid() && pointer_->is_last(); }
private:
    node_type* pointer_;
};

template <class value_t, class node_t>
class basic_unsafe_reverse : private basic_unsafe_forward<value_t, node_t>
{
public:
    typedef basic_unsafe_forward<value_t, node_t> base_iterator;
    typedef value_t value_type;
    typedef value_t* pointer;
    typedef value_t& reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef node_t node_type;
    basic_unsafe_reverse() = default;
    inline basic_unsafe_reverse(node_type* pointer)
	:
	    base_iterator(pointer)
    { }
    inline basic_unsafe_reverse(const basic_unsafe_reverse& other)
	:
	    base_iterator(static_cast<const base_iterator&>(other))
    { }
    inline basic_unsafe_reverse(basic_unsafe_reverse&& other)
	:
	    base_iterator(std::forward<base_iterator>(other))
    { }
    template <class other_value_t>
    inline basic_unsafe_reverse(const basic_unsafe_reverse<other_value_t, node_t>& other)
	:
	    base_iterator(static_cast<const typename basic_unsafe_reverse<other_value_t, node_t>::base_iterator&>(other))
    { }
    basic_unsafe_reverse& operator=(const basic_unsafe_reverse& other)
    {
	return static_cast<const base_iterator&>(*this) = static_cast<const base_iterator&>(other);
    }
    basic_unsafe_reverse& operator=(basic_unsafe_reverse&& other)
    {
	base_iterator::operator=(std::forward<base_iterator>(other));
	return *this;
    }
    template <class other_value_t>
    basic_unsafe_reverse& operator=(const basic_unsafe_reverse<other_value_t, node_t>& other)
    {
	return static_cast<const base_iterator&>(*this) = static_cast<const typename basic_unsafe_reverse<other_value_t, node_t>::base_iterator&>(other);
    }
    basic_unsafe_reverse& operator=(node_type* other)
    {
	return static_cast<const base_iterator&>(*this) = other;
    }
    ~basic_unsafe_reverse() = default;
    inline bool operator==(const basic_unsafe_reverse& other) const
    {
	return static_cast<const base_iterator&>(*this) == static_cast<const base_iterator&>(other);
    }
    inline bool operator!=(const basic_unsafe_reverse& other) const
    {
	return !(*this == other);
    }
    using base_iterator::operator*;
    using base_iterator::operator->;
    inline basic_unsafe_reverse& operator++()
    {
	base_iterator::operator--();
	return *this;
    }
    inline basic_unsafe_reverse operator++(int)
    {
	base_iterator::operator--(0);
	return *this;
    }
    inline basic_unsafe_reverse& operator--()
    {
	base_iterator::operator++();
	return *this;
    }
    inline basic_unsafe_reverse operator--(int)
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

class invalid_dereference : public std::out_of_range
{
public:
    explicit inline invalid_dereference(const std::string& what) : out_of_range(what) { }
    explicit inline invalid_dereference(const char* what) : out_of_range(what) { }
};

template <class value_t, class typed_allocator_t = turbo::memory::typed_allocator>
class emplacing_list
{
private:
    struct safe_node;
    struct unsafe_node;
public:
    typedef value_t value_type;
    typedef typed_allocator_t typed_allocator_type;
    typedef emplacing_list_iterator::basic_safe_forward<const value_t, safe_node> const_iterator;
    typedef emplacing_list_iterator::basic_safe_forward<value_t, safe_node> iterator;
    typedef emplacing_list_iterator::basic_safe_reverse<const value_t, safe_node> const_reverse_iterator;
    typedef emplacing_list_iterator::basic_safe_reverse<value_t, safe_node> reverse_iterator;
    typedef emplacing_list_iterator::basic_unsafe_forward<const value_t, unsafe_node> const_unsafe_iterator;
    typedef emplacing_list_iterator::basic_unsafe_forward<value_t, unsafe_node> unsafe_iterator;
    typedef emplacing_list_iterator::basic_unsafe_reverse<const value_t, unsafe_node> const_reverse_unsafe_iterator;
    typedef emplacing_list_iterator::basic_unsafe_reverse<value_t, unsafe_node> reverse_unsafe_iterator;
    static constexpr std::size_t allocation_size()
    {
	return sizeof(safe_node);
    }
    static constexpr std::size_t allocation_alignment()
    {
	return alignof(safe_node);
    }
    static constexpr std::size_t unsafe_allocation_size()
    {
	return sizeof(unsafe_node);
    }
    static constexpr std::size_t unsafe_allocation_alignment()
    {
	return alignof(unsafe_node);
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
	return back_.expired() ? reverse_iterator() : reverse_iterator(back_.lock());
    }
    inline reverse_iterator rend() noexcept
    {
	return reverse_iterator();
    }
    inline const_reverse_iterator crbegin() noexcept
    {
	return back_.expired() ? const_reverse_iterator() : const_reverse_iterator(back_.lock());
    }
    inline const_reverse_iterator crend() noexcept
    {
	return const_reverse_iterator();
    }
    inline unsafe_iterator unsafe_begin() noexcept
    {
	return unsafe_iterator(unsafe_front_);
    }
    inline unsafe_iterator unsafe_end() noexcept
    {
	return unsafe_iterator();
    }
    inline const_unsafe_iterator unsafe_cbegin() noexcept
    {
	return const_unsafe_iterator(unsafe_front_);
    }
    inline const_unsafe_iterator unsafe_cend() noexcept
    {
	return const_unsafe_iterator();
    }
    inline reverse_unsafe_iterator unsafe_rbegin() noexcept
    {
	return reverse_unsafe_iterator(unsafe_back_);
    }
    inline reverse_unsafe_iterator unsafe_rend() noexcept
    {
	return reverse_unsafe_iterator();
    }
    inline const_reverse_unsafe_iterator unsafe_crbegin() noexcept
    {
	return const_reverse_unsafe_iterator(unsafe_back_);
    }
    inline const_reverse_unsafe_iterator unsafe_crend() noexcept
    {
	return const_reverse_unsafe_iterator();
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
    template <class... args_t>
    unsafe_iterator unsafe_emplace_front(args_t&&... args);
    template <class... args_t>
    unsafe_iterator unsafe_emplace_back(args_t&&... args);
    template <class... args_t>
    unsafe_iterator unsafe_emplace(const_unsafe_iterator new_next, args_t&&... args);
    void unsafe_pop_front();
    void unsafe_pop_back();
    unsafe_iterator unsafe_erase(const_unsafe_iterator position);
private:
    struct safe_node : public std::enable_shared_from_this<safe_node>
    {
	template <class... args_t>
	safe_node(args_t&&... args);
	inline bool is_first() const { return previous.expired(); }
	inline bool is_last() const { return next.use_count() == 0; }
	value_t value;
	bool alive;
	std::shared_ptr<safe_node> next;
	std::weak_ptr<safe_node> previous;
    };
    struct unsafe_node
    {
	template <class... args_t>
	unsafe_node(args_t&&... args);
	inline bool is_first() const { return previous == nullptr; }
	inline bool is_last() const { return next == nullptr; }
	value_t value;
	bool alive;
	unsafe_node* next;
	unsafe_node* previous;
    };
    template <class... args_t>
    std::shared_ptr<safe_node> create_safe_node(args_t&&... args);
    void destroy_safe_node(safe_node* pointer);
    template <class... args_t>
    unsafe_node* create_unsafe_node(args_t&&... args);
    void destroy_unsafe_node(unsafe_node* pointer);
    typed_allocator_type& allocator_;
    std::size_t size_;
    std::shared_ptr<safe_node> front_;
    std::weak_ptr<safe_node> back_;
    unsafe_node* unsafe_front_;
    unsafe_node* unsafe_back_;
};

} // namespace container
} // namespace turbo

#endif
