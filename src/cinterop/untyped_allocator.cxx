#include "untyped_allocator.hpp"
#include <utility>
#include <turbo/container/bitwise_trie.hxx>
#include <turbo/container/trie_key.hpp>
#include <turbo/memory/pool.hxx>

namespace tco = turbo::container;
namespace tme = turbo::memory;

namespace turbo {
namespace cinterop {

untyped_allocator::untyped_allocator(
	std::uint32_t default_capacity,
	const std::vector<tme::block_config>& config)
    :
	allocation_pool_(default_capacity, config),
	trie_pool_(default_capacity, derive_trie_config(tme::calibrate(default_capacity, config))),
	address_map_(trie_pool_)
{
    for (tme::block_list& list: allocation_pool_)
    {
	for (tme::block& block: list)
	{
	    address_map_.emplace(reinterpret_cast<std::uintptr_t>(block.get_base_address()), block.get_value_size());
	}
    }
}

///
/// For some reason using the default destructor requires users of this library
/// to include bitwise_trie.hxx (a GCC bug?), so defining a custom destructor
/// to avoid this
///
untyped_allocator::~untyped_allocator()
{ }

void* untyped_allocator::malloc(std::size_t size)
{
    if (!allocation_pool_.in_configured_range(size))
    {
	return nullptr;
    }
    turbo::memory::block_list& list = allocation_pool_.at(size);
    std::size_t old_size = list.get_list_size();
    void* result = allocation_pool_.malloc(size);
    std::size_t new_size = list.get_list_size();
    if (old_size < new_size)
    {
	for (auto iter = list.begin(); iter != list.end(); ++iter)
	{
	    if (iter.is_last())
	    {
		address_map_.emplace(reinterpret_cast<std::uintptr_t>(iter->get_base_address()), iter->get_value_size());
	    }
	}
    }
    return result;
}

void untyped_allocator::free(void* ptr)
{
    auto iter = address_map_.find_less_equal(reinterpret_cast<std::uintptr_t>(ptr));
    if (iter != address_map_.cend())
    {
	allocation_pool_.free(ptr, *iter);
    }
}

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
