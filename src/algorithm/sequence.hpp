#ifndef TURBO_ALGORITHM_SEQUENCE_HPP
#define TURBO_ALGORITHM_SEQUENCE_HPP

namespace turbo {
namespace algorithm {

template <class value_t, class mutex_t>
inline void swap(value_t& left_value, mutex_t& left_mutex, value_t& right_value, mutex_t& right_mutex);

} // namespace algorithm
} // namespace turbo

#endif
