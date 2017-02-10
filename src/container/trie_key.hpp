#ifndef TURBO_CONTAINER_TRIE_KEY_HPP
#define TURBO_CONTAINER_TRIE_KEY_HPP

#include <cstdint>
#include <cmath>
#include <limits>
#include <tuple>
#include <turbo/math/const_expr.hpp>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace container {


template <class uint_t, std::size_t radix>
class uint_trie_key
{
public:
    typedef uint_t uint_type;
    static_assert(!std::numeric_limits<uint_type>::is_signed, "uint_t template parameter must be an unsigned integral");
    enum class write_result
    {
	success = 0U,
	key_full
    };
    enum class read_result
    {
	success = 0U,
	prefix_unavailable
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
    inline uint_trie_key(uint_type key)
	:
	    key_(key),
	    write_cursor_(key_bit_size()),
	    read_cursor_(0U)
    { }
    inline uint_trie_key()
	:
	    key_(0U),
	    write_cursor_(0U),
	    read_cursor_(0U)
    { }
    inline uint_trie_key(const uint_trie_key& other) = default;
    ~uint_trie_key() = default;
    uint_trie_key& operator=(const uint_trie_key& other) = default;
    inline bool is_unavailable() const
    {
	return write_cursor_ == read_cursor_;
    }
    inline bool is_full() const
    {
	return write_cursor_ == key_bit_size();
    }
    inline std::tuple<get_result, uint_type> get_written_prefixes() const
    {
	if (TURBO_UNLIKELY(write_cursor_ == 0U))
	{
	    return std::make_tuple(get_result::unavailable, 0U);
	}
	return std::make_tuple(get_result::success, key_ & predecessor_mask(write_cursor_));
    }
    inline std::tuple<get_result, uint_type> get_read_prefixes() const
    {
	if (TURBO_UNLIKELY(read_cursor_ == 0U))
	{
	    return std::make_tuple(get_result::unavailable, 0U);
	}
	return std::make_tuple(get_result::success, key_ & predecessor_mask(read_cursor_));
    }
    inline write_result write(uint_type prefix)
    {
	if (is_full())
	{
	    return write_result::key_full;
	}
	key_ = clear_prefix(key_, write_cursor_) | align(prefix, write_cursor_);
	write_cursor_ += radix_bit_size();
	return write_result::success;
    }
    inline std::tuple<read_result, uint_type> read()
    {
	if (is_unavailable())
	{
	    return std::make_tuple(read_result::prefix_unavailable, 0U);
	}
	uint_type prefix = (key_ & radix_mask(read_cursor_)) >> (key_bit_size() - radix_bit_size() - read_cursor_);
	read_cursor_ += radix_bit_size();
	return std::make_tuple(read_result::success, prefix);
    }
private:
    inline uint_type predecessor_mask(std::uint8_t cursor) const
    {
	return std::numeric_limits<uint_type>::max() << (key_bit_size() - cursor);
    }
    inline uint_type successor_mask(std::uint8_t cursor) const
    {
	return std::numeric_limits<uint_type>::max() >> (radix_bit_size() + cursor);
    }
    inline uint_type radix_mask(std::uint8_t cursor) const
    {
	return ((1U << radix_bit_size()) - 1U) << (key_bit_size() - radix_bit_size() - cursor);
    }
    inline uint_type align(uint_type prefix, std::uint8_t position) const
    {
	return prefix << (key_bit_size() - radix_bit_size() - position);
    }
    inline uint_type clear_prefix(uint_type key, std::uint8_t cursor)
    {
	return key & ~(radix_mask(cursor));
    }
    uint_type key_;
    std::uint8_t write_cursor_;
    std::uint8_t read_cursor_;
};

} // namespace container
} // namespace turbo

#endif
