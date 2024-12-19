/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/service_bus/service_bus_intf.h>

namespace fep3::arya {
class ILogger;
} // namespace fep3::arya

namespace fep3 {
namespace native {

struct ServiceBusConfiguration : public base::Configuration {
    ServiceBusConfiguration();

    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;

public:
    base::PropertyVariable<int32_t> _start_priority{FEP3_SERVICE_BUS_START_PRIORITY_DEFAULT_VALUE};
    base::PropertyVariable<int32_t> _init_priority{FEP3_SERVICE_BUS_INIT_PRIORITY_DEFAULT_VALUE};
};

} // namespace native
} // namespace fep3
