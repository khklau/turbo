#ifndef TURBO_MEMORY_POOL_HPP
#define TURBO_MEMORY_POOL_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <turbo/container/mpmc_ring_queue.hpp>

namespace turbo {
namespace memory {

template <class value_t>
using pool_unique_ptr = std::unique_ptr<value_t, std::function<void (value_t*)>>;

enum class make_result
{
    success,
    pool_full
};

template <std::uint32_t block_size_c, template <class type_t> class allocator_t = std::allocator>
class block_pool
{
public:
    block_pool(std::uint32_t capacity, std::uint16_t user_limit);
    template <class value_t, class... args_t>
    std::pair<make_result, pool_unique_ptr<value_t>> make_unique(args_t&&... args);
private:
    typedef typename std::aligned_storage<block_size_c>::type block_type;
    typedef turbo::container::mpmc_ring_queue<std::uint32_t, allocator_t<std::uint32_t>> free_list_type;
    free_list_type free_list_;
    typename std::vector<block_type, allocator_t<block_type>> block_list_;
};

} // namespace memory
} // namespace turbo

#endif
