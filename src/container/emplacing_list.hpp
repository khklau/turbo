#ifndef TURBO_CONTAINER_EMPLACING_LIST_HPP
#define TURBO_CONTAINER_EMPLACING_LIST_HPP

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <turbo/memory/typed_allocator.hpp>

namespace turbo {
namespace container {

template <class value_t>
using list_unique_ptr = std::unique_ptr<value_t, std::function<void (value_t*)>>;


template <class value_t, class typed_allocator_t = turbo::memory::typed_allocator>
class emplacing_list
{
private:
    struct node;
public:
    typedef value_t value_type;
    typedef typed_allocator_t typed_allocator_type;
    class invalid_dereference : public std::out_of_range
    {
    public:
	explicit inline invalid_dereference(const std::string& what) : out_of_range(what) { }
	explicit inline invalid_dereference(const char* what) : out_of_range(what) { }
    };
    class iterator : public std::bidirectional_iterator_tag
    {
    public:
	iterator();
	iterator(const std::shared_ptr<node>& pointer);
	iterator(const iterator& other);
	iterator& operator=(const iterator& other);
	~iterator() = default;
	bool operator==(const iterator& other) const;
	inline bool operator!=(const iterator& other) const { return !(*this == other); }
	value_type& operator*();
	value_type* operator->();
	iterator& operator++();
	iterator operator++(int);
	iterator& operator--();
	iterator operator--(int);
	inline bool is_valid() const { return pointer_.use_count() != 0; }
	inline bool is_first() const { return is_valid() && pointer_->is_first(); }
	inline bool is_last() const { return is_valid() && pointer_->is_last(); }
    private:
	std::shared_ptr<node> pointer_;
    };
    static constexpr std::size_t allocation_size()
    {
	return sizeof(node);
    }
    static constexpr std::size_t allocation_alignment()
    {
	return alignof(node);
    }
    explicit emplacing_list(typed_allocator_type& allocator);
    ~emplacing_list();
    inline iterator begin() noexcept
    {
	return iterator(front_);
    }
    template <class... args_t>
    void emplace_front(args_t&&... args);
private:
    struct node : public std::enable_shared_from_this<node>
    {
	template <class... args_t>
	node(args_t&&... args);
	inline bool is_first() const { return previous.expired(); }
	inline bool is_last() const { return next.use_count() == 0; }
	value_t value;
	std::shared_ptr<node> next;
	std::weak_ptr<node> previous;
    };
    template <class... args_t>
    node* create_node(args_t&&... args);
    void destroy_node(node* pointer);
    typed_allocator_type& allocator_;
    std::shared_ptr<node> front_;
    std::weak_ptr<node> back_;
};

} // namespace container
} // namespace turbo

#endif
