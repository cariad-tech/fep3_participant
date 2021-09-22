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


#include "test_plugin.h"

void fep3_plugin_getPluginVersion
    (void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "SimulationBus interface test c plugin 0.0.1");
}

::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>* g_mock_simulation_bus = nullptr;

void setMockSimulationBus(::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>* mock_simulation_bus)
{
    g_mock_simulation_bus = mock_simulation_bus;
}