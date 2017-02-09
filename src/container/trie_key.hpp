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
    enum class push_result
    {
	success = 0U,
	key_full
    };
    enum class pop_result
    {
	success = 0U,
	key_empty
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
	    push_cursor_(0U),
	    pop_cursor_(0U)
    { }
    inline uint_trie_key()
	:
	    uint_trie_key(0U)
    { }
    inline uint_trie_key(const uint_trie_key& other) = default;
    ~uint_trie_key() = default;
    uint_trie_key& operator=(const uint_trie_key& other) = default;
    inline bool is_empty() const
    {
	return push_cursor_ == pop_cursor_;
    }
    inline bool is_full() const
    {
	return push_cursor_ == key_bit_size();
    }
    inline std::tuple<get_result, uint_type> get_pushed_prefixes() const
    {
	if (TURBO_UNLIKELY(push_cursor_ == 0U))
	{
	    return std::make_tuple(get_result::unavailable, 0U);
	}
	return std::make_tuple(get_result::success, key_ & predecessor_mask(push_cursor_));
    }
    inline std::tuple<get_result, uint_type> get_popped_prefixes() const
    {
	if (TURBO_UNLIKELY(pop_cursor_ == 0U))
	{
	    return std::make_tuple(get_result::unavailable, 0U);
	}
	return std::make_tuple(get_result::success, key_ & predecessor_mask(pop_cursor_));
    }
    inline push_result push(uint_type prefix)
    {
	if (is_full())
	{
	    return push_result::key_full;
	}
	key_ = clear_prefix(key_, push_cursor_) | align(prefix, push_cursor_);
	push_cursor_ += radix_bit_size();
	return push_result::success;
    }
    inline std::tuple<pop_result, uint_type> pop()
    {
	return std::make_tuple(pop_result::success, 0U);
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
    std::uint8_t push_cursor_;
    std::uint8_t pop_cursor_;
};

} // namespace container
} // namespace turbo

#endif
