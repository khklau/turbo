#ifndef TURBO_MEMORY_MARKED_PTR_HPP
#define TURBO_MEMORY_MARKED_PTR_HPP

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace memory {

class unaligned_ptr_error : public std::invalid_argument
{
public:
    explicit unaligned_ptr_error(const std::string& what) : invalid_argument(what) { }
    explicit unaligned_ptr_error(const char* what) : invalid_argument(what) { }
};

template <class value_t, class mark_t>
class marked_ptr
{
public:
    inline explicit marked_ptr(value_t* ptr)
	:
	    ptr_(ptr)
    {
	if ((reinterpret_cast<std::uintptr_t>(ptr) & mark_mask()) != 0U)
	{
	    throw unaligned_ptr_error("Given pointer is not a multiple of 4");
	}
    }
    inline marked_ptr(const marked_ptr<value_t, mark_t>& other)
	:
	    ptr_(other.ptr_)
    { }
    inline marked_ptr<value_t, mark_t>& operator=(const marked_ptr<value_t, mark_t>& other)
    {
	if (TURBO_LIKELY(this != &other))
	{
	    ptr_ = other.ptr_;
	}
	return *this;
    }
    ~marked_ptr() = default;
    inline value_t* get_ptr() const
    {
	return reinterpret_cast<value_t*>(reinterpret_cast<std::uintptr_t>(ptr_) & ptr_mask());
    }
    inline mark_t get_mark() const
    {
	return static_cast<mark_t>(reinterpret_cast<std::uintptr_t>(ptr_) & mark_mask());
    }
    inline bool operator==(const marked_ptr<value_t, mark_t>& other) const
    {
	return ptr_ == other.ptr_;
    }
    inline bool operator!=(const marked_ptr<value_t, mark_t>& other) const
    {
	return !(*this == other);
    }
    inline marked_ptr<value_t, mark_t> operator|(const mark_t mark) const
    {
	return marked_ptr<value_t, mark_t>(reinterpret_cast<value_t*>(reinterpret_cast<std::uintptr_t>(get_ptr()) | (static_cast<std::uintptr_t>(mark) & mark_mask())));
    }
    inline void reset(value_t* ptr)
    {
	ptr_ = ptr;
    }
    inline void set_mark(mark_t mark)
    {
	ptr_ = reinterpret_cast<value_t*>(reinterpret_cast<std::uintptr_t>(get_ptr()) | (static_cast<std::uintptr_t>(mark) & mark_mask()));
    }
    inline value_t& operator*()
    {
	return *get_ptr();
    }
    inline value_t* operator->()
    {
	return get_ptr();
    }
private:
    constexpr std::uintptr_t ptr_mask()
    {
	return std::numeric_limits<std::uintptr_t>::max() - 3U;
    }
    constexpr std::uintptr_t mark_mask()
    {
	return static_cast<std::uintptr_t>(3U);
    }
    value_t* ptr_;
};

} // namespace memory
} // namespace turbo

#endif
