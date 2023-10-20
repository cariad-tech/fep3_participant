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

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>

#include <stdint.h>

// symbols that must never change, because they are not bound to a namespace version
/// Defines the symbol name of the function that returns the participant library version (major,
/// minor, etc.)
#define SYMBOL_fep3_plugin_getParticipantLibraryVersion "fep3_plugin_getParticipantLibraryVersion"
/// Defines the symbol name of the function that returns the plugin version as string
#define SYMBOL_fep3_plugin_getPluginVersion "fep3_plugin_getPluginVersion"

#ifdef __cplusplus
extern "C" {
#endif

/// Structure for the participant library version
typedef struct {
    /// The version identifier of the participant library
    const char* _id;
    /// The major version integer
    int32_t _major;
    /// The minor version integer
    int32_t _minor;
    /// The patch version integer
    int32_t _patch;
    /// The build version integer
    int32_t _build;
} fep3_plugin_base_ParticipantLibraryVersion;

/**
 * Returns the version information of the plugin via callback
 * @note This function has to be implemented in the plugin
 * @param[in] callback The callback to be called with the plugin version string
 * @param[in] destination The pointer to the destination the callback
 *                    target shall copy the plugin version string to
 */
FEP3_PLUGIN_EXPORT void FEP3_PLUGIN_CALL fep3_plugin_getPluginVersion(void (*callback)(void*,
                                                                                       const char*),
                                                                      void* destination);

#ifdef __cplusplus
}
#endif