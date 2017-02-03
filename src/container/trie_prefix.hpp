#ifndef TURBO_CONTAINER_TRIE_PREFIX
#define TURBO_CONTAiNER_TRIE_PREFIX

#include <cstddef>
#include <cmath>
#include <turbo/math/const_expr.hpp>

namespace turbo {
namespace container {

template <class key_t, std::size_t radix>
class uint_trie_prefix
{
public:
    typedef key_t key_type;
    static constexpr std::size_t key_bit_size()
    {
	return sizeof(key_type) * 8U;
    }
    static constexpr std::size_t radix_bit_size()
    {
	return static_cast<std::size_t>(turbo::math::const_expr::trunc(
		std::log(static_cast<double>(radix) /
		std::log(static_cast<double>(2U)))));
    }
    static constexpr key_type radix_mask()
    {
	return ((1U << radix_bit_size()) - 1U) << (key_bit_size() - radix_bit_size());
    }
    static constexpr std::size_t max_usage()
    {
	return key_bit_size() / radix_bit_size();
    }
    static_assert(radix_bit_size() < key_bit_size(), "radix must be smaller than key");
    uint_trie_prefix() = delete;
    inline uint_trie_prefix(key_type key)
	:
	    common_prefix_(0U),
	    unused_prefix_(key),
	    usage_count_(0U)
    { }
    inline uint_trie_prefix(const uint_trie_prefix& other) = default;
    ~uint_trie_prefix() = default;
    uint_trie_prefix& operator=(const uint_trie_prefix& other) = default;
    inline key_type get_common_prefix() const
    {
	return common_prefix_;
    }
    inline key_type get_unused_prefix() const
    {
	return unused_prefix_;
    }
    inline key_type get_next_prefix() const
    {
	return (unused_prefix_ & radix_mask()) >> (key_bit_size() - radix_bit_size());
    }
    inline std::size_t get_usage_count() const
    {
	return usage_count_;
    }
    uint_trie_prefix operator<<(const std::size_t& radix_count)
    {
	return (radix_count == 0U)
		? uint_trie_prefix(*this)
		: uint_trie_prefix(
			(common_prefix_ << (radix_count * radix_bit_size())) + get_next_prefix(),
			unused_prefix_ << (radix_count * radix_bit_size()),
			get_usage_count() + radix_count);
    }
private:
    inline uint_trie_prefix(key_type common_prefix, key_type unused_prefix, std::size_t usage_count)
	:
	    common_prefix_(common_prefix),
	    unused_prefix_(unused_prefix),
	    usage_count_(usage_count)
    { }
    key_type common_prefix_;
    key_type unused_prefix_;
    std::size_t usage_count_;
};

} // namespace container
} // namespace turbo

#endif
