/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#pragma once

#include <stdint.h>

#include <fep3/fep3_macros.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>
#include "c_plugin_intf.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Returns the version of the fep participant library, the plugin is compiled with, via callback
 * @param[in] callback The callback to be called with the version of the fep participant library
 * @param[in] destination The pointer to the destination the callback
 *                    target shall copy the participant library version string to
 */
FEP3_PLUGIN_EXPORT void FEP3_PLUGIN_CALL fep3_plugin_getParticipantLibraryVersion
    (void(*callback)(void*, fep3_plugin_base_ParticipantLibraryVersion), void* destination)
{
    callback(destination, fep3_plugin_base_ParticipantLibraryVersion
        {FEP3_PARTICIPANT_LIBRARY_VERSION_ID
        , FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR
        , FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR
        , FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH
        , FEP3_PARTICIPANT_LIBRARY_VERSION_BUILD + 0 // + 0 because macro value is not set in developer versions
        });
}

#ifdef __cplusplus
}
#endif