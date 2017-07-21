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
    block(const block& other);
    ~block() = default;
    block& operator=(const block& other);
    bool operator==(const block& other) const;
    inline std::size_t get_value_size() const { return value_size_; }
    inline std::size_t get_capacity() const { return capacity_; }
    inline std::size_t get_usable_size() const { return usable_size_; }
    inline const void* get_base_address() const { return base_; }
    inline bool is_empty() const { return storage_.get() == nullptr; }
    inline bool in_range(const void* pointer) const
    {
	if (is_empty())
	{
	    return false;
	}
	else
	{
	    return base_ <= pointer && pointer < (static_cast<std::uint8_t*>(base_) + usable_size_);
	}
    }
    void* allocate();
    void free(void* pointer);
private:
    typedef turbo::container::mpmc_ring_queue<capacity_type> free_list_type;
    block() = delete;
    block(block&&) = delete;
    block& operator=(block&&) = delete;
    std::size_t value_size_;
    std::size_t capacity_;
    std::size_t usable_size_;
    std::unique_ptr<std::uint8_t[]> storage_;
    std::uint8_t* base_;
    free_list_type free_list_;
};

typedef std::uint32_t capacity_type;

struct TURBO_SYMBOL_DECL block_config
{
    block_config();
    block_config(std::size_t size, capacity_type capacity);
    block_config(std::size_t size, capacity_type capacity, capacity_type contingency);
    block_config(std::size_t size, capacity_type capacity, capacity_type contingency, std::size_t growth);
    bool operator<(const block_config& other) const;
    bool operator==(const block_config& other) const;
    std::size_t block_size;
    capacity_type initial_capacity;
    capacity_type contingency_capacity;
    std::size_t growth_factor;
};

class TURBO_SYMBOL_DECL block_list
{
private:
    class node;
public:
    enum class append_result
    {
	success,
	beaten
    };
    enum class truncate_result
    {
	success,
	beaten
    };
    class invalid_dereference : public std::out_of_range
    {
    public:
	explicit inline invalid_dereference(const std::string& what) : out_of_range(what) { }
	explicit inline invalid_dereference(const char* what) : out_of_range(what) { }
    };
    template <class block_t, class node_t>
    class basic_iterator : public std::forward_iterator_tag
    {
    public:
	typedef block_t block_type;
	typedef node_t node_type;
	basic_iterator();
	basic_iterator(node_type* pointer);
	basic_iterator(const basic_iterator& other);
	~basic_iterator() = default;
	basic_iterator& operator=(const basic_iterator& other);
	bool operator==(const basic_iterator& other) const;
	inline bool operator!=(const basic_iterator& other) const { return !(*this == other); }
	basic_iterator& operator++();
	basic_iterator operator++(int);
	inline bool is_valid() const { return pointer_ != nullptr; }
	inline bool is_last() const { return is_valid() && pointer_->is_last(); }
	inline node_type& get_node() const { return *pointer_; }
    protected:
	node_type* pointer_;
    };
    class iterator : public basic_iterator<block, node>
    {
    public:
	typedef basic_iterator<block, node> base_iterator;
	iterator();
	iterator(node* pointer);
	iterator(const iterator& other);
	~iterator() = default;
	iterator& operator=(const iterator& other);
	bool operator==(const iterator& other) const;
	inline bool operator!=(const iterator& other) const { return !(*this == other); }
	block_type& operator*();
	block_type* operator->();
	append_result try_append(std::unique_ptr<node> successor);
	truncate_result try_truncate();
    };
    class const_iterator : public basic_iterator<const block, const node>
    {
    public:
	typedef basic_iterator<const block, const node> base_iterator;
	const_iterator();
	const_iterator(const node* pointer);
	const_iterator(const const_iterator& other);
	~const_iterator() = default;
	const_iterator& operator=(const const_iterator& other);
	bool operator==(const const_iterator& other) const;
	inline bool operator!=(const const_iterator& other) const { return !(*this == other); }
	block_type& operator*() const;
	block_type* operator->() const;
    };
    block_list(std::size_t value_size, block::capacity_type initial);
    block_list(std::size_t value_size, block::capacity_type initial, block::capacity_type contingency);
    block_list(std::size_t value_size, block::capacity_type initial, block::capacity_type contingency, std::size_t growth_factor);
    block_list(const block_config& config); // allow implicit conversion
    block_list(const block_list& other);
    ~block_list() noexcept = default;
    block_list& operator=(const block_list& other);
    bool operator==(const block_list& other) const;
    inline std::size_t get_value_size() const { return value_size_; }
    inline std::size_t get_growth_factor() const { return growth_factor_; }
    inline std::size_t get_contingency_capacity() const { return contingency_capacity_; }
    inline std::size_t get_list_size() const { return list_size_; }
    inline iterator begin() noexcept { return iterator(&first_); }
    inline iterator end() noexcept { return iterator(); }
    inline const_iterator cbegin() const noexcept { return const_iterator(&first_); }
    inline const_iterator cend() const noexcept { return const_iterator(); }
    std::unique_ptr<node> create_node(block::capacity_type capacity) const;
    std::unique_ptr<node> clone_node(const node& other) const;
    void* allocate();
private:
    class node
    {
    public:
	node(std::size_t value_size, block::capacity_type capacity);
	node(const node& other);
	~node() noexcept;
	node& operator=(const node& other);
	bool operator==(const node& other) const;
	inline const block& get_block() const { return block_; }
	inline block& mutate_block() { return block_; }
	inline const std::atomic<node*>& get_next() const { return next_; }
	inline std::atomic<node*>& mutate_next() { return next_; }
	inline bool is_last() const { return next_.load(std::memory_order_acquire) == nullptr; }
    private:
	node() = delete;
	node(node&&) = delete;
	node& operator=(node&&) = delete;
	block block_;
	std::atomic<node*> next_;
    };
    block_list() = delete;
    block_list(block_list&&) = delete;
    block_list& operator=(block_list&&) = delete;
    std::size_t value_size_;
    std::size_t growth_factor_;
    block::capacity_type contingency_capacity_;
    std::size_t list_size_;
    node first_;
};

} // namespace memory
} // namespace turbo

#endif
