#ifndef TURBO_CONTANER_HEAP_HPP
#define TURBO_CONTANER_HEAP_HPP

#include <functional>
#include <memory>
#include <vector>

namespace turbo {
namespace container {

template <class element_t, class compare_f = std::greater<element_t>, class allocator_t = std::allocator<element_t>>
class heap_tester;

template <class element_t, class compare_f = std::greater<element_t>, class allocator_t = std::allocator<element_t>>
class heap
{
public:
    typedef element_t element_type;
    typedef compare_f compare_func;
    typedef allocator_t allocator_type;
    heap() = default;
    ~heap() = default;
    inline std::size_t size()
    {
	return data_.size();
    }
    inline element_type& front()
    {
	return data_.front();
    }
    inline const element_type& front() const
    {
	return data_.front();
    }
    inline element_type& back()
    {
	return data_.back();
    }
    inline const element_type& back() const
    {
	return data_.back();
    }
    template <class... args_t>
    void emplace_back(args_t&&... args);
    friend class heap_tester<element_t, compare_f, allocator_t>;
private:
    enum class designation
    {
	left,
	right
    };
    template <class vector_t, class node_t>
    inline static std::size_t index(vector_t& data, node_t& node);
    inline designation which_child(const element_type& node) const;
    inline bool is_root(const element_type& node) const;
    inline bool is_leaf(const element_type& node) const;
    inline element_type* parent(element_type& node);
    template <class vector_t, class node_t>
    inline static node_t* left_child(vector_t& data, node_t& node);
    template <class vector_t, class node_t>
    inline static node_t* right_child(vector_t& data, node_t& node);
    inline element_type* sibling(element_type& node);
    bool has_heap_property() const;
    void sift_root(element_type& node);
    void sift_leaf(element_type& node);
    std::vector<element_t> data_;
};

} // namespace container
} // namespace turbo

#endif
