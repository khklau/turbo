#ifndef TURBO_CONTAINER_CONCURRENT_VECTOR_HPP
#define TURBO_CONTAINER_CONCURRENT_VECTOR_HPP

#include <cstddef>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <turbo/container/mpmc_ring_queue.hpp>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace container {

class TURBO_SYMBOL_DECL invalid_capacity_argument : public std::invalid_argument
{
public:
    explicit invalid_capacity_argument(const char* what) : invalid_argument(what) { };
    explicit invalid_capacity_argument(const std::string& what) : invalid_argument(what) { };
};

class TURBO_SYMBOL_DECL exceeded_capacity_error : public std::length_error
{
public:
    explicit exceeded_capacity_error(const char* what) : length_error(what) { };
    explicit exceeded_capacity_error(const std::string& what) : length_error(what) { };
};

class TURBO_SYMBOL_DECL corrupt_vector_error : public std::runtime_error
{
public:
    explicit corrupt_vector_error(const char* what) : std::runtime_error(what) { };
    explicit corrupt_vector_error(const std::string& what) : std::runtime_error(what) { };
};

///
/// Design taken from Dechev, Pirkelbauer & Stroustrup's Lock-free dynamically resizeable arrays paper
///
template <class value_t, template <class type_t> class allocator_t = std::allocator>
class TURBO_SYMBOL_DECL concurrent_vector
{
public:
    typedef std::uint16_t throughput_type;
    typedef std::uint32_t capacity_type;
    enum class change_result
    {
	success,
	beaten,
	busy,
	max_write_reached
    };
    concurrent_vector(std::uint8_t initial_capacity_exponent, std::uint8_t max_capacity_exponent);
    concurrent_vector(std::uint8_t initial_capacity_exponent, std::uint8_t max_capacity_exponent, throughput_type max_concurrent_writes);
    ~concurrent_vector();
    value_t& operator[](capacity_type index);
    const value_t& operator[](capacity_type index) const;
    value_t& at(capacity_type index);
    const value_t& at(capacity_type index) const;
    change_result try_pushback(value_t&& value);
private:
    template <class field_t>
    struct versioned_value
    {
	typedef std::uint32_t type;
	inline static type create(std::uint16_t version, field_t field)
	{
	    type result = version;
	    result = result << 16U;
	    return result + static_cast<std::uint16_t>(field);
	}
	inline static std::uint16_t version(const type& value)
	{
	    return static_cast<std::uint16_t>((value & 0xFF00) >> 16U);
	}
	inline static field_t value(const type& value)
	{
	    return static_cast<field_t>(value & 0x00FF);
	}
    };
    struct alignas(alignof(value_t)) node
    {
	node();
	enum class status : std::uint16_t
	{
	    ready,
	    updating
	};
	typedef versioned_value<status> versioned_guard;
	value_t value;
	std::atomic<typename versioned_guard::type> guard;
    };
    struct descriptor
    {
	descriptor();
	descriptor(capacity_type size_, capacity_type capacity_, bool pending_, std::uint16_t version_, value_t&& value_, capacity_type location_);
	capacity_type size;
	capacity_type capacity;
	std::atomic<bool> has_pending_write;
	std::uint16_t expected_version;
	value_t new_value;
	capacity_type location;
    };
    typedef versioned_value<throughput_type> descriptor_reference;
    typedef std::pair<capacity_type, capacity_type> subscript_type;
    static const std::uint8_t capacity_base_ = 2U;
    change_result complete_write(descriptor& operation);
    capacity_type allocate_bucket(capacity_type bucket_index);
    node& get_node(capacity_type index);
    const node& get_node(capacity_type index) const;
    subscript_type find_subscript(capacity_type index) const;
    void check_range(capacity_type index) const;
    const std::uint8_t initial_exponent_;
    const std::uint8_t max_exponent_;
    std::unique_ptr<std::atomic<node*>[]> buckets_;
    std::unique_ptr<descriptor[]> descriptors_;
    std::atomic<typename descriptor_reference::type> current_descriptor_;
    turbo::container::mpmc_ring_queue<throughput_type, allocator_t> free_descriptors_;
};

} // namespace container
} // namespace turbo

#endif
