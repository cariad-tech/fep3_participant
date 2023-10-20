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

#include <fep3/native_components/clock/clock_service.h>
#include <fep3/native_components/clock_sync/clock_sync_service.h>
#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/native_components/data_registry/data_registry.h>
#include <fep3/native_components/health/health_service.h>
#include <fep3/native_components/logging/logging_service.h>
#include <fep3/native_components/participant_info/participant_info.h>
#include <fep3/native_components/scheduler/local_scheduler_service.h>
#include <fep3/native_components/service_bus/include/service_bus.h>
#include <fep3/native_components/simulation_bus/simulation_bus.h>
#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>
#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>

void fep3_plugin_getPluginVersion(void (*callback)(void*, const char*), void* destination)
{
    callback(destination, FEP3_PARTICIPANT_LIBRARY_VERSION_STR);
}

template <template <typename...> typename component_factory_type>
using NativeComponentsFactory = component_factory_type<fep3::native::ClockService,
                                                       fep3::native::ClockSynchronizationService,
                                                       fep3::native::ConfigurationService,
                                                       fep3::native::DataRegistry,
                                                       fep3::native::HealthService,
                                                       fep3::native::JobRegistry,
                                                       fep3::native::LoggingService,
                                                       fep3::native::LocalSchedulerService,
                                                       fep3::native::ServiceBus,
                                                       fep3::native::SimulationBus,
                                                       fep3::native::ParticipantInfo>;

fep3::plugin::cpp::arya::ICPPPluginComponentFactory* fep3_plugin_cpp_arya_getFactory()
{
    static auto component_factory =
        std::make_shared<NativeComponentsFactory<fep3::plugin::cpp::arya::ComponentFactory>>();
    return new fep3::plugin::cpp::arya::ComponentFactoryWrapper(component_factory);
}

fep3::plugin::cpp::catelyn::IComponentFactory* fep3_plugin_cpp_catelyn_getFactory()
{
    return new NativeComponentsFactory<fep3::plugin::cpp::catelyn::ComponentFactory>();
}
