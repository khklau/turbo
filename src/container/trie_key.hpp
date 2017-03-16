#ifndef TURBO_CONTAINER_TRIE_KEY_HPP
#define TURBO_CONTAINER_TRIE_KEY_HPP

#include <cstdint>
#include <cmath>
#include <iterator>
#include <limits>
#include <tuple>
#include <turbo/math/const_expr.hpp>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {

template <class uint_t, std::size_t radix>
class uint_trie_key_tester;

template <class uint_t, std::size_t radix>
class uint_trie_key
{
public:
    typedef uint_t uint_type;
    static_assert(!std::numeric_limits<uint_type>::is_signed, "uint_t template parameter must be an unsigned integral");
    enum class write_result
    {
	success = 0U,
	out_of_bounds
    };
    enum class read_result
    {
	success = 0U,
	out_of_bounds
    };
    enum class get_result
    {
	success = 0U,
	unavailable
    };
    static constexpr std::size_t key_bit_size()
    {
	return sizeof(uint_type) * 8U;
    }
    static constexpr std::size_t radix_bit_size()
    {
	return std::llround(std::log2(radix) / std::log2(2U));
    }
    static constexpr std::size_t max_prefix_capacity()
    {
	return key_bit_size() / radix_bit_size();
    }
    class iterator : public std::forward_iterator_tag
    {
    public:
	typedef uint_type value_type;
	typedef uint_type* pointer;
	typedef uint_type& reference;
	typedef std::ptrdiff_t difference_type;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef std::uint8_t index_type;
	inline iterator()
	    :
		index_(0U)
	{ }
	inline explicit iterator(index_type index)
	    :
		index_(index)
	{ }
	iterator(const iterator& other)
	    :
		index_(other.index_)
	{ }
	~iterator() = default;
	iterator& operator=(const iterator& other)
	{
	    if (this != &other)
	    {
		index_ = other.index_;
	    }
	    return *this;
	}
	inline bool operator==(const iterator& other) const
	{
	    return (!is_valid() && !other.is_valid()) || index_ == other.index_;
	}
	inline bool operator!=(const iterator& other) const
	{
	    return !(*this == other);
	}
	inline iterator operator+(index_type quantity) const
	{
	    return iterator(index_ + quantity);
	}
	value_type& operator*() = delete;
	value_type* operator->() = delete;
	inline iterator& operator+=(index_type quantity)
	{
	    index_ += quantity;
	    return *this;
	}
	inline iterator& operator++()
	{
	    return (*this) += 1U;
	}
	inline iterator operator++(int)
	{
	    iterator tmp(*this);
	    ++(*this);
	    return tmp;
	}
	inline bool is_valid() const
	{
	    return index_ < max_prefix_capacity();
	}
	inline bool is_min_index() const
	{
	    return index_ == 0U;
	}
	inline bool is_max_index() const
	{
	    return index_ == (max_prefix_capacity() - 1U);
	}
	inline index_type get_index() const
	{
	    return index_;
	}
    private:
	index_type index_;
    };
    inline explicit uint_trie_key(uint_type key)
	:
	    key_(key)
    { }
    inline uint_trie_key()
	:
	    key_(0U)
    { }
    inline uint_trie_key(const uint_trie_key& other) = default;
    ~uint_trie_key() = default;
    uint_trie_key& operator=(const uint_trie_key& other) = default;
    inline iterator begin() const
    {
	return iterator(0U);
    }
    static constexpr iterator end()
    {
	return iterator(max_prefix_capacity());
    }
    inline uint_type get_key() const
    {
	return key_;
    }
    inline std::tuple<get_result, uint_type> get_preceding_prefixes(iterator iter) const
    {
	if (iter.get_index() == 0U)
	{
	    return std::make_tuple(get_result::unavailable, 0U);
	}
	return std::make_tuple(get_result::success, key_ & predecessor_mask(iter));
    }
    inline write_result write(iterator iter, uint_type prefix)
    {
	if (!iter.is_valid())
	{
	    return write_result::out_of_bounds;
	}
	key_ = clear_prefix(key_, iter) | align(prefix, iter);
	return write_result::success;
    }
    inline std::tuple<read_result, uint_type> read(iterator iter) const
    {
	if (!iter.is_valid())
	{
	    return std::make_tuple(read_result::out_of_bounds, 0U);
	}
	uint_type prefix = (key_ & radix_mask(iter)) >> (key_bit_size() - radix_bit_size() - bit_position(iter));
	return std::make_tuple(read_result::success, prefix);
    }
    friend uint_trie_key_tester<uint_type, radix>;
private:
    inline static std::size_t bit_position(iterator iter)
    {
	return iter.is_valid()
		? iter.get_index() * radix_bit_size()
		: key_bit_size();
    }
    inline static uint_type predecessor_mask(iterator iter)
    {
	return std::numeric_limits<uint_type>::max() << (key_bit_size() - bit_position(iter));
    }
    inline static uint_type successor_mask(iterator iter)
    {
	return std::numeric_limits<uint_type>::max() >> (radix_bit_size() + bit_position(iter));
    }
    inline static uint_type radix_mask(iterator iter)
    {
	return ((1U << radix_bit_size()) - 1U) << (key_bit_size() - radix_bit_size() - bit_position(iter));
    }
    inline static uint_type align(uint_type prefix, iterator iter)
    {
	return prefix << (key_bit_size() - radix_bit_size() - bit_position(iter));
    }
    inline static uint_type clear_prefix(uint_type key, iterator iter)
    {
	return key & ~(radix_mask(iter));
    }
    uint_type key_;
};

} // namespace container
} // namespace turbo

#endif
