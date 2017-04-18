#include "block.hpp"
#include <cstring>
#include <algorithm>
#include <turbo/algorithm/recovery.hpp>
#include <turbo/algorithm/recovery.hxx>
#include <turbo/memory/alignment.hpp>
#include <turbo/container/mpmc_ring_queue.hxx>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace memory {

out_of_memory_error::out_of_memory_error(const std::string& what)
    :
	runtime_error(what)
{ }

out_of_memory_error::out_of_memory_error(const char* what)
    :
	runtime_error(what)
{ }

invalid_size_error::invalid_size_error(const std::string& what)
    :
	invalid_argument(what)
{ }

invalid_size_error::invalid_size_error(const char* what)
    :
	invalid_argument(what)
{ }

invalid_alignment_error::invalid_alignment_error(const std::string& what)
    :
	invalid_argument(what)
{ }

invalid_alignment_error::invalid_alignment_error(const char* what)
    :
	invalid_argument(what)
{ }

invalid_pointer_error::invalid_pointer_error(const std::string& what)
    :
	invalid_argument(what)
{ }

invalid_pointer_error::invalid_pointer_error(const char* what)
    :
	invalid_argument(what)
{ }

block::block(std::size_t value_size, capacity_type capacity)
    :
	block(value_size, capacity, alignof(void*))
{ }

block::block(std::size_t value_size, capacity_type capacity, std::size_t alignment)
    :
	value_size_(value_size),
	capacity_(capacity),
	usable_size_(capacity == 0U ? 0U : value_size_ * (capacity_ + 1)), // need extra in case of bad alignment
	storage_(capacity == 0U ? nullptr : new std::uint8_t[usable_size_]),
	base_(capacity == 0U ? nullptr : &(storage_[0])),
	free_list_(capacity)
{
    if (TURBO_UNLIKELY(value_size_ == 0))
    {
	throw invalid_size_error("value size cannot be 0");
    }
    if (capacity_ == 0U)
    {
	return;
    }
    if (TURBO_UNLIKELY(!storage_))
    {
	throw out_of_memory_error("insufficient space in heap");
    }
    std::fill_n(base_, usable_size_, 0U);
    void* tmp = base_;
    if (TURBO_UNLIKELY(!turbo::memory::align(alignment, value_size_, tmp, usable_size_)))
    {
	throw invalid_alignment_error("alignment exceeds requested total size");
    }
    base_ = static_cast<std::uint8_t*>(tmp);
    capacity_ = usable_size_ / value_size;
    if (TURBO_UNLIKELY(capacity_ < capacity))
    {
	throw out_of_memory_error("available space in heap is insufficient for required alignment");
    }
    else
    {
	capacity_ = capacity;
	for (capacity_type index = 0; index < capacity_; ++index)
	{
	    free_list_.try_enqueue_copy(index);
	}
    }
}

block::block(const block& other)
    :
	value_size_(other.value_size_),
	capacity_(other.capacity_),
	usable_size_(other.usable_size_),
	storage_(capacity_ == 0U ? nullptr : new std::uint8_t[usable_size_]),
	base_(capacity_ == 0U ? nullptr : &(storage_[0])),
	free_list_(other.free_list_)
{
    if (storage_.get() != nullptr)
    {
	std::copy_n(&(other.storage_[0]), usable_size_, &(this->storage_[0]));
    }
}

block& block::operator=(const block& other)
{
    if (this != &other
	    && this->value_size_ == other.value_size_
	    && this->capacity_ == other.capacity_
	    && this->usable_size_ == other.usable_size_)
    {
	if (this->storage_.get() != nullptr && other.storage_.get() != nullptr)
	{
	    std::copy_n(&(other.storage_[0]), this->usable_size_, &(this->storage_[0]));
	}
	this->free_list_ = other.free_list_;
    }
    return *this;
}

bool block::operator==(const block& other) const
{
    return this->value_size_ == other.value_size_
	&& this->capacity_ == other.capacity_
	&& this->usable_size_ == other.usable_size_
	&& std::memcmp(this->storage_.get(), other.storage_.get(), usable_size_) == 0
	&& this->free_list_ == other.free_list_;
}

void* block::allocate()
{
    namespace tar = turbo::algorithm::recovery;
    capacity_type reservation = 0U;
    void* result = nullptr;
    if (is_empty())
    {
	return nullptr;
    }
    tar::retry_with_random_backoff([&] () -> tar::try_state
    {
	switch (free_list_.try_dequeue_copy(reservation))
	{
	    case free_list_type::consumer::result::queue_empty:
	    {
		// no free blocks available
		result = nullptr;
		return tar::try_state::done;
	    }
	    case free_list_type::consumer::result::success:
	    {
		result = &(base_[reservation * value_size_]);
		return tar::try_state::done;
	    }
	    default:
	    {
		return tar::try_state::retry;
	    }
	}
    });
    return result;
}

void block::free(void* pointer)
{
    namespace tar = turbo::algorithm::recovery;
    if (is_empty() || pointer == nullptr)
    {
	return;
    }
    std::size_t diff = static_cast<std::uint8_t*>(pointer) - base_;
    if (diff % value_size_ != 0)
    {
	throw invalid_pointer_error("address points to the middle of a value");
    }
    std::size_t offset = diff / value_size_;
    if (offset < (usable_size_ / value_size_))
    {
	tar::retry_with_random_backoff([&] () -> tar::try_state
	{
	    switch (free_list_.try_enqueue_copy(offset))
	    {
		case free_list_type::producer::result::queue_full:
		{
		    // log a warning?
		    return tar::try_state::done;
		}
		case free_list_type::producer::result::success:
		{
		    return tar::try_state::done;
		}
		default:
		{
		    return tar::try_state::retry;
		}
	    }
	});
    }
    else
    {
	throw invalid_pointer_error("given address does not come from this block");
    }
}

} // namespace memory
} // namespace turbo
