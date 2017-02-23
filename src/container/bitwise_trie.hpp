#ifndef TURBO_CONTAINER_BITWISE_TRIE_HPP
#define TURBO_CONTAINER_BITWISE_TRIE_HPP

#include <cstdint>
#include <array>
#include <iterator>
#include <tuple>
#include <turbo/container/trie_key.hpp>
#include <turbo/memory/tagged_ptr.hpp>
#include <turbo/memory/typed_allocator.hpp>

namespace turbo {
namespace container {

namespace bitwise_trie_iterator {

template <class key_t, class value_t, class node_t>
class basic_forward : public std::bidirectional_iterator_tag
{
public:
    typedef key_t key_type;
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
    basic_forward(const basic_forward<key_type, other_value_t, node_type>& other);
    basic_forward& operator=(const basic_forward& other);
    basic_forward& operator=(basic_forward&& other);
    template <class other_value_t>
    basic_forward& operator=(const basic_forward<key_type, other_value_t, node_type>& other);
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
    key_type get_key() const;
private:
    node_type* pointer_;
};

template <class key_t, class value_t, class node_t>
class basic_reverse : private basic_forward<key_t, value_t, node_t>
{
public:
    typedef basic_forward<key_t, value_t, node_t> base_iterator;
    typedef key_t key_type;
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
    inline basic_reverse(const basic_reverse<key_type, other_value_t, node_type>& other)
	:
	    base_iterator(static_cast<const typename basic_reverse<key_type, other_value_t, node_type>::base_iterator&>(other))
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
    basic_reverse& operator=(const basic_reverse<key_type, other_value_t, node_type>& other)
    {
	return static_cast<const base_iterator&>(*this) =
		static_cast<const typename basic_reverse<key_type, other_value_t, node_type>::base_iterator&>(other);
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
    using base_iterator::ptr;
    using base_iterator::is_valid;
    using base_iterator::get_key;
};

} // namespace bitwise_trie_iterator

class invalid_bitwise_trie_error : public std::logic_error
{
public:
    explicit inline invalid_bitwise_trie_error(const std::string& what) : logic_error(what) { }
    explicit inline invalid_bitwise_trie_error(const char* what) : logic_error(what) { }
};

template <class key_t, class value_t, class allocator_t = turbo::memory::typed_allocator>
class bitwise_trie_tester;

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
    typedef bitwise_trie_iterator::basic_forward<key_type, const value_type, leaf> const_iterator;
    typedef bitwise_trie_iterator::basic_forward<key_type, value_type, leaf> iterator;
    typedef bitwise_trie_iterator::basic_reverse<key_type, const value_type, leaf> const_reverse_iterator;
    typedef bitwise_trie_iterator::basic_reverse<key_type, value_type, leaf> reverse_iterator;
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
    template <class... value_args_t>
    std::tuple<iterator, bool> emplace(key_type key, value_args_t&&... value_args);
    friend class bitwise_trie_tester<key_type, value_type, allocator_type>;
private:
    enum class child_type
    {
	branch = 0U,
	leaf
    };
    typedef turbo::memory::tagged_ptr<branch, child_type> branch_ptr;
    typedef uint_trie_key<key_type, radix> trie_key;
    struct leaf
    {
	template <class... value_args_t>
	leaf(key_type key, value_args_t&&... value_args);
	const key_type key;
	value_type value;
    };
    struct branch
    {
	branch();
	std::array<branch_ptr, radix> children;
    };
    class leading_zero_index
    {
    public:
	leading_zero_index(branch_ptr& root);
	leading_zero_index(const leading_zero_index&) = delete;
	~leading_zero_index() = default;
	leading_zero_index& operator=(const leading_zero_index&) = delete;
	inline std::tuple<branch_ptr*, typename trie_key::iterator> search(const trie_key& key);
	inline std::tuple<const branch_ptr*, typename trie_key::iterator> const_search(const trie_key& key) const;
	void insert(branch* branch, const typename trie_key::iterator& iter);
    private:
	branch_ptr& root_;
	std::array<branch_ptr, trie_key::max_prefix_capacity()> index_;
    };
    template <typename compare_t>
    leaf* least_search(key_type key, compare_t compare_func) const;
    template <typename compare_t>
    leaf* most_search(key_type key, compare_t compare_func) const;
    inline leaf* min() const;
    inline leaf* max() const;
    template <class... value_args_t>
    leaf* create_leaf(key_type key_arg, value_args_t&&... value_args);
    void destroy_leaf(leaf* pointer);
    branch* create_branch();
    void destroy_branch(branch* pointer);
    allocator_type& allocator_;
    std::size_t size_;
    branch_ptr root_;
    leading_zero_index index_;
};

} // namespace container
} // namespace turbo

#endif
