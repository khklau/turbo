#ifndef TURBO_CONTAINER_INVALID_DEREFERENCE_ERROR_HPP
#define TURBO_CONTAINER_INVALID_DEREFERENCE_ERROR_HPP

#include <stdexcept>

namespace turbo {
namespace container {

class invalid_dereference_error : public std::out_of_range
{
public:
    explicit inline invalid_dereference_error(const std::string& what) : out_of_range(what) { }
    explicit inline invalid_dereference_error(const char* what) : out_of_range(what) { }
};

} // namespace container
} // namespace turbo

#endif
