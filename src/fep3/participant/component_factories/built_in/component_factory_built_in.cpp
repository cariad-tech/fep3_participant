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

#include "component_factory_built_in.h"
#include <fep3/components/logging/easy_logger.h>
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
namespace arya
{
    template<typename interface_type>
    void createAndRegisterComponent(ComponentRegistry& components, const ComponentFactoryBuiltIn& factory, const ILogger* logger)
    {
        components.registerComponent<interface_type>(factory.createComponent(getComponentIID<interface_type>(), logger));
    }

    ComponentFactoryBuiltIn::ComponentFactoryBuiltIn()
    {
    }

    ComponentFactoryBuiltIn::~ComponentFactoryBuiltIn()
    {
    }

    std::unique_ptr<fep3::arya::IComponent> ComponentFactoryBuiltIn::createComponent(const std::string& iid, const ILogger* logger) const
    {
        std::unique_ptr<fep3::arya::IComponent> component;

        // note: if/else-if/else in alphabetic order by component interface type
        if (iid == getComponentIID<IClockService>())
        {
            component.reset(new fep3::native::LocalClockService());
        }
        else if (iid == getComponentIID<IClockSyncService>())
        {
            component.reset(new fep3::native::ClockSynchronizationService());
        }
        else if (iid == getComponentIID<IConfigurationService>())
        {
            component.reset(new fep3::native::ConfigurationService());
        }
        else if (iid == getComponentIID<IDataRegistry>())
        {
            component.reset(new fep3::native::DataRegistry());
        }
        else if (iid == getComponentIID<fep3::experimental::IHealthService>())
        {
            component.reset(new fep3::native::HealthService());
        }
        else if (iid == getComponentIID<IJobRegistry>())
        {
            component.reset(new fep3::native::JobRegistry());
        }
        else if (iid == getComponentIID<ILoggingService>())
        {
            component.reset(new fep3::native::LoggingService());
        }
        else if (iid == getComponentIID<ISchedulerService>())
        {
            component.reset(new fep3::native::LocalSchedulerService());
        }
        else if (iid == getComponentIID<IServiceBus>())
        {
            component.reset(new fep3::native::ServiceBus());
        }
        else if (iid == getComponentIID<ISimulationBus>())
        {
            component.reset(new fep3::native::SimulationBus());
        }
        else if (iid == getComponentIID<IParticipantInfo>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::ParticipantInfo());
        }

        if(component)
        {
            FEP3_LOGGER_LOG_INFO
                (logger
                , std::string() + "Created built-in FEP Component for interface \"" + iid + "\""
                )
        }
        else
        {
            FEP3_LOGGER_LOG_ERROR
                (logger
                , std::string() + "Failed to create built-in FEP Component for interface \"" + iid + "\""
                )
        }

        return component;
    }

    void ComponentFactoryBuiltIn::createDefaults(ComponentRegistry& components, const ILogger* logger) const
    {
        createAndRegisterComponent<ILoggingService>(components, *this, logger);
        createAndRegisterComponent<IConfigurationService>(components, *this, logger);
        createAndRegisterComponent<IServiceBus>(components, *this, logger);
        createAndRegisterComponent<IClockService>(components, *this, logger);
        createAndRegisterComponent<IClockSyncService>(components, *this, logger);
        createAndRegisterComponent<IDataRegistry>(components, *this, logger);
        createAndRegisterComponent<IJobRegistry>(components, *this, logger);
        createAndRegisterComponent<ISchedulerService>(components, *this, logger);
        createAndRegisterComponent<ISimulationBus>(components, *this, logger);
        createAndRegisterComponent<IParticipantInfo>(components, *this, logger);
    }
}
}
