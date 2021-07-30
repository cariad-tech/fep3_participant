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


#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>
#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>
#include <fep3/components/base/component.h>
#include "simulation_bus/fep_connext_dds_simulation_bus.h"
#include <functional>


void fep3_plugin_getPluginVersion(void(*callback)(void*, const char*), void* destination)
{
    callback(destination, FEP3_PARTICIPANT_LIBRARY_VERSION_STR);
}

fep3::plugin::cpp::arya::ICPPPluginComponentFactory* fep3_plugin_cpp_arya_getFactory()
{
    return new fep3::plugin::cpp::arya::CPPPluginComponentFactory<ConnextDDSSimulationBus>();
}


