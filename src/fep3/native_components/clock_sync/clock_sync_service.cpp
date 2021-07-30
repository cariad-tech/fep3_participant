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


#include "clock_sync_service.h"

#include <cstdint>
#include <string>
#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>

#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/native_components/clock_sync/master_on_demand_clock_client.h>
#include "fep3/components/service_bus/service_bus_intf.h"

#include <a_util/strings.h>

namespace fep3
{
namespace native
{

ClockSyncServiceConfiguration::ClockSyncServiceConfiguration()
    : Configuration(FEP3_CLOCKSYNC_SERVICE_CONFIG)
{
}

fep3::Result ClockSyncServiceConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_timing_master_name, FEP3_TIMING_MASTER_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_slave_sync_cycle_time, FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY));

    return {};
}

fep3::Result ClockSyncServiceConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_timing_master_name, FEP3_TIMING_MASTER_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_slave_sync_cycle_time, FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY));
    return {};
}

std::pair<bool, fep3::Result>  ClockSyncServiceConfiguration::validateConfiguration(
    const std::string& main_clock_name,
    const std::shared_ptr<ILogger>& logger) const
{
    //clock synchronization requires one of the master on demand clocks to be configured
    //on the timing slave side
    if (main_clock_name == FEP3_CLOCK_SLAVE_MASTER_ONDEMAND
        || main_clock_name == FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE)
    {

        if (static_cast<std::string>(_timing_master_name).empty())
        {
            auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                a_util::strings::format(
                    "No timing master configured. A timing master is necessary for the clock sync service to work correctly.")
                .c_str());

            FEP3_ARYA_LOGGER_LOG_ERROR(logger, a_util::strings::format(
                "Error during validation of clock sync service configuration: '%s'.", result.getDescription()));

            return { true, result };
        }

        if (0 >= static_cast<int64_t>(_slave_sync_cycle_time))
        {
            auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                a_util::strings::format(
                    "Invalid slave sync cycle time of %lld. Slave sync cycle time has to be > 0.",
                    static_cast<int64_t>(_slave_sync_cycle_time))
                .c_str());

            FEP3_ARYA_LOGGER_LOG_ERROR(logger, a_util::strings::format(
                "Error during validation of clock sync service configuration: '%s'.", result.getDescription()));

            return { true, result };
        }

        return { true, {} };
    }
    else
    {
        return { false, {} };
    }
}

fep3::Result ClockSynchronizationService::create()
{
    const std::shared_ptr<const IComponents> components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get logging and configuration interface");
    }

    FEP3_RETURN_IF_FAILED(initLogger(*components, "clock_sync_service.component"));

    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration service is not registered");
    }

    FEP3_RETURN_IF_FAILED(_configuration.initConfiguration(*configuration_service));

    return {};
}

fep3::Result ClockSynchronizationService::destroy()
{
    deinitLogger();

    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get logging and configuration interface");
    }

    _configuration.deinitConfiguration();

    return {};
}

fep3::Result ClockSynchronizationService::initialize()
{
    _configuration.updatePropertyVariables();

    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get logging and configuration interface");
    }
    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration Service is not registered");
    }

    const auto main_clock_service = components->getComponent<IClockService>();
    if (!main_clock_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Clock Service is not registered");
    }
    const auto main_clock_name = main_clock_service->getMainClockName();
    const auto validation_result = _configuration.validateConfiguration(
        main_clock_name, getLogger());
    if (validation_result.first)
    {
        if (fep3::isFailed(validation_result.second))
        {
            return validation_result.second;
        }

        FEP3_RETURN_IF_FAILED(setupSlaveClock(*components, main_clock_name));
    }

    return {};
}

fep3::Result ClockSynchronizationService::tense()
{
    return {};
}

fep3::Result ClockSynchronizationService::deinitialize()
{
    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Component pointer is invalid");
    }
    auto clock_service = components->getComponent<IClockService>();
    if (!clock_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Clock Service is not registered");
    }
    if (_slave_clock.first)
    {
        if (_slave_clock.second)
        {
            _slave_clock.second->stopRPC();
        }
        clock_service->unregisterClock(_slave_clock.first->getName());
    }

    _slave_clock.first.reset();
    _slave_clock.second = nullptr;
    return {};
}

fep3::Result ClockSynchronizationService::start()
{
    if (_slave_clock.first)
    {
        if (_slave_clock.second)
        {
            _slave_clock.second->startWork();
        }
    }

    return {};
}
fep3::Result ClockSynchronizationService::stop()
{
    if (_slave_clock.first)
    {
        if (_slave_clock.second)
        {
            _slave_clock.second->stopWorkingIfStarted();
        }
    }

    return {};
}

fep3::Result ClockSynchronizationService::setupSlaveClock(
    const IComponents& components,
    const std::string& main_clock_name)
{
    auto clock_service = components.getComponent<IClockService>();
    if (!clock_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Clock Service is not registered");
    }
    const auto service_bus = components.getComponent<IServiceBus>();
    if (!service_bus)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }
    const auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }
    const auto rpc_requester = service_bus->getRequester(
        _configuration._timing_master_name);
    if (!rpc_requester)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Requester not found");
    }

    if (FEP3_CLOCK_SLAVE_MASTER_ONDEMAND == main_clock_name)
    {
        const auto clock_synchronizer = std::make_shared<rpc::arya::MasterOnDemandClockInterpolating>(
            Duration{ _configuration._slave_sync_cycle_time },
            rpc_server,
            rpc_requester,
            components,
            std::make_unique<fep3::InterpolationTime>(),
            rpc_server->getName());
        _slave_clock.first = clock_synchronizer;
        _slave_clock.second = clock_synchronizer.get();
    }
    else if (FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE == main_clock_name)
    {
        const auto clock_synchronizer = std::make_shared<rpc::arya::MasterOnDemandClockDiscrete>(
            Duration{ _configuration._slave_sync_cycle_time },
            rpc_server,
            rpc_requester,
            false,
            components,
            rpc_server->getName());
        _slave_clock.first = clock_synchronizer;
        _slave_clock.second = clock_synchronizer.get();
    }
    if (_slave_clock.first)
    {
        FEP3_RETURN_IF_FAILED(clock_service->registerClock(_slave_clock.first));
        if (_slave_clock.second)
        {
            _slave_clock.second->startRPC();
        }
    }

    FEP3_LOG_DEBUG(a_util::strings::format(
        "Participant '%s' is configured as timing master.", _configuration._timing_master_name.toString().c_str()));

    return {};
}

} // namespace native
} // namespace fep3