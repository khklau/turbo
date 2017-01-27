#ifndef TURBO_CONTAINER_BITWISE_TRIE_HPP
#define TURBO_CONTAINER_BITWISE_TRIE_HPP

#include <cstdint>
#include <array>
#include <turbo/memory/tagged_ptr.hpp>
#include <turbo/memory/typed_allocator.hpp>

namespace turbo {
namespace container {

template <class key_t, class value_t, class allocator_t = turbo::memory::typed_allocator>
class bitwise_trie final
{
public:
    struct leaf;
private:
    struct branch;
public:
    typedef key_t key_type;
    typedef value_t value_type;
    typedef allocator_t allocator_type;
    struct leaf
    {
	template <class key_arg_t, class... value_args_t>
	leaf(const key_arg_t& key_arg, value_args_t&&... value_args);
	key_t key;
	value_t value;
    };
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
    static const std::size_t radix = 2U;
    bitwise_trie(allocator_type& allocator);
    inline std::size_t size() const
    {
	return size_;
    }
private:
    enum class child_type
    {
	branch,
	leaf
    };
    struct branch
    {
	branch();
	std::array<turbo::memory::tagged_ptr<branch, child_type>, radix> child;
    };
    template <class key_arg_t, class... value_args_t>
    leaf* create_leaf(const key_arg_t& key_arg, value_args_t&&... value_args);
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
