#ifndef TURBO_CONTAINER_CONCURRENT_VECTOR_HPP
#define TURBO_CONTAINER_CONCURRENT_VECTOR_HPP

#include <cstddef>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
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

///
/// Design taken from Dechev, Pirkelbauer & Stroustrup's Lock-free dynamically resizeable arrays paper
///
template <class value_t, template <class type_t> class allocator_t = std::allocator>
class TURBO_SYMBOL_DECL concurrent_vector
{
public:
    typedef std::uint32_t capacity_type;
    typedef std::pair<capacity_type, capacity_type> subscript_type;
    concurrent_vector(std::uint8_t initial_capacity_exponent, std::uint8_t max_capacity_exponent);
    ~concurrent_vector();
    value_t& operator[](capacity_type index);
    const value_t& operator[](capacity_type index) const;
    value_t& at(capacity_type index);
    const value_t& at(capacity_type index) const;
private:
    struct alignas(alignof(value_t)) node
    {
	enum class status : uint8_t
	{
	    ready,
	    updating
	};
	value_t value;
	std::atomic<status> guard;
    };
    static const std::uint8_t capacity_base_ = 2U;
    subscript_type find_subscript(capacity_type index) const;
    void range_check(capacity_type index) const;
    const std::uint8_t initial_exponent_;
    const std::uint8_t max_exponent_;
    std::unique_ptr<std::atomic<node*>[]> segments_;
};

} // namespace container
} // namespace turbo

#endif
