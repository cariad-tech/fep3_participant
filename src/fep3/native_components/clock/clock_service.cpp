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

#include "clock_service.h"

#include "rpc_clock_service.h"
#include "rpc_clock_sync_service.h"
#include "variant_handling/clock_variant_handling.h"

#include <fep3/base/properties/property_type.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/fep3_errors.h>
#include <fep3/rpc_services/clock/clock_service_rpc_intf_def.h>

#include <a_util/result/result_type.h>
#include <a_util/strings.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>

#include <algorithm>

namespace fep3 {
namespace native {

ClockService::ClockService()
    : _is_started(false),
      _is_tensed(false),
      _realtime_clock(std::make_shared<SystemClock>()),
      _simulation_clock(std::make_shared<SimulationClock>()),
      _current_clock(GenericClockAdapter(_realtime_clock)),
      _clock_event_sink_registry(std::make_shared<ClockEventSinkRegistry>())
{
}

fep3::Result ClockService::create()
{
    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            "No IComponents set, can not get logging and configuration interface");
    }

    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration service is not registered");
    }

    FEP3_RETURN_IF_FAILED(_configuration.initConfiguration(*configuration_service));

    const auto service_bus = components->getComponent<IServiceBus>();
    if (!service_bus) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }

    _clock_registry = std::make_unique<ClockRegistry>();
    FEP3_RETURN_IF_FAILED(setupLogger(*components));

    FEP3_RETURN_IF_FAILED(setupClockMainEventSink(*service_bus));

    const auto rpc_server = service_bus->getServer();
    if (!rpc_server) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }

    FEP3_RETURN_IF_FAILED(setupRPCClockSyncService(*rpc_server));
    FEP3_RETURN_IF_FAILED(setupRPCClockService(*rpc_server));
    FEP3_RETURN_IF_FAILED(setupRPCLogger(*components));
    FEP3_RETURN_IF_FAILED(registerDefaultClocks());

    return {};
}

fep3::Result ClockService::destroy()
{
    deinitLogger();
    _clock_event_sink_registry->deinitLogger();
    _rpc_clock_sync_service->deinitLogger();
    _clock_registry->deinitLogger();

    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                 "No IComponents set, can not get configuration interface");
    }

    auto res = unregisterServices(*components);

    _configuration.deinitConfiguration();

    return res;
}

fep3::Result ClockService::initialize()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    // make sure the local clock service is in a defined state
    deinitialize();

    return {};
}

fep3::Result ClockService::tense()
{
    _configuration.updatePropertyVariables();

    {
        std::lock_guard<std::mutex> lock_guard(_select_main_clock_mutex);

        FEP3_RETURN_IF_FAILED(selectMainClock(_configuration._main_clock_name));
    }

    try {
        FEP3_RETURN_IF_FAILED(
            _clock_main_event_sink->updateTimeout(Duration(_configuration._time_update_timeout)));
    }
    catch (const std::exception& exception) {
        FEP3_LOG_ERROR(a_util::strings::format(
            "Exception during update of clock master timeout configuration: %s", exception.what()));
        RETURN_ERROR_DESCRIPTION(ERR_EMPTY, exception.what());
    }

    if (FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME ==
        static_cast<std::string>(_configuration._main_clock_name)) {
        FEP3_RETURN_IF_FAILED(_configuration.validateSimClockConfiguration(getLogger()));

        _simulation_clock->updateConfiguration(Duration(_configuration._clock_sim_time_step_size),
                                               _configuration._clock_sim_time_time_factor);
    }

    _is_tensed = true;
    return {};
}

fep3::Result ClockService::relax()
{
    _is_tensed = false;
    return {};
}

fep3::Result ClockService::start()
{
    performLockedOp([&](GenericClockAdapter& clock) {
        FEP3_LOG_DEBUG(a_util::strings::format("Clock '%s' is configured as main clock.",
                                               clock.getName().c_str()));
        clock.start(_clock_event_sink_registry);
    });
    _is_started = true;

    return {};
}

fep3::Result ClockService::stop()
{
    performLockedOp([&](GenericClockAdapter& clock) { clock.stop(); });
    _is_started = false;

    return {};
}

std::string ClockService::getMainClockName() const
{
    if (!_is_tensed) {
        _configuration.updatePropertyVariables();

        return _configuration._main_clock_name.toString();
    }

    return performLockedOp(
        [&](const GenericClockAdapter& clock) -> std::string { return clock.getName(); });
}

Timestamp ClockService::getTime() const
{
    if (!_is_started) {
        return Timestamp{0};
    }

    return performLockedOp(
        [&](const GenericClockAdapter& clock) -> Timestamp { return clock.getTime(); });
}

Optional<Timestamp> ClockService::getTime(const std::string& clock_name) const
{
    const auto clock_found =
        _clock_registry->findClock<fep3::arya::IClock>(std::string(clock_name));
    if (clock_found) {
        return {clock_found->getTime()};
    }
    else {
        FEP3_LOG_WARNING(a_util::strings::format(
            "Receiving clock time failed. A clock with the name %s is not registered.",
            clock_name.c_str()));

        return {};
    }
}

fep3::arya::IClock::ClockType ClockService::getType() const
{
    if (!_is_tensed) {
        _configuration.updatePropertyVariables();

        const auto clock_type = getType(_configuration._main_clock_name.toString());

        if (clock_type.has_value()) {
            return clock_type.value();
        }
    }

    return performLockedOp([&](const GenericClockAdapter& clock) -> fep3::arya::IClock::ClockType {
        return clock.getType();
    });
}

Optional<fep3::arya::IClock::ClockType> ClockService::getType(const std::string& clock_name) const
{
    const auto clock_found = _clock_registry->getClockAdapter(std::string(clock_name));

    if (clock_found) {
        return clock_found->getType();
    }
    else {
        FEP3_LOG_WARNING(a_util::strings::format(
            "Receiving clock type failed. A clock with the name %s is not registered.",
            clock_name.c_str()));

        return {};
    }
}

fep3::Result ClockService::selectMainClock(const std::string& clock_name)
{
    if (_is_started) {
        auto result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            a_util::strings::format("Setting main clock %s failed. Can not reset main clock after "
                                    "start of clock service.",
                                    clock_name.c_str())
                .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    const auto set_main_clock_locked = [&]() -> bool {
        auto current_clock = _clock_registry->getClockAdapter(clock_name);
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

        if (!current_clock) {
            current_clock = _clock_registry->getClockAdapter(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);

            _current_clock = current_clock.value();

            return false;
        }
        else {
            _current_clock = current_clock.value();
            return true;
        }
    };

    if (!set_main_clock_locked()) {
        auto result = CREATE_ERROR_DESCRIPTION(
            ERR_NOT_FOUND,
            a_util::strings::format("Setting main clock failed. A clock with the name %s is not "
                                    "registered. Resetting to default.",
                                    clock_name.c_str())
                .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    FEP3_RETURN_IF_FAILED(fep3::base::setPropertyValue(
        *_configuration.getNode()->getChild(FEP3_MAIN_CLOCK_PROPERTY), clock_name));

    FEP3_LOG_DEBUG(a_util::strings::format("Clock '%s' set as main clock of the clock service.",
                                           clock_name.c_str()));

    return {};
}

fep3::Result ClockService::registerClock(const std::shared_ptr<fep3::arya::IClock>& clock)
{
    return registerClockImpl(clock);
}

fep3::Result ClockService::registerClock(const std::shared_ptr<fep3::experimental::IClock>& clock)
{
    return registerClockImpl(clock);
}

template <typename T>
fep3::Result ClockService::registerClockImpl(std::shared_ptr<T> clock)
{
    if (_is_started) {
        auto result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            a_util::strings::format("Registering clock %s failed. Can not register clock after "
                                    "start of clock service.",
                                    clock->getName().c_str())
                .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    return _clock_registry->registerClock(clock);
}

fep3::Result ClockService::unregisterClock(const std::string& clock_name)
{
    if (_is_started) {
        auto result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            a_util::strings::format("Unregistering clock %s failed. Can not unregister clock after "
                                    "start of clock service.",
                                    clock_name.c_str())
                .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    FEP3_RETURN_IF_FAILED(_clock_registry->unregisterClock(clock_name));

    const auto is_main_clock_unregistered = [&]() -> std::pair<fep3::Result, bool> {
        std::lock_guard<std::mutex> lock_guard(_select_main_clock_mutex);

        if (getMainClockName() == clock_name) {
            const auto result = selectMainClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);

            return {result, true};
        }

        return {{}, false};
    };

    const auto result = is_main_clock_unregistered();

    FEP3_RETURN_IF_FAILED(result.first);

    if (result.second) {
        FEP3_RETURN_IF_FAILED(selectMainClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));

        FEP3_LOG_WARNING(a_util::strings::format(
            "Unregistered main clock %s. Reset main clock to default value %s.",
            clock_name.c_str(),
            FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));
    }

    return {};
}

std::list<std::string> ClockService::getClockNames() const
{
    return _clock_registry->getClockNames();
}

std::shared_ptr<fep3::arya::IClock> ClockService::findClock(const std::string& clock_name) const
{
    return _clock_registry->findClock<fep3::arya::IClock>(clock_name);
}

std::shared_ptr<experimental::IClock> ClockService::findClockCatelyn(
    const std::string& clock_name) const
{
    return _clock_registry->findClock<fep3::experimental::IClock>(clock_name);
}

fep3::Result ClockService::registerEventSink(
    const std::weak_ptr<fep3::arya::IClock::IEventSink>& clock_event_sink)
{
    return registerEventSinkImpl(clock_event_sink);
}

fep3::Result ClockService::unregisterEventSink(
    const std::weak_ptr<fep3::arya::IClock::IEventSink>& clock_event_sink)
{
    return unregisterEventSinkImpl(clock_event_sink);
}

fep3::Result ClockService::registerEventSink(
    const std::weak_ptr<fep3::experimental::IClock::IEventSink>& clock_event_sink)
{
    return registerEventSinkImpl(clock_event_sink);
}

fep3::Result ClockService::unregisterEventSink(
    const std::weak_ptr<fep3::experimental::IClock::IEventSink>& clock_event_sink)
{
    return unregisterEventSinkImpl(clock_event_sink);
}

template <typename T>
fep3::Result ClockService::registerEventSinkImpl(const std::weak_ptr<T>& clock_event_sink)
{
    if (clock_event_sink.expired()) {
        auto result = CREATE_ERROR_DESCRIPTION(
            ERR_POINTER,
            a_util::strings::format("Registering event sink failed. Event sink does not exist")
                .c_str());

        FEP3_LOG_RESULT(result);
        return result;
    }

    _clock_event_sink_registry->registerSink(clock_event_sink);

    return {};
}

template <typename T>
fep3::Result ClockService::unregisterEventSinkImpl(const std::weak_ptr<T>& clock_event_sink)
{
    if (clock_event_sink.expired()) {
        auto result = CREATE_ERROR_DESCRIPTION(
            ERR_POINTER,
            a_util::strings::format("Unregistering event sink failed. Event sink does not exist")
                .c_str());

        FEP3_LOG_RESULT(result);
        return result;
    }

    _clock_event_sink_registry->unregisterSink(clock_event_sink);

    return {};
}

fep3::Result ClockService::setupLogger(const IComponents& components)
{
    FEP3_RETURN_IF_FAILED(initLogger(components, "clock_service.component"));
    FEP3_RETURN_IF_FAILED(
        _clock_registry->initLogger(components, "clock_registry.clock_service.component"));
    FEP3_RETURN_IF_FAILED(_clock_event_sink_registry->initLogger(
        components, "event_sink_registry.clock_service.component"));

    return {};
}

fep3::Result ClockService::setupRPCLogger(const IComponents& components)
{
    FEP3_RETURN_IF_FAILED(
        _rpc_clock_sync_service->initLogger(components, "clock_master.clock_service.component"));

    return {};
}

fep3::Result ClockService::unregisterServices(const IComponents& components) const
{
    const auto service_bus = components.getComponent<IServiceBus>();
    if (!service_bus) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Service bus is not available");
    }
    auto rpc_server = service_bus->getServer();
    if (!rpc_server) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "RPC server is not available");
    }
    rpc_server->unregisterService(rpc::IRPCClockSyncMasterDef::getRPCDefaultName());
    rpc_server->unregisterService(rpc::IRPCClockServiceDef::getRPCDefaultName());

    return {};
}

fep3::Result ClockService::registerDefaultClocks()
{
    FEP3_RETURN_IF_FAILED(
        _clock_registry->registerNativeClocks({_realtime_clock, _simulation_clock}));

    return {};
}

fep3::Result ClockService::setupClockMainEventSink(const fep3::arya::IServiceBus& service_bus)
{
    const auto get_rpc_requester_by_name =
        [&service_bus](const std::string& service_participant_name) {
            return service_bus.getRequester(service_participant_name);
        };

    try {
        _clock_main_event_sink = std::make_shared<rpc::ClockMainEventSink>(
            getLogger(), Duration(_configuration._time_update_timeout), get_rpc_requester_by_name);
    }
    catch (const std::runtime_error& ex) {
        FEP3_LOG_ERROR(
            a_util::strings::format("Exception during setup of clock master: %s", ex.what()));

        RETURN_ERROR_DESCRIPTION(ERR_EMPTY, ex.what());
    }

    _clock_event_sink_registry->registerSink(
        std::weak_ptr<fep3::experimental::IClock::IEventSink>(_clock_main_event_sink));

    return {};
}

fep3::Result ClockService::setupRPCClockSyncService(IServiceBus::IParticipantServer& rpc_server)
{
    if (_rpc_clock_sync_service == nullptr) {
        _rpc_clock_sync_service =
            std::make_shared<RPCClockSyncService>(*_clock_main_event_sink, *this);
    }
    FEP3_RETURN_IF_FAILED(rpc_server.registerService(
        rpc::IRPCClockSyncMasterDef::getRPCDefaultName(), _rpc_clock_sync_service));

    return {};
}

fep3::Result ClockService::setupRPCClockService(IServiceBus::IParticipantServer& rpc_server)
{
    if (_rpc_clock_service == nullptr) {
        _rpc_clock_service = std::make_shared<RPCClockService>(*this);
    }

    FEP3_RETURN_IF_FAILED(rpc_server.registerService(rpc::IRPCClockServiceDef::getRPCDefaultName(),
                                                     _rpc_clock_service));

    return {};
}

} // namespace native
} // namespace fep3
