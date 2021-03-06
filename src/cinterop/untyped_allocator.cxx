#include "untyped_allocator.hpp"
#include <utility>
#include <turbo/container/bitwise_trie.hh>
#include <turbo/container/trie_key.hpp>
#include <turbo/memory/slab_allocator.hh>

namespace tco = turbo::container;
namespace tme = turbo::memory;

namespace turbo {
namespace cinterop {

untyped_allocator::untyped_allocator(
	std::uint32_t contingency_capacity,
	const std::vector<tme::block_config>& config)
    :
	allocation_slab_(contingency_capacity, config),
	trie_slab_(contingency_capacity, derive_trie_config(tme::calibrate(contingency_capacity, config))),
	address_map_(trie_slab_)
{
    init_address_map();
}

untyped_allocator::untyped_allocator(const untyped_allocator& other)
    :
	allocation_slab_(other.allocation_slab_),
	trie_slab_(2U, other.trie_slab_.get_block_config()),
	address_map_(trie_slab_)
{
    init_address_map();
}

///
/// For some reason using the default destructor requires users of this library
/// to include bitwise_trie.hh (a GCC bug?), so defining a custom destructor
/// to avoid this
///
untyped_allocator::~untyped_allocator()
{ }

untyped_allocator& untyped_allocator::operator=(const untyped_allocator& other)
{
    if (this != &other
	    && this->trie_slab_.get_block_config() == other.trie_slab_.get_block_config()
	    && this->address_map_.size() >= other.address_map_.size())
    {
	this->allocation_slab_ = other.allocation_slab_;
    }
    return *this;
}

void* untyped_allocator::malloc(std::size_t size)
{
    if (!allocation_slab_.in_configured_range(size))
    {
	return nullptr;
    }
    turbo::memory::block_list& list = allocation_slab_.at(size);
    std::size_t old_size = list.get_list_size();
    void* result = allocation_slab_.malloc(size);
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
	allocation_slab_.free(ptr, *iter);
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

void untyped_allocator::init_address_map()
{
    for (tme::block_list& list: allocation_slab_)
    {
	for (tme::block& block: list)
	{
	    address_map_.emplace(reinterpret_cast<std::uintptr_t>(block.get_base_address()), block.get_value_size());
	}
    }
}

} // namespace cinterop
} // namespace turbo
