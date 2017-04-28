#include "untyped_allocator.hpp"
#include <utility>
#include <turbo/container/bitwise_trie.hxx>
#include <turbo/container/trie_key.hpp>
#include <turbo/memory/pool.hpp>

namespace tco = turbo::container;
namespace tme = turbo::memory;

namespace turbo {
namespace cinterop {

untyped_allocator::untyped_allocator(
	std::uint32_t default_capacity,
	const std::vector<tme::block_config>& config)
    :
	allocation_pool_(default_capacity, config),
	trie_pool_(default_capacity, tme::calibrate(config)),
	address_map_(trie_pool_)
{ }

///
/// For some reason using the default destructor requires users of this library
/// to include bitwise_trie.hxx (a GCC bug?), so defining a custom destructor
/// to avoid this
///
untyped_allocator::~untyped_allocator()
{ }

std::vector<tme::block_config> untyped_allocator::derive_trie_config(const std::vector<tme::block_config>& alloc_config)
{
    typedef tco::uint_trie_key<std::uintptr_t, trie_type::radix> key_type;
    std::vector<tme::block_config> result;
    result.push_back(tme::block_config(trie_type::node_sizes[0], alloc_config.size() * growth_contingency));
    result.push_back(tme::block_config(trie_type::node_sizes[1], alloc_config.size() * growth_contingency * key_type::max_prefix_capacity()));
    return std::move(result);
}

} // namespace cinterop
} // namespace turbo
