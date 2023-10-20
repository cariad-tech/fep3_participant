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

#include "service_bus.h"

#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>
#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>

void fep3_plugin_getPluginVersion(void (*callback)(void*, const char*), void* destination)
{
    callback(destination, FEP3_PARTICIPANT_LIBRARY_VERSION_STR);
}

fep3::plugin::cpp::arya::ICPPPluginComponentFactory* fep3_plugin_cpp_arya_getFactory()
{
    static auto component_factory =
        std::make_shared<fep3::plugin::cpp::arya::ComponentFactory<fep3::native::ServiceBus>>();
    return new fep3::plugin::cpp::arya::ComponentFactoryWrapper(component_factory);
}

fep3::plugin::cpp::catelyn::IComponentFactory* fep3_plugin_cpp_catelyn_getFactory()
{
    return new fep3::plugin::cpp::catelyn::ComponentFactory<fep3::native::ServiceBus>();
}