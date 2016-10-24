#ifndef TURBO_MEMORY_BLOCK_HPP
#define TURBO_MEMORY_BLOCK_HPP

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <turbo/container/mpmc_ring_queue.hpp>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace memory {

class out_of_memory_error : public std::runtime_error
{
public:
    explicit out_of_memory_error(const std::string& what);
    explicit out_of_memory_error(const char* what);
};

class invalid_size_error : public std::invalid_argument
{
public:
    explicit invalid_size_error(const std::string& what);
    explicit invalid_size_error(const char* what);
};

class invalid_alignment_error : public std::invalid_argument
{
public:
    explicit invalid_alignment_error(const std::string& what);
    explicit invalid_alignment_error(const char* what);
};

class invalid_pointer_error : public std::invalid_argument
{
public:
    explicit invalid_pointer_error(const std::string& what);
    explicit invalid_pointer_error(const char* what);
};

class TURBO_SYMBOL_DECL block
{
public:
    typedef std::uint32_t capacity_type;
    block(std::size_t value_size, capacity_type capacity);
    block(std::size_t value_size, capacity_type capacity, std::size_t alignment);
    inline std::size_t get_capacity() const { return capacity_; }
    inline std::size_t get_usable_size() const { return usable_size_; }
    inline const void* get_base_address() const { return base_; }
    inline bool in_range(const void* pointer) const { return base_ <= pointer && pointer < (static_cast<std::uint8_t*>(base_) + usable_size_); }
    void* allocate();
    void free(void* pointer);
private:
    typedef turbo::container::mpmc_ring_queue<capacity_type> free_list_type;
    block() = delete;
    block(const block&) = delete;
    block(block&&) = delete;
    block& operator=(const block&) = delete;
    block& operator=(block&&) = delete;
    std::size_t value_size_;
    std::size_t capacity_;
    std::size_t usable_size_;
    std::unique_ptr<std::uint8_t[]> storage_;
    std::uint8_t* base_;
    free_list_type free_list_;
};

} // namespace memory
} // namespace turbo

#endif
