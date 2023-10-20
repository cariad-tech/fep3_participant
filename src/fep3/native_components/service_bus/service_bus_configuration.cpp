/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */
#include "service_bus_configuration.h"

namespace fep3 {
namespace native {

ServiceBusConfiguration::ServiceBusConfiguration() : Configuration(FEP3_SERVICE_BUS_CONFIG)
{
}

fep3::Result ServiceBusConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_init_priority, FEP3_SERVICE_BUS_INIT_PRIORITY));
    FEP3_RETURN_IF_FAILED(
        registerPropertyVariable(_start_priority, FEP3_SERVICE_BUS_START_PRIORITY));

    return {};
}

fep3::Result ServiceBusConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(
        unregisterPropertyVariable(_init_priority, FEP3_SERVICE_BUS_INIT_PRIORITY));
    FEP3_RETURN_IF_FAILED(
        unregisterPropertyVariable(_start_priority, FEP3_SERVICE_BUS_START_PRIORITY));

    return {};
}

} // namespace native
} // namespace fep3
