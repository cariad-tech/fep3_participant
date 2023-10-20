/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "../../../include/plugin/base/shared_library.h"

#include <a_util/filesystem.h>
#include <a_util/strings.h>

namespace fep3 {
namespace plugin {
namespace arya {

SharedLibrary::SharedLibrary(const std::string& file_path, bool prevent_unloading)
    : _file_path(file_path), _prevent_unloading(prevent_unloading)
{
    std::string trimmed_file_path = file_path;
    a_util::strings::trim(trimmed_file_path);
    a_util::filesystem::Path full_file_path{trimmed_file_path};

    // rebuild plugin extension
    if (!a_util::filesystem::exists(full_file_path)) {
        // add prefix
#ifndef WIN32
        auto file_name = full_file_path.getLastElement().toString();
        if (0 != file_name.find("lib")) {
            file_name = "lib" + file_name;
            full_file_path.removeLastElement();
            full_file_path.append(file_name);
        }
#endif
        auto file_extension = full_file_path.getExtension();
        // add extension
#ifdef WIN32
        if (file_extension != "dll") {
            full_file_path = a_util::filesystem::Path{full_file_path.toString() + ".dll"};
        }
#else
        if (file_extension != "so") {
            full_file_path = a_util::filesystem::Path{full_file_path.toString() + ".so"};
        }
#endif
    }

    const auto& full_file_path_string = full_file_path.toString();
#ifdef WIN32
    // remember the cwd
    const auto& original_working_dir = a_util::filesystem::getWorkingDirectory();
    // on windows we need to switch to the directory where the library is located
    // to ensure loading of dependee dlls that reside in the same directory
    a_util::filesystem::setWorkingDirectory(full_file_path.getParent());

    _library_handle = ::LoadLibrary(full_file_path_string.c_str());
    if (!_library_handle) {
        throw std::runtime_error("failed to load shared library '" + file_path +
                                 "' with error code '" + std::to_string(GetLastError()) + "'");
    }

    // switch back to the original cwd
    if (a_util::filesystem::Error::OK !=
        a_util::filesystem::setWorkingDirectory(original_working_dir)) {
        throw std::runtime_error("unable to switch back to original working directory; current "
                                 "working directory might be wrong from now on");
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
