#ifndef TURBO_CONTAINER_CONCURRENT_UNORDERED_MAP
#define TURBO_CONTAINER_CONCURRENT_UNORDERED_MAP

#include <functional>
#include <iterator>
#include <mutex>
#include <utility>
#include <vector>
#include <turbo/memory/cstdlib_allocator.hpp>
#include <turbo/threading/shared_mutex.hpp>

namespace turbo {
namespace container {

template<typename key_t, typename element_t, typename hash_f = std::hash<key_t>, class typed_allocator_t = turbo::memory::cstdlib_typed_allocator>
class concurrent_unordered_map
{
public:
    typedef key_t key_type;
    typedef element_t mapped_type;
    typedef std::pair<key_type, mapped_type> value_type;
    typedef hash_f hasher;
    typedef typed_allocator_t allocator_type;

private:
    class bucket;
    typedef std::vector<value_type> bucket_storage_type;
    typedef typename bucket_storage_type::const_iterator const_storage_iterator;
    typedef typename bucket_storage_type::iterator storage_iterator;
    typedef std::vector<bucket> bucket_group_type;
    typedef typename bucket_group_type::const_iterator const_group_iterator;
    typedef typename bucket_group_type::iterator group_iterator;
    struct bound_accessor
    {
	static inline storage_iterator begin(group_iterator& group_iter, group_iterator& group_end)
	{
	    return group_iter != group_end ? group_iter->begin() : storage_iterator();
	}
	static inline storage_iterator end(group_iterator& group_iter, group_iterator& group_end)
	{
	    return group_iter != group_end ? group_iter->end() : storage_iterator();
	}
    };
    struct const_bound_accessor
    {
	static inline const_storage_iterator begin(const const_group_iterator& group_iter, const const_group_iterator& group_end)
	{
	    return group_iter != group_end ? group_iter->cbegin() : const_storage_iterator();
	}
	static inline const_storage_iterator end(const const_group_iterator& group_iter, const const_group_iterator& group_end)
	{
	    return group_iter != group_end ? group_iter->cend() : const_storage_iterator();
	}
    };

public:
    template <class value_t, class storage_iterator_t, class group_iterator_t, class bound_t>
    class basic_iterator : public std::forward_iterator_tag
    {
    public:
	typedef value_t value_type;
	typedef value_t* pointer;
	typedef value_t& reference;
	typedef std::ptrdiff_t difference_type;
	typedef std::forward_iterator_tag iterator_category;
	typedef storage_iterator_t storage_iterator_type;
	typedef group_iterator_t group_iterator_type;
	typedef bound_t bound;
	basic_iterator(
		const group_iterator_t& group_end,
		const group_iterator_t& group_iter);
	basic_iterator(
		const group_iterator_t& group_end,
		const group_iterator_t& group_iter,
		const storage_iterator_t& storage_iter,
		turbo::threading::shared_mutex& storage_mutex);
	inline value_t& operator*() { return *storage_iter_; }
	inline value_t* operator->() { return &(*storage_iter_); }
	basic_iterator& operator++();
	basic_iterator& operator++(int);
    private:
	group_iterator_t group_end_;
	group_iterator_t group_iter_;
	storage_iterator_t storage_iter_;
	turbo::threading::shared_mutex* storage_mutex_;
    };

    typedef basic_iterator<value_type, storage_iterator, group_iterator, bound_accessor> iterator;
    typedef basic_iterator<const value_type, const_storage_iterator, const_group_iterator, const_bound_accessor> const_iterator;

    explicit concurrent_unordered_map(
	    allocator_type& allocator,
	    std::size_t min_buckets = 64U,
	    const hasher& hash_func = hasher());

    inline bool empty() const
    {
	return group_.empty();
    }
    inline iterator begin()
    {
	return group_.empty()
		? end()
		: iterator(group_.end(), group_.begin(), group_.begin()->begin(), group_.begin()->mutex());
    }
    inline const_iterator cbegin()
    {
	return group_.empty()
		? cend()
		: const_iterator(group_.cend(), group_.cbegin(), group_.cbegin()->cbegin(), group_.begin()->mutex());
    }
    inline iterator end()
    {
	return iterator(group_.end(), group_.end());
    }
    inline const_iterator cend()
    {
	return const_iterator(group_.cend(), group_.cend());
    }
private:
    class bucket
    {
    public:
	bucket() = default;
	inline turbo::threading::shared_mutex& mutex()
	{
	    return mutex_;
	}
	inline const_storage_iterator cbegin() const
	{
	    return storage_.cbegin();
	}
	inline storage_iterator begin()
	{
	    return storage_.begin();
	}
	inline const_storage_iterator cend() const
	{
	    return storage_.cend();
	}
	inline storage_iterator end()
	{
	    return storage_.end();
	}
	const_storage_iterator find(const key_type& key) const;
	storage_iterator find(const key_type& key);
	storage_iterator erase(const_storage_iterator& position);
	storage_iterator push_back(value_type&& value);
    private:
	bucket_storage_type storage_;
	mutable turbo::threading::shared_mutex mutex_;
    };
    allocator_type& allocator_;
    const hasher& hash_func_;
    bucket_group_type group_;
};

} // namespace container
} // namespace turbo

#endif
