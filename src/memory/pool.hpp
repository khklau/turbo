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

typedef std::uint32_t capacity_type;

enum class make_result
{
    success,
    pool_full
};

template <std::size_t block_size_c, template <class type_t> class allocator_t = std::allocator>
class block_pool
{
public:
    block_pool(capacity_type capacity);
    std::size_t get_byte_size() const;
    const void* get_base_address() const;
    std::pair<make_result, void*> allocate();
    void free(void* pointer);
    template <class value_t, class... args_t>
    std::pair<make_result, pool_unique_ptr<value_t>> make_unique(args_t&&... args);
    template <class value_t, class... args_t>
    std::pair<make_result, std::shared_ptr<value_t>> make_shared(args_t&&... args);
private:
    typedef typename std::aligned_storage<block_size_c>::type block_type;
    typedef turbo::container::mpmc_ring_queue<capacity_type, allocator_t<capacity_type>> free_list_type;
    free_list_type free_list_;
    typename std::vector<block_type, allocator_t<block_type>> block_list_;
};

} // namespace memory
} // namespace turbo

#endif
