/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/fep3_participant_version.h>
#include <fep3/plugin/base/plugin_base_intf.h>
#include <fep3/plugin/cpp/cpp_plugin_component_factory_intf.h>

extern "C" {

#ifdef WIN32
FEP3_PLUGIN_EXPORT
bool FEP3_PLUGIN_CALL fep3_plugin_cpp_isDebugPlugin()
{
    #ifdef _DEBUG
    return true;
    #else
    return false;
    #endif
}
#endif

/**
 * Returns the version of the fep participant library, the plugin is compiled with.
 * @return The version of the fep participant library
 */
FEP3_PLUGIN_EXPORT fep3_plugin_base_ParticipantLibraryVersion FEP3_PLUGIN_CALL
fep3_plugin_getParticipantLibraryVersion()
{
    return fep3_plugin_base_ParticipantLibraryVersion{
        FEP3_PARTICIPANT_LIBRARY_VERSION_ID,
        FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH,
        FEP3_PARTICIPANT_LIBRARY_VERSION_BUILD +
            0 // + 0 because macro value is not set in developer versions
    };
}

/**
 * Returns the component factory of the plugin with type @ref
 * fep3::plugin::cpp::arya::ICPPPluginComponentFactory
 * @note This function has to be implemented in the plugin
 * @return Pointer to the plugin component factory
 * @remark The function transfers ownership of the returned component factory to the caller.
 */
FEP3_PLUGIN_EXPORT fep3::plugin::cpp::arya::ICPPPluginComponentFactory* FEP3_PLUGIN_CALL
fep3_plugin_cpp_arya_getFactory();

/**
 * Returns the component factory of the plugin with type @ref
 * fep3::plugin::cpp::catelyn::IComponentFactory
 * @note This function has to be implemented in the plugin
 * @return Pointer to the plugin component factory
 * @remark The function transfers ownership of the returned component factory to the caller.
 */
FEP3_PLUGIN_EXPORT fep3::plugin::cpp::catelyn::IComponentFactory* FEP3_PLUGIN_CALL
fep3_plugin_cpp_catelyn_getFactory();
}