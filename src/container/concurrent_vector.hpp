#ifndef TURBO_CONTAINER_CONCURRENT_VECTOR_HPP
#define TURBO_CONTAINER_CONCURRENT_VECTOR_HPP

#include <cstddef>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <string>

namespace turbo {
namespace container {

class invalid_capacity_argument : public std::invalid_argument
{
public:
    explicit invalid_capacity_argument(const char* what) : invalid_argument(what) { };
    explicit invalid_capacity_argument(const std::string& what) : invalid_argument(what) { };
};

class exceeded_capacity_error : public std::length_error
{
public:
    explicit exceeded_capacity_error(const char* what) : length_error(what) { };
    explicit exceeded_capacity_error(const std::string& what) : length_error(what) { };
};

///
/// Design taken from Dechev, Pirkelbauer & Stroustrup's Lock-free dynamically resizeable arrays paper
///
template <class value_t, template <class type_t> class allocator_t = std::allocator>
class concurrent_vector
{
public:
    typedef std::uint32_t capacity_type;
    concurrent_vector(std::uint8_t initial_capacity_exponent, std::uint8_t max_capacity_exponent);
    ~concurrent_vector();
    value_t& operator[](capacity_type index);
    const value_t& operator[](capacity_type index) const;
    value_t& at(capacity_type index);
    const value_t& at(capacity_type index) const;
private:
    static const std::uint8_t capacity_base_ = 2U;
    const std::uint8_t initial_exponent_;
    const std::uint8_t max_exponent_;
    std::unique_ptr<std::atomic<value_t*>[]> segments_;
};

} // namespace container
} // namespace turbo

#endif
