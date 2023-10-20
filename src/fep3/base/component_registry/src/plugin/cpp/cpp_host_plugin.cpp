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

#include "../../../include/plugin/cpp/cpp_host_plugin.h"

#ifdef WIN32
    #include <Windows.h>
#else // WIN32
    #include <dlfcn.h>
    #include <string.h>
#endif // !WIN32

namespace fep3 {
namespace plugin {
namespace cpp {

HostPlugin::HostPlugin(const std::string& file_path)
    : HostPluginBase(file_path,
                     // right now we must prevent unloading because shared binary management is not
                     // implemented in CPP Plugin System
                     true)
{
    // on windows debug and non-debug builds are considered incompatible for C++ interfaces
#ifdef WIN32
    auto is_debug_plugin_function = get<bool()>(SYMBOL_fep3_plugin_cpp_isDebugPlugin);
    if (!is_debug_plugin_function) {
        throw std::runtime_error("the plugin '" + file_path + "' does not provide the required '" +
                                 SYMBOL_fep3_plugin_cpp_isDebugPlugin + "' function");
    }

    #ifdef _DEBUG
    if (!is_debug_plugin_function()) {
        throw std::runtime_error("the plugin '" + file_path +
                                 "' is not compiled in debug mode which this executable is");
    }
    #else
    if (is_debug_plugin_function()) {
        throw std::runtime_error("the plugin '" + file_path +
                                 "' is compiled in debug mode which this executable is not");
    }
    #endif
#endif

    auto get_participant_library_version_function =
        get<fep3_plugin_base_ParticipantLibraryVersion()>(
            SYMBOL_fep3_plugin_getParticipantLibraryVersion);
    if (!get_participant_library_version_function) {
        throw std::runtime_error("The plugin '" + file_path +
                                 "' does not provide an appropriate '" +
                                 SYMBOL_fep3_plugin_getParticipantLibraryVersion + "' function.");
    }
    _participant_library_version = get_participant_library_version_function();
}

HostPlugin::~HostPlugin()
{
}

} // namespace cpp
} // namespace plugin
} // namespace fep3
