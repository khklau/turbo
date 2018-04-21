#ifndef TURBO_ALGORITHM_SEQUENCE_HXX
#define TURBO_ALGORITHM_SEQUENCE_HXX

#include "sequence.hpp"
#include <algorithm>
#include <mutex>
#include <turbo/toolset/extension.hpp>

namespace turbo {
namespace algorithm {

template <class value_t, class mutex_t>
void swap(value_t& left_value, mutex_t& left_mutex, value_t& right_value, mutex_t& right_mutex)
{
    if (TURBO_UNLIKELY(&left_value == &right_value))
    {
	return;
    }
    std::unique_lock<mutex_t> left_lock(left_mutex, std::defer_lock);
    std::unique_lock<mutex_t> right_lock(right_mutex, std::defer_lock);
    std::lock(left_lock, right_lock);
    std::swap(left_value, right_value);
}

} // namespace algorithm
} // namespace turbo

#endif
