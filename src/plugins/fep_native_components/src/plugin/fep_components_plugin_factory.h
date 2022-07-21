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

#include <fep3/native_components/clock/local_clock_service.h>
#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/native_components/data_registry/data_registry.h>
#include <fep3/native_components/health/health_service.h>
#include <fep3/native_components/scheduler/local_scheduler_service.h>
#include <fep3/native_components/service_bus/service_bus.h>
#include <fep3/native_components/simulation_bus/simulation_bus.h>
#include <fep3/native_components/clock_sync/clock_sync_service.h>
#include <fep3/native_components/job_registry/local_job_registry.h>
#include <fep3/native_components/logging/logging_service.h>
#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/native_components/participant_info/participant_info.h>

namespace fep3
{
namespace native
{
    template <typename T>
    std::unique_ptr<fep3::arya::IComponent> create_comp()
    {
        return std::make_unique<T>();
    }

    template <typename T>
    std::pair<std::string, std::function<std::unique_ptr<fep3::arya::IComponent>(void)>> create_comp_factory_pair()
    {
        return std::make_pair(T::getComponentIID(), create_comp<T>);
    }

    class CPPPluginMultiComponentFactory : public fep3::plugin::cpp::arya::ICPPPluginComponentFactory
    {
    public:

        std::unique_ptr<fep3::arya::IComponent> createComponent(const std::string& component_iid) const
        {
            auto it = m_callbacks.find(component_iid);
            if (it != m_callbacks.end())
            {
                return it->second();
            }
            else
            {
                return {};
            }
        }
    private:

        const std::map <std::string, std::function<std::unique_ptr<fep3::arya::IComponent>(void)>> m_callbacks =
        {
            {create_comp_factory_pair<fep3::native::LocalClockService>()},
            {create_comp_factory_pair<fep3::native::ClockSynchronizationService>()},
            {create_comp_factory_pair<fep3::native::ConfigurationService>()},
            {create_comp_factory_pair<fep3::native::DataRegistry>()},
            {create_comp_factory_pair<fep3::native::HealthService>()},
            {create_comp_factory_pair<fep3::native::JobRegistry>()},
            {create_comp_factory_pair<fep3::native::LoggingService>()},
            {create_comp_factory_pair<fep3::native::LocalSchedulerService>()},
            {create_comp_factory_pair<fep3::native::ServiceBus>()},
            {create_comp_factory_pair<fep3::native::SimulationBus>()},
            {create_comp_factory_pair<fep3::native::ParticipantInfo>()}
        };
    };
} // namespace native
} // namespace fep3
