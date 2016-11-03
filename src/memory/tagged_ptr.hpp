#ifndef TURBO_MEMORY_TAGGED_PTR_HPP
#define TURBO_MEMORY_TAGGED_PTR_HPP

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

template <class value_t, class tag_t>
class tagged_ptr
{
public:
    inline tagged_ptr() noexcept
	:
	    ptr_(nullptr)
    { }
    inline explicit tagged_ptr(value_t* ptr)
	:
	    ptr_(ptr)
    {
	if ((reinterpret_cast<std::uintptr_t>(ptr) & tag_mask()) != 0U)
	{
	    throw unaligned_ptr_error("Given pointer is not a multiple of 4");
	}
    }
    tagged_ptr(const tagged_ptr<value_t, tag_t>& other) = default;
    tagged_ptr(tagged_ptr<value_t, tag_t>&& other) = default;
    tagged_ptr<value_t, tag_t>& operator=(const tagged_ptr<value_t, tag_t>& other) = default;
    tagged_ptr<value_t, tag_t>& operator=(tagged_ptr<value_t, tag_t>&& other) = default;
    ~tagged_ptr() = default;
    inline value_t* get_ptr() const
    {
	return reinterpret_cast<value_t*>(reinterpret_cast<std::uintptr_t>(ptr_) & ptr_mask());
    }
    inline tag_t get_tag() const
    {
	return static_cast<tag_t>(reinterpret_cast<std::uintptr_t>(ptr_) & tag_mask());
    }
    inline bool is_empty() const
    {
	return get_ptr() == nullptr;
    }
    inline void set_tag(tag_t tag)
    {
	ptr_ = reinterpret_cast<value_t*>(reinterpret_cast<std::uintptr_t>(get_ptr()) | (static_cast<std::uintptr_t>(tag) & tag_mask()));
    }
    inline bool operator==(const tagged_ptr<value_t, tag_t>& other) const
    {
	return ptr_ == other.ptr_;
    }
    inline bool operator!=(const tagged_ptr<value_t, tag_t>& other) const
    {
	return !(*this == other);
    }
    inline tagged_ptr<value_t, tag_t> operator|(const tag_t tag) const
    {
	tagged_ptr<value_t, tag_t> tmp;
	tmp.set_tag(tag);
	return tmp;
    }
    inline void reset(value_t* ptr)
    {
	ptr_ = ptr;
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
    constexpr std::uintptr_t tag_mask()
    {
	return static_cast<std::uintptr_t>(3U);
    }
    value_t* ptr_;
};

} // namespace memory
} // namespace turbo

#endif
