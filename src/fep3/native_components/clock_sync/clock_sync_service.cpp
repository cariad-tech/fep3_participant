/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "clock_sync_service.h"

#include "master_on_demand_clock_client.h"

#include <fep3/native_components/clock/variant_handling/clock_service_handling.h>
#include <fep3/native_components/clock/variant_handling/clock_variant_handling.h>

namespace fep3 {
namespace native {

ClockSyncServiceConfiguration::ClockSyncServiceConfiguration()
    : Configuration(FEP3_CLOCKSYNC_SERVICE_CONFIG)
{
}

fep3::Result ClockSyncServiceConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(
        registerPropertyVariable(_timing_master_name, FEP3_TIMING_MASTER_PROPERTY));
    FEP3_RETURN_IF_FAILED(
        registerPropertyVariable(_slave_sync_cycle_time, FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY));

    return {};
}

fep3::Result ClockSyncServiceConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(
        unregisterPropertyVariable(_timing_master_name, FEP3_TIMING_MASTER_PROPERTY));
    FEP3_RETURN_IF_FAILED(
        unregisterPropertyVariable(_slave_sync_cycle_time, FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY));
    return {};
}

std::pair<bool, fep3::Result> ClockSyncServiceConfiguration::validateConfiguration(
    const std::string& main_clock_name, const std::shared_ptr<ILogger>& logger) const
{
    // clock synchronization requires one of the master on demand clocks to be configured
    // on the timing slave side
    if (main_clock_name == FEP3_CLOCK_SLAVE_MASTER_ONDEMAND ||
        main_clock_name == FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE) {
        if (static_cast<std::string>(_timing_master_name).empty()) {
            auto result = CREATE_ERROR_DESCRIPTION(
                ERR_INVALID_ARG,
                a_util::strings::format("No timing master configured. A timing master is necessary "
                                        "for the clock sync service to work correctly.")
                    .c_str());

            FEP3_ARYA_LOGGER_LOG_ERROR(
                logger,
                a_util::strings::format(
                    "Error during validation of clock sync service configuration: '%s'.",
                    result.getDescription()));

            return {true, result};
        }

        if (0 >= static_cast<int64_t>(_slave_sync_cycle_time)) {
            auto result = CREATE_ERROR_DESCRIPTION(
                ERR_INVALID_ARG,
                a_util::strings::format(
                    "Invalid slave sync cycle time of %lld. Slave sync cycle time has to be > 0.",
                    static_cast<int64_t>(_slave_sync_cycle_time))
                    .c_str());

            FEP3_ARYA_LOGGER_LOG_ERROR(
                logger,
                a_util::strings::format(
                    "Error during validation of clock sync service configuration: '%s'.",
                    result.getDescription()));

            return {true, result};
        }

        return {true, {}};
    }
    else {
        return {false, {}};
    }
}

fep3::Result ClockSynchronizationService::create()
{
    const std::shared_ptr<const IComponents> components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            "No IComponents set, can not get logging and configuration interface");
    }

    FEP3_RETURN_IF_FAILED(initLogger(*components, "clock_sync_service.component"));

    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration service is not registered");
    }

    FEP3_RETURN_IF_FAILED(_configuration.initConfiguration(*configuration_service));

    return {};
}

fep3::Result ClockSynchronizationService::destroy()
{
    deinitLogger();

    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            "No IComponents set, can not get logging and configuration interface");
    }

    _configuration.deinitConfiguration();

    return {};
}

fep3::Result ClockSynchronizationService::initialize()
{
    _configuration.updatePropertyVariables();

    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            "No IComponents set, can not get logging and configuration interface");
    }

    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration Service is not registered");
    }

    const auto [result, main_clock_service] = getClockServiceAdapter(*components, getLogger());
    FEP3_RETURN_IF_FAILED(result);
    const auto main_clock_name = main_clock_service->getMainClockName();
    const auto validation_result =
        _configuration.validateConfiguration(main_clock_name, getLogger());
    if (validation_result.first) {
        if (!validation_result.second) {
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
    if (!components) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Component pointer is invalid");
    }

    if (_slave_clock) {
        FEP3_RETURN_IF_FAILED(
            unregisterClockFromService(_slave_clock->get()->getName(), *components, getLogger()));
    }

    if (_rpc_clock_sync_slave) {
        const auto [result, rpc_server] = getRpcServer(*components);
        FEP3_RETURN_IF_FAILED(result);
        rpc_server->unregisterService(fep3::rpc::arya::IRPCClockSyncSlaveDef::getRPCDefaultName());
        _rpc_clock_sync_slave->get()->stopRPC();
    }
    // clock is stopped there will be no further rpc calls
    _slave_clock.reset();
    _rpc_clock_sync_slave.reset();

    return {};
}

fep3::Result ClockSynchronizationService::setupSlaveClock(const IComponents& components,
                                                          const std::string& main_clock_name)
{
    const auto service_bus = components.getComponent<IServiceBus>();
    if (!service_bus) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }
    const auto [result, rpc_server] = getRpcServer(components);
    FEP3_RETURN_IF_FAILED(result);

    const auto rpc_requester = service_bus->getRequester(_configuration._timing_master_name);
    if (!rpc_requester) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Requester not found");
    }

    if (FEP3_CLOCK_SLAVE_MASTER_ONDEMAND == main_clock_name) {
        auto clock = std::make_shared<MasterOnDemandClockInterpolating>(
            std::make_unique<fep3::InterpolationTime>(),
            [&]() { return _rpc_clock_sync_slave->get()->getTimeFromMaster(); },
            Duration{_configuration._slave_sync_cycle_time});

        auto far_clock_updater = std::make_shared<fep3::rpc::FarClockUpdater>(
            rpc_requester,
            rpc_server->getName(),
            [clock](fep3::rpc::IRPCClockSyncMasterDef::EventID event_id,
                    Timestamp new_time,
                    Timestamp old_time,
                    std::optional<Timestamp> next_tick) mutable {
                return clock->masterTimeEvent(event_id, new_time, old_time, next_tick);
            });

        FEP3_RETURN_IF_FAILED(
            clock->initLogger(components, "slave_master_on_demand.clock_sync_service.component"));
        _slave_clock = std::make_unique<SlaveClockAdapter<fep3::experimental::IClock>>(clock);
        _rpc_clock_sync_slave =
            std::make_unique<SlaveClockAdapter<fep3::rpc::arya::FarClockUpdater>>(
                far_clock_updater);
    }
    else if (FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE == main_clock_name) {
        auto clock = std::make_shared<MasterOnDemandClockDiscrete>();

        auto far_clock_updater = std::make_shared<fep3::rpc::FarClockUpdater>(
            rpc_requester,
            rpc_server->getName(),
            [clock](fep3::rpc::IRPCClockSyncMasterDef::EventID event_id,
                    Timestamp new_time,
                    Timestamp old_time,
                    std::optional<Timestamp> next_tick) mutable {
                return clock->masterTimeEvent(event_id, new_time, old_time, next_tick);
            });

        FEP3_RETURN_IF_FAILED(clock->initLogger(
            components, "slave_master_on_demand_discrete.clock_sync_service.component"));
        _slave_clock = std::make_unique<SlaveClockAdapter<fep3::experimental::IClock>>(clock);
        _rpc_clock_sync_slave =
            std::make_unique<SlaveClockAdapter<fep3::rpc::arya::FarClockUpdater>>(
                far_clock_updater);
    }
    if (_slave_clock && _rpc_clock_sync_slave) {
        FEP3_RETURN_IF_FAILED(registerClockToService(_slave_clock->get(), components, getLogger()));
        FEP3_RETURN_IF_FAILED(_rpc_clock_sync_slave->logger()->initLogger(
            components, "rpc_clock_sync_slave.clock_sync_service.component"));
        rpc_server->registerService(fep3::rpc::arya::IRPCClockSyncSlaveDef::getRPCDefaultName(),
                                    _rpc_clock_sync_slave->get());
        _rpc_clock_sync_slave->get()->startRPC();
    }

    FEP3_LOG_DEBUG(a_util::strings::format("Participant '%s' is configured as timing master.",
                                           _configuration._timing_master_name.toString().c_str()));

    return {};
}

std::pair<fep3::Result, std::shared_ptr<fep3::arya::IServiceBus::IParticipantServer>>
ClockSynchronizationService::getRpcServer(const IComponents& components) const
{
    const auto service_bus = components.getComponent<IServiceBus>();
    if (!service_bus) {
        return {CREATE_ERROR_DESCRIPTION(fep3::ERR_NO_INTERFACE,
                                         "%s is not part of the given component registry",
                                         fep3::IServiceBus::getComponentIID()),
                nullptr};
    }

    const auto rpc_server = service_bus->getServer();
    if (!rpc_server) {
        return {CREATE_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found"), nullptr};
    }
    else {
        return {{}, rpc_server};
    }
}

} // namespace native
} // namespace fep3