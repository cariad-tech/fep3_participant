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

#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/components/data_registry/mock_data_registry.h>
#include <fep3/components/simulation_bus/mock_simulation_bus.h>
#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>

using StubComponentsFactory = fep3::plugin::cpp::catelyn::ComponentFactory<
    ::testing::NiceMock<fep3::stub::arya::DataRegistry>,
    ::testing::NiceMock<fep3::stub::arya::SimulationBus>,
    ::testing::NiceMock<fep3::stub::experimental::ClockService>>;