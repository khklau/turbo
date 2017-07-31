#include "block.hpp"
#include "block.hxx"
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

block_config::block_config()
    :
	block_config(0U, 0U, 0U, 0U)
{ }

block_config::block_config(std::size_t size, capacity_type initial)
    :
	block_config(size, initial, 0U, 2U)
{ }

block_config::block_config(std::size_t size, capacity_type initial, capacity_type contingency)
    :
	block_config(size, initial, contingency, 2U)
{ }

block_config::block_config(std::size_t size, capacity_type initial, capacity_type contingency, std::size_t growth)
    :
	block_size(size),
	initial_capacity(initial),
	contingency_capacity(contingency),
	growth_factor(growth < 2U ? 2U : growth)
{
    if (block_size != 0U && initial_capacity == 0U && contingency_capacity == 0U)
    {
	throw std::invalid_argument("block_config - the initial and contingency capacity arguments cannot both be 0");
    }
}

bool block_config::operator<(const block_config& other) const
{
    return block_size < other.block_size;
}

bool block_config::operator==(const block_config& other) const
{
    return block_size == other.block_size
	    && initial_capacity == other.initial_capacity
	    && growth_factor == other.growth_factor;
}

block_list::iterator::iterator()
    :
	base_iterator()
{ }

block_list::iterator::iterator(block_list::node* pointer)
    :
	base_iterator(pointer)
{ }

block_list::iterator::iterator(const iterator& other)
    :
	base_iterator(static_cast<const base_iterator&>(other))
{ }

block_list::iterator& block_list::iterator::operator=(const iterator& other)
{
    static_cast<base_iterator&>(*this) = static_cast<const base_iterator&>(other);
    return *this;
}

bool block_list::iterator::operator==(const iterator& other) const
{
    return static_cast<const base_iterator&>(*this) == static_cast<const base_iterator&>(other);
}

typename block_list::iterator::block_type& block_list::iterator::operator*()
{
    if (TURBO_LIKELY(is_valid()))
    {
	return pointer_->mutate_block();
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::basic_iterator");
    }
}

typename block_list::iterator::block_type* block_list::iterator::operator->()
{
    if (TURBO_LIKELY(is_valid()))
    {
	return &(pointer_->mutate_block());
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::basic_iterator");
    }
}

block_list::append_result block_list::iterator::try_append(std::unique_ptr<block_list::node> successor)
{
    if (TURBO_LIKELY(is_valid()))
    {
	successor->mutate_next().store(nullptr, std::memory_order_release);
	node* next = pointer_->get_next().load(std::memory_order_acquire);
	if (next == nullptr)
	{
	    if (pointer_->mutate_next().compare_exchange_strong(next, successor.get(), std::memory_order_release))
	    {
		successor.release();
		return block_list::append_result::success;
	    }
	}
	return block_list::append_result::beaten;
    }
    else
    {
	throw block_list::invalid_dereference("cannot append to invalid block_list::iterator");
    }
}

block_list::truncate_result block_list::iterator::try_truncate()
{
    if (TURBO_LIKELY(is_valid()))
    {
	node* next = pointer_->get_next().load(std::memory_order_acquire);
	if (next != nullptr)
	{
	    if (pointer_->mutate_next().compare_exchange_strong(next, nullptr, std::memory_order_release))
	    {
		delete next;
		return block_list::truncate_result::success;
	    }
	    else
	    {
		return block_list::truncate_result::beaten;
	    }
	}
	else
	{
	    // already truncated, nothing to do
	    return block_list::truncate_result::success;
	}
    }
    else
    {
	throw block_list::invalid_dereference("cannot truncate an invalid block_list::iterator");
    }
}

block_list::const_iterator::const_iterator()
    :
	base_iterator()
{ }

block_list::const_iterator::const_iterator(const block_list::node* pointer)
    :
	base_iterator(pointer)
{ }

block_list::const_iterator::const_iterator(const const_iterator& other)
    :
	base_iterator(static_cast<const base_iterator&>(other))
{ }

block_list::const_iterator& block_list::const_iterator::operator=(const const_iterator& other)
{
    static_cast<base_iterator&>(*this) = static_cast<const base_iterator&>(other);
    return *this;
}

bool block_list::const_iterator::operator==(const const_iterator& other) const
{
    return static_cast<const base_iterator&>(*this) == static_cast<const base_iterator&>(other);
}

typename block_list::const_iterator::block_type& block_list::const_iterator::operator*() const
{
    if (TURBO_LIKELY(is_valid()))
    {
	return pointer_->get_block();
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::basic_const_iterator");
    }
}

typename block_list::const_iterator::block_type* block_list::const_iterator::operator->() const
{
    if (TURBO_LIKELY(is_valid()))
    {
	return &(pointer_->get_block());
    }
    else
    {
	throw block_list::invalid_dereference("cannot dereference invalid block_list::basic_const_iterator");
    }
}

block_list::node::node(std::size_t value_size, block::capacity_type capacity)
    :
	block_(value_size, capacity, value_size),
	next_(nullptr)
{ }

block_list::node::node(const node& other)
    :
	block_(other.block_),
	next_(nullptr)
{ }

block_list::node::~node() noexcept
{
    try
    {
	node* next = next_.load(std::memory_order_acquire);
	if (next != nullptr)
	{
	    delete next;
	}
    }
    catch (...)
    {
	// Do nothing
    }
}

block_list::node& block_list::node::operator=(const node& other)
{
    if (this != &other)
    {
	this->block_ = other.block_;
    }
    return *this;
}

bool block_list::node::operator==(const node& other) const
{
    node* this_next = this->next_.load(std::memory_order_acquire);
    node* other_next = other.next_.load(std::memory_order_acquire);
    return this->block_ == other.block_
	&& ((this_next == nullptr && other_next == nullptr) || (this_next != nullptr && other_next != nullptr));
}

block_list::block_list(std::size_t value_size, block::capacity_type initial)
    :
	block_list(value_size, initial, 0U, 2U)
{ }

block_list::block_list(std::size_t value_size, block::capacity_type initial, block::capacity_type contingency)
    :
	block_list(value_size, initial, contingency, 2U)
{ }

block_list::block_list(std::size_t value_size, block::capacity_type initial, block::capacity_type contingency, std::size_t growth_factor)
    :
	value_size_(value_size),
	growth_factor_(growth_factor),
	contingency_capacity_(contingency),
	list_size_(1U),
	first_(value_size, initial)
{
    if (value_size == 0U)
    {
	throw std::invalid_argument("block_list - the value size argument cannot be 0");
    }
    else if (initial == 0U && contingency == 0U)
    {
	throw std::invalid_argument("block_list - the initial and contingency capacity arguments cannot both be 0");
    }
}

block_list::block_list(const block_config& config)
    :
	value_size_(config.block_size),
	growth_factor_(config.growth_factor),
	contingency_capacity_(config.contingency_capacity),
	list_size_(1U),
	first_(config.block_size, config.initial_capacity)
{ }

block_list::block_list(const block_list& other)
    :
	value_size_(other.value_size_),
	growth_factor_(other.growth_factor_),
	contingency_capacity_(other.contingency_capacity_),
	list_size_(other.list_size_),
	first_(other.first_)
{
    auto this_iter = this->begin();
    auto other_iter = other.cbegin();
    // first_ has already been copied, so skip it
    ++other_iter;
    for (; other_iter != other.cend(); ++other_iter)
    {
	if (other_iter.is_valid())
	{
	    this_iter.try_append(clone_node(other_iter.get_node()));
	    ++this_iter;
	}
    }
}

block_list& block_list::operator=(const block_list& other)
{
    if (this != &other && this->value_size_ == other.value_size_ && this->growth_factor_ && other.growth_factor_)
    {
	auto this_iter = this->begin();
	auto other_iter = other.cbegin();
	for (; other_iter != other.cend(); ++this_iter, ++other_iter)
	{
	    this_iter.get_node() = other_iter.get_node();
	    if (!other_iter.is_last() && this_iter.is_last())
	    {
		this_iter.try_append(create_node(other_iter->get_capacity() * other.get_growth_factor()));
	    }
	    else if (other_iter.is_last() && !this_iter.is_last())
	    {
		this_iter.try_truncate();
	    }
	}
	this->contingency_capacity_ = other.contingency_capacity_;
	this->list_size_ = other.list_size_;
    }
    return *this;
}

bool block_list::operator==(const block_list& other) const
{
    bool is_list_matching = true;
    auto this_iter = this->cbegin();
    for (auto other_iter = other.cbegin(); other_iter != other.cend() && this_iter != this->cend();)
    {
	is_list_matching = is_list_matching && (*this_iter == *other_iter);
	++this_iter;
	++other_iter;
    }
    return this->value_size_ == other.value_size_
	&& this->growth_factor_ == other.growth_factor_
	&& this->contingency_capacity_ == other.contingency_capacity_
	&& this->list_size_ == other.list_size_
	&& is_list_matching;
}

std::unique_ptr<block_list::node> block_list::create_node(block::capacity_type capacity) const
{
    return std::move(std::unique_ptr<block_list::node>(new block_list::node(value_size_, capacity)));
}

std::unique_ptr<block_list::node> block_list::clone_node(const node& other) const
{
    return std::move(std::unique_ptr<block_list::node>(new block_list::node(other)));
}

void* block_list::allocate()
{
    void* allocation = nullptr;
    for (auto iter = begin(); allocation == nullptr && iter != end(); ++iter)
    {
	allocation = iter->allocate();
	if (allocation == nullptr && iter.is_last())
	{
	    block::capacity_type capacity = iter->is_empty() ?
		    contingency_capacity_ :
		    iter->get_capacity() * get_growth_factor();
	    iter.try_append(std::move(create_node(capacity)));
	    ++list_size_;
	}
    }
    return allocation;
}

void block_list::free(void* pointer)
{
    for (auto&& block : *this)
    {
	if (block.in_range(pointer))
	{
	    block.free(pointer);
	    break;
	}
    }
}

} // namespace memory
} // namespace turbo
