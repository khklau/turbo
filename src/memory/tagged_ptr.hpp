#ifndef TURBO_MEMORY_TAGGED_PTR_HPP
#define TURBO_MEMORY_TAGGED_PTR_HPP

#include <cstdint>
#include <atomic>
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
    typedef value_t value_type;
    typedef tag_t tag_type;
    typedef value_t* value_ptr_type;
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
    ~tagged_ptr() noexcept = default;
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
    friend class std::atomic<tagged_ptr<value_t, tag_t>>;
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

namespace std {

///
/// Partial specialisation of std::atomic for tagged_ptr
///
template <class value_t, class tag_t>
struct atomic<turbo::memory::tagged_ptr<value_t, tag_t>> : public std::atomic<typename turbo::memory::tagged_ptr<value_t, tag_t>::value_ptr_type>
{
    typedef turbo::memory::tagged_ptr<value_t, tag_t> tagged_ptr_type;
    typedef std::atomic<typename tagged_ptr_type::value_ptr_type> base_type;
    atomic() noexcept = default;
    constexpr atomic(tagged_ptr_type value) noexcept
	:
	    base_type(value.ptr_)
    { }
    atomic(const atomic&) = delete;
    ~atomic() noexcept = default;
    using base_type::is_lock_free;
    inline void store(tagged_ptr_type value, memory_order sync = memory_order_seq_cst) volatile noexcept
    {
	base_type::store(value.ptr_, sync);
    }
    inline void store(tagged_ptr_type value, memory_order sync = memory_order_seq_cst) noexcept
    {
	base_type::store(value.ptr_, sync);
    }
    inline tagged_ptr_type load(memory_order sync = memory_order_seq_cst) const volatile noexcept
    {
	tagged_ptr_type tmp;
	tmp.reset(base_type::load(sync));
	return tmp;
    }
    inline tagged_ptr_type load(memory_order sync = memory_order_seq_cst) const noexcept
    {
	tagged_ptr_type tmp;
	tmp.reset(base_type::load(sync));
	return tmp;
    }
    operator tagged_ptr_type() const volatile noexcept
    {
	return load();
    }
    operator tagged_ptr_type() const noexcept
    {
	return load();
    }
    inline tagged_ptr_type exchange(tagged_ptr_type value, memory_order sync = memory_order_seq_cst) volatile noexcept
    {
	return tagged_ptr_type(base_type::exchange(value.ptr_, sync));
    }
    inline tagged_ptr_type exchange(tagged_ptr_type value, memory_order sync = memory_order_seq_cst) noexcept
    {
	return tagged_ptr_type(base_type::exchange(value.ptr_, sync));
    }
    inline bool compare_exchange_weak(tagged_ptr_type& expected, tagged_ptr_type value, memory_order sync = memory_order_seq_cst) volatile noexcept
    {
	return base_type::compare_exchange_weak(expected.ptr_, value.ptr_, sync);
    }
    inline bool compare_exchange_weak(tagged_ptr_type& expected, tagged_ptr_type value, memory_order sync = memory_order_seq_cst) noexcept
    {
	return base_type::compare_exchange_weak(expected.ptr_, value.ptr_, sync);
    }
    inline bool compare_exchange_weak(tagged_ptr_type& expected, tagged_ptr_type value, memory_order success, memory_order failure) volatile noexcept
    {
	return base_type::compare_exchange_weak(expected.ptr_, value.ptr_, success, failure);
    }
    inline bool compare_exchange_weak(tagged_ptr_type& expected, tagged_ptr_type value, memory_order success, memory_order failure) noexcept
    {
	return base_type::compare_exchange_weak(expected.ptr_, value.ptr_, success, failure);
    }
    inline bool compare_exchange_strong(tagged_ptr_type& expected, tagged_ptr_type value, memory_order sync = memory_order_seq_cst) volatile noexcept
    {
	return base_type::compare_exchange_strong(expected.ptr_, value.ptr_, sync);
    }
    inline bool compare_exchange_strong(tagged_ptr_type& expected, tagged_ptr_type value, memory_order sync = memory_order_seq_cst) noexcept
    {
	return base_type::compare_exchange_strong(expected.ptr_, value.ptr_, sync);
    }
    inline bool compare_exchange_strong(tagged_ptr_type& expected, tagged_ptr_type value, memory_order success, memory_order failure) volatile noexcept
    {
	return base_type::compare_exchange_strong(expected.ptr_, value.ptr_, success, failure);
    }
    inline bool compare_exchange_strong(tagged_ptr_type& expected, tagged_ptr_type value, memory_order success, memory_order failure) noexcept
    {
	return base_type::compare_exchange_strong(expected.ptr_, value.ptr_, success, failure);
    }
};

} // namespace std

#endif
