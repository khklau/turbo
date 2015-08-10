#ifndef TURBO_PROCESS_STATUS_HPP
#define TURBO_PROCESS_STATUS_HPP

#include <turbo/filesystem/path.hpp>

namespace turbo {
namespace process {
namespace status {

turbo::filesystem::path current_exe_path();

} // namespace status
} // namespace process
} // namespace turbo

#endif
