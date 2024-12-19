/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../../include/plugin/base/shared_library.h"

#include <fep3/fep3_filesystem.h>

#include <a_util/strings/strings_functions.h>

namespace fep3 {
namespace plugin {
namespace arya {

SharedLibrary::SharedLibrary(const std::string& file_path, bool prevent_unloading)
    : _file_path(file_path), _prevent_unloading(prevent_unloading)
{
    std::string trimmed_file_path = file_path;
    a_util::strings::trim(trimmed_file_path);
    fs::path full_file_path{trimmed_file_path};

    // rebuild plugin extension
    if (!fs::exists(full_file_path)) {
        // add prefix
#ifndef WIN32
        if (auto file_name = full_file_path.filename().string(); 0 != file_name.find("lib")) {
            file_name = "lib" + file_name;
            full_file_path.replace_filename(file_name);
        }
#endif
        auto file_extension = full_file_path.extension();
        // add extension
#ifdef WIN32
        if (file_extension != "dll") {
            full_file_path = fs::path{full_file_path.string() + ".dll"};
        }
#else
        if (file_extension != "so") {
            full_file_path = fs::path{full_file_path.string() + ".so"};
        }
#endif
    }

    const auto& full_file_path_string = full_file_path.string();
#ifdef WIN32
    // remember the cwd
    const auto& original_working_dir = fs::current_path();
    // on windows we need to switch to the directory where the library is located
    // to ensure loading of dependee dlls that reside in the same directory
    fs::current_path(full_file_path.parent_path());

    _library_handle = ::LoadLibrary(full_file_path_string.c_str());
    if (!_library_handle) {
        throw std::runtime_error("failed to load shared library '" + file_path +
                                 "' with error code '" + std::to_string(GetLastError()) + "'");
    }

    // switch back to the original cwd
    auto ec = std::error_code{};
    if (fs::current_path(original_working_dir, ec); ec) {
        throw std::runtime_error("unable to switch back to original working directory; error: '" +
                                 std::to_string(ec.value()) + " - " + ec.message() +
                                 "'; current working directory might be wrong from now on.");
    }
#else
    _library_handle = ::dlopen(full_file_path_string.c_str(), RTLD_LAZY);
    if (!_library_handle) {
        throw std::runtime_error("failed to load shared library '" + file_path + "' with error '" +
                                 dlerror() + "'");
    }
#endif
}

SharedLibrary::~SharedLibrary()
{
    if (!_prevent_unloading) {
        if (nullptr != _library_handle) {
#ifdef WIN32
            ::FreeLibrary(_library_handle);
#else
            ::dlclose(_library_handle);
#endif
        }
    }
}

SharedLibrary::SharedLibrary(SharedLibrary&& other)
    : _library_handle(std::move(other._library_handle)),
      _file_path(std::move(other._file_path)),
      _prevent_unloading(std::move(other._prevent_unloading))
{
    other._library_handle = nullptr;
}

std::string SharedLibrary::getFilePath() const
{
    return _file_path;
}

} // namespace arya
} // namespace plugin
} // namespace fep3
