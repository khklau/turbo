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

template <class trie_t, class key_t, class value_t, class node_t>
class basic_forward : public std::bidirectional_iterator_tag
{
public:
    typedef trie_t trie_type;
    typedef key_t key_type;
    typedef value_t value_type;
    typedef value_t* pointer;
    typedef value_t& reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef node_t node_type;
    inline explicit basic_forward(trie_type& trie) noexcept;
    inline basic_forward(trie_type& trie, node_type* pointer) noexcept;
    inline basic_forward(const basic_forward& other);
    inline basic_forward(basic_forward&& other);
    template <class other_value_t>
    inline basic_forward(const basic_forward<trie_type, key_type, other_value_t, node_type>& other);
    inline basic_forward& operator=(const basic_forward& other);
    inline basic_forward& operator=(basic_forward&& other);
    template <class other_value_t>
    inline basic_forward& operator=(const basic_forward<trie_type, key_type, other_value_t, node_type>& other);
    inline basic_forward& operator=(node_type* other);
    ~basic_forward() = default;
    inline bool operator==(const basic_forward& other) const;
    inline bool operator!=(const basic_forward& other) const { return !(*this == other); }
    inline value_t& operator*();
    inline value_t* operator->();
    inline basic_forward& operator++();
    inline basic_forward operator++(int);
    inline basic_forward& operator--();
    inline basic_forward operator--(int);
    inline trie_type& get_trie() { return trie_; }
    inline node_type* get_ptr() const { return pointer_; }
    inline bool is_valid() const { return pointer_ != nullptr; }
    inline key_type get_key() const;
private:
    trie_type& trie_;
    node_type* pointer_;
};

template <class trie_t, class key_t, class value_t, class node_t>
class basic_reverse : private basic_forward<trie_t, key_t, value_t, node_t>
{
public:
    typedef basic_forward<trie_t, key_t, value_t, node_t> base_iterator;
    typedef trie_t trie_type;
    typedef key_t key_type;
    typedef value_t value_type;
    typedef value_t* pointer;
    typedef value_t& reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef node_t node_type;
    inline explicit basic_reverse(trie_type& trie)
	:
	    base_iterator(trie)
    { }
    inline basic_reverse(trie_type& trie, node_type* pointer) noexcept
	:
	    base_iterator(trie, pointer)
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
    inline basic_reverse(const basic_reverse<trie_type, key_type, other_value_t, node_type>& other)
	:
	    base_iterator(static_cast<const typename basic_reverse<trie_type, key_type, other_value_t, node_type>::base_iterator&>(other))
    { }
    inline basic_reverse& operator=(const basic_reverse& other)
    {
	return static_cast<const base_iterator&>(*this) = static_cast<const base_iterator&>(other);
    }
    inline basic_reverse& operator=(basic_reverse&& other)
    {
	base_iterator::operator=(std::forward<base_iterator>(other));
	return *this;
    }
    template <class other_value_t>
    inline basic_reverse& operator=(const basic_reverse<trie_type, key_type, other_value_t, node_type>& other)
    {
	return static_cast<const base_iterator&>(*this) =
		static_cast<const typename basic_reverse<trie_type, key_type, other_value_t, node_type>::base_iterator&>(other);
    }
    inline basic_reverse& operator=(node_type* other)
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
    using base_iterator::get_trie;
    using base_iterator::get_ptr;
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
    typedef bitwise_trie<key_type, value_type, allocator_type> self_type;
    typedef bitwise_trie_iterator::basic_forward<const self_type, key_type, const value_type, leaf> const_iterator;
    typedef bitwise_trie_iterator::basic_forward<self_type, key_type, value_type, leaf> iterator;
    typedef bitwise_trie_iterator::basic_reverse<const self_type, key_type, const value_type, leaf> const_reverse_iterator;
    typedef bitwise_trie_iterator::basic_reverse<self_type, key_type, value_type, leaf> reverse_iterator;
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
    ~bitwise_trie();
    inline std::size_t size() const noexcept
    {
	return size_;
    }
    inline iterator begin()
    {
	return iterator(*this, min());
    }
    inline iterator end() noexcept
    {
	return iterator(*this);
    }
    inline const_iterator cbegin() const
    {
	return const_iterator(*this, min());
    }
    inline const_iterator cend() const noexcept
    {
	return const_iterator(*this);
    }
    inline reverse_iterator rbegin()
    {
	return reverse_iterator(*this, max());
    }
    inline reverse_iterator rend() noexcept
    {
	return reverse_iterator(*this);
    }
    inline const_reverse_iterator crbegin() const
    {
	return const_reverse_iterator(*this, max());
    }
    inline const_reverse_iterator crend() const noexcept
    {
	return const_reverse_iterator(*this);
    }
    inline const_iterator find(key_type key) const;
    inline const_iterator find_successor(const_iterator iter) const;
    inline const_iterator find_predecessor(const_iterator iter) const;
    inline const_iterator find_less_equal(key_type key) const;
    template <class... value_args_t>
    std::tuple<iterator, bool> emplace(key_type key, value_args_t&&... value_args);
    std::size_t erase(key_type key);
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
	void remove(const typename trie_key::iterator& iter);
    private:
	branch_ptr& root_;
	std::array<branch_ptr, trie_key::key_bit_size()> index_;
    };
    inline leaf* min() const;
    inline leaf* max() const;
    template <typename compare_t>
    leaf* least_first_search(
	    const branch_ptr* branch,
	    trie_key key_wanted,
	    trie_key key_found,
	    typename trie_key::iterator iter,
	    compare_t compare_func) const;
    template <typename compare_t>
    leaf* most_first_search(
	    const branch_ptr* branch,
	    trie_key key_wanted,
	    trie_key key_found,
	    typename trie_key::iterator iter,
	    compare_t compare_func) const;
    template <typename compare_t>
    std::tuple<std::size_t, std::size_t> erase_recursive(
	    branch_ptr* branch,
	    const trie_key& key,
	    typename trie_key::iterator iter,
	    compare_t compare_func);
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
