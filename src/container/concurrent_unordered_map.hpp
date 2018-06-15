#ifndef TURBO_CONTAINER_CONCURRENT_UNORDERED_MAP
#define TURBO_CONTAINER_CONCURRENT_UNORDERED_MAP

#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <utility>
#include <tuple>
#include <vector>
#include <turbo/memory/slab_allocator.hpp>
#include <turbo/threading/shared_mutex.hpp>

namespace turbo {
namespace container {

template<typename key_t, typename element_t, typename hash_f = std::hash<key_t>, class typed_allocator_t = turbo::memory::concurrent_sized_slab>
class concurrent_unordered_map
{
public:
    typedef key_t key_type;
    typedef element_t mapped_type;
    typedef std::pair<key_type, mapped_type> value_type;
    typedef std::shared_ptr<value_type> shared_value_type;
    typedef hash_f hasher;
    typedef typed_allocator_t allocator_type;

private:
    class bucket;
    typedef std::vector<shared_value_type> bucket_storage_type;
    typedef typename bucket_storage_type::const_iterator const_storage_iterator;
    typedef typename bucket_storage_type::iterator storage_iterator;
    typedef std::vector<bucket> bucket_group_type;
    typedef typename bucket_group_type::const_iterator const_group_iterator;
    typedef typename bucket_group_type::iterator group_iterator;
    struct bound_accessor
    {
	static inline storage_iterator begin(bucket_group_type* bucket_group, std::size_t bucket_id)
	{
	    return (bucket_group && bucket_id < bucket_group->size()) ? (*bucket_group)[bucket_id].begin() : storage_iterator();
	}
	static inline storage_iterator end(bucket_group_type* bucket_group, std::size_t bucket_id)
	{
	    return (bucket_group && bucket_id < bucket_group->size()) ? (*bucket_group)[bucket_id].end() : storage_iterator();
	}
    };
    struct const_bound_accessor
    {
	static inline const_storage_iterator begin(const bucket_group_type* bucket_group, std::size_t bucket_id)
	{
	    return (bucket_group && bucket_id < bucket_group->size()) ? (*bucket_group)[bucket_id].cbegin() : storage_iterator();
	}
	static inline const_storage_iterator end(const  bucket_group_type* bucket_group, std::size_t bucket_id)
	{
	    return (bucket_group && bucket_id < bucket_group->size()) ? (*bucket_group)[bucket_id].cend() : storage_iterator();
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
		bucket_group_type& bucket_group,
		turbo::threading::shared_mutex& group_mutex);
	basic_iterator(
		bucket_group_type& bucket_group,
		turbo::threading::shared_mutex& group_mutex,
		std::size_t bucket_id,
		const storage_iterator_t& storage_iter,
		turbo::threading::shared_mutex& storage_mutex);
	bool operator==(const basic_iterator& other) const;
	inline bool operator!=(const basic_iterator& other) const { return !(*this == other); }
	inline value_t& operator*() { return *storage_iter_; }
	inline value_t* operator->() { return &(*storage_iter_); }
	basic_iterator& operator++();
	basic_iterator& operator++(int);
    private:
	bucket_group_type* bucket_group_;
	turbo::threading::shared_mutex* group_mutex_;
	std::size_t bucket_id_;
	storage_iterator_t storage_iter_;
	turbo::threading::shared_mutex* storage_mutex_;
    };

    typedef basic_iterator<shared_value_type, storage_iterator, group_iterator, bound_accessor> iterator;
    typedef basic_iterator<const shared_value_type, const_storage_iterator, const_group_iterator, const_bound_accessor> const_iterator;

    enum class emplace_result
    {
	success,
	key_exists,
	beaten,
	allocator_full
    };

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
		: iterator(group_, mutex_, 0U, group_[0U].begin(), group_[0U].mutex());
    }
    inline const_iterator cbegin()
    {
	return group_.empty()
		? cend()
		: const_iterator(group_, mutex_, 0U, group_[0U].cbegin(), group_[0U].mutex());
    }
    inline iterator end()
    {
	return iterator(group_, mutex_);
    }
    inline const_iterator cend()
    {
	return const_iterator(group_, mutex_);
    }

    const_iterator find(const key_type& key) const;
    iterator find(const key_type& key);

    template <class... key_args_t, class... value_args_t>
    emplace_result try_emplace(std::tuple<key_args_t...>&& key_args, std::tuple<value_args_t...>&& value_args);
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
	storage_iterator push_back(shared_value_type&& value);
    private:
	bucket_storage_type storage_;
	mutable turbo::threading::shared_mutex mutex_;
    };
    allocator_type& allocator_;
    const hasher& hash_func_;
    bucket_group_type group_;
    mutable turbo::threading::shared_mutex mutex_;
};

} // namespace container
} // namespace turbo

#endif
