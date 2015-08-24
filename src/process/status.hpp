#ifndef TURBO_PROCESS_STATUS_HPP
#define TURBO_PROCESS_STATUS_HPP

#include <turbo/filesystem/path.hpp>
#include <turbo/toolset/attribute.hpp>

namespace turbo {
namespace process {
namespace status {

TURBO_SYMBOL_DECL turbo::filesystem::path current_exe_path();

} // namespace status
} // namespace process
} // namespace turbo

#endif
