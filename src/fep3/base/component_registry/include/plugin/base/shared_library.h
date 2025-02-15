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

#pragma once

#ifdef WIN32
    #include <Windows.h>
#else
    #include <dlfcn.h>
#endif

#include <string>

namespace fep3 {
namespace plugin {
namespace arya {

/**
 * A shared library class capable of loading functions from within a shared library.
 * Note: This class is compatible to boost::dll::shared_library.
 */
class SharedLibrary {
public:
    /**
     * @brief Deleted default CTOR
     */
    SharedLibrary() = delete;
    /**
     * @brief CTOR Loads the library identified by the passed @p file_path
     *
     * @param[in] file_path             The file path of the shared library to be loaded;
     *                              prefix and extension are automatically added if not present, i.
     * e.:
     *                              * on non-windows /lib/my_stuff results in loading of
     * /lib/libmy_stuff.so
     *                              * on windows C:\lib\my_stuff results in loading of
     * C:\lib\my_stuff.dll
     * @param[in] prevent_unloading     If false the shared library will be unloaded when this is
     * destroyed , if true the shared library will not be unloaded when this is destroyed
     * @throw Throws an exception of type std::runtime_error if opening the shared library at @p
     * file_path fails
     */
    SharedLibrary(const std::string& file_path, bool prevent_unloading = false);
    /**
     * @brief Deleted copy CTOR
     */
    SharedLibrary(const SharedLibrary&) = delete;
    /**
     * @brief Deleted copy assignment
     */
    SharedLibrary& operator=(const SharedLibrary&) = delete;
    /**
     * @brief move CTOR
     */
    SharedLibrary(SharedLibrary&&);
    /**
     * @brief Deleted move assignment
     */
    SharedLibrary& operator=(SharedLibrary&&) = delete;
    /**
     * DTOR
     */
    virtual ~SharedLibrary();

    /**
     * Gets a pointer to the function with \p symbol_name in the shared library
     *
     * @tparam t signature of the function
     * @param[in] symbol_name Name of the symbol in the shared library
     * @return Pointer to the function if the symbol with @p symbol_name was found, nullptr
     * otherwise
     */
    template <typename t>
    t* get(const std::string& symbol_name) const
    {
#ifdef WIN32
        return reinterpret_cast<t*>(::GetProcAddress(_library_handle, symbol_name.c_str()));
#else
        return reinterpret_cast<t*>(::dlsym(_library_handle, symbol_name.c_str()));
#endif
    }

    /**
     * @brief Gets the file path, the library was loaded from
     * @return The file path, the library was loaded from
     */
    std::string getFilePath() const;

private:
#ifdef WIN32
    HMODULE _library_handle;
#else
    void* _library_handle;
#endif
    std::string _file_path;
    bool _prevent_unloading{false};
};

} // namespace arya
using arya::SharedLibrary;
} // namespace plugin
} // namespace fep3
