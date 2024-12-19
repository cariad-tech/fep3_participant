/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "faulty_test_plugin_intf.h"

#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>
#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>

class FaultyCreateMethodComponent : public fep3::base::Component<IFaultyCreateMethodComponent> {
public:
    FaultyCreateMethodComponent() = default;
    ~FaultyCreateMethodComponent() override = default;

    fep3::Result create() override
    {
        return CREATE_ERROR_DESCRIPTION(fep3::ERR_FAILED, "Creating the Component failed");
    }
};

void fep3_plugin_getPluginVersion(void (*callback)(void*, const char*), void* destination)
{
    callback(destination, "0.0.1");
}

fep3::plugin::cpp::catelyn::IComponentFactory* fep3_plugin_cpp_catelyn_getFactory()
{
    return new fep3::plugin::cpp::catelyn::ComponentFactory<FaultyCreateMethodComponent>;
}
