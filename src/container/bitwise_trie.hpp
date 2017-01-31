#ifndef TURBO_CONTAINER_BITWISE_TRIE_HPP
#define TURBO_CONTAINER_BITWISE_TRIE_HPP

#include <cmath>
#include <cstdint>
#include <array>
#include <iterator>
#include <turbo/memory/tagged_ptr.hpp>
#include <turbo/memory/typed_allocator.hpp>

namespace turbo {
namespace container {

namespace bitwise_trie_iterator {

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
    basic_forward() noexcept;
    basic_forward(node_type* pointer) noexcept;
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
    inline basic_reverse(node_type* pointer) noexcept
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
};

} // namespace bitwise_trie_iterator

class invalid_bitwise_trie_error : public std::logic_error
{
public:
    explicit inline invalid_bitwise_trie_error(const std::string& what) : logic_error(what) { }
    explicit inline invalid_bitwise_trie_error(const char* what) : logic_error(what) { }
};

template <class key_t, class value_t, class allocator_t = turbo::memory::typed_allocator>
class bitwise_trie final
{
private:
    struct leaf;
    struct branch;
public:
    typedef key_t key_type;
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    struct record
    {
	template <class key_arg_t, class... value_args_t>
	record(const key_arg_t& key_arg, value_args_t&&... value_args);
	key_t key;
	value_t value;
    };
    typedef bitwise_trie_iterator::basic_forward<const record, leaf> const_iterator;
    typedef bitwise_trie_iterator::basic_forward<record, leaf> iterator;
    typedef bitwise_trie_iterator::basic_reverse<const record, leaf> const_reverse_iterator;
    typedef bitwise_trie_iterator::basic_reverse<record, leaf> reverse_iterator;
    static const std::size_t radix = 2U;
    static constexpr std::array<std::size_t, 2U> node_sizes
    {
	sizeof(leaf),
	sizeof(branch)
    };
    static constexpr std::array<std::size_t, 2U> node_alignments
    {
	alignof(leaf),
	alignof(branch)
    };
    static constexpr std::size_t key_bit_size()
    {
	return sizeof(key_type) * 8U;
    }
    static constexpr std::size_t depth()
    {
	return static_cast<std::size_t>(std::ceil(
		std::log(static_cast<double>(key_bit_size()) /
		std::log(static_cast<double>(radix)))));
    }
    bitwise_trie(allocator_type& allocator);
    inline std::size_t size() const noexcept
    {
	return size_;
    }
    inline iterator begin()
    {
	return iterator(min());
    }
    inline iterator end() noexcept
    {
	return iterator();
    }
    inline const_iterator cbegin()
    {
	return const_iterator(min());
    }
    inline const_iterator cend() noexcept
    {
	return const_iterator();
    }
    inline reverse_iterator rbegin()
    {
	return reverse_iterator(max());
    }
    inline reverse_iterator rend() noexcept
    {
	return reverse_iterator();
    }
    inline const_reverse_iterator crbegin()
    {
	return const_reverse_iterator(max());
    }
    inline const_reverse_iterator crend() noexcept
    {
	return const_reverse_iterator();
    }
private:
    struct leaf
    {
	template <class key_arg_t, class... value_args_t>
	leaf(branch* a_branch, const key_arg_t& key_arg, value_args_t&&... value_args);
	const branch* parent;
	record value;
    };
    enum class child_type
    {
	branch = 0U,
	leaf
    };
    typedef turbo::memory::tagged_ptr<branch, child_type> branch_ptr;
    struct branch
    {
	branch();
	std::array<branch_ptr, radix> children;
    };
    leaf* min();
    leaf* max();
    template <class key_arg_t, class... value_args_t>
    leaf* create_leaf(branch* a_branch, const key_arg_t& key_arg, value_args_t&&... value_args);
    void destroy_leaf(leaf* pointer);
    branch* create_branch();
    void destroy_branch(branch* pointer);
    allocator_type& allocator_;
    std::size_t size_;
    branch* root_;
    std::array<branch*, key_bit_size()> leading_zero_index_;
};

} // namespace container
} // namespace turbo

#endif
