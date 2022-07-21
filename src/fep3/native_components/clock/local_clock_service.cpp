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


#include "local_clock_service.h"

#include <algorithm>

#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>

#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/base/properties/property_type.h>
#include <fep3/fep3_errors.h>

#include "fep3/components/service_bus/rpc/fep_rpc_stubs_service.h"
#include "fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h"
#include "fep3/rpc_services/clock_sync/clock_sync_master_service_stub.h"
#include "local_system_clock.h"
#include "local_system_clock_discrete.h"

#include <a_util/strings.h>

namespace fep3
{
namespace native
{

class ClockEventSinkRegistry
    : public IClock::IEventSink
    , public base::EasyLogging
{
private:
    class EventSinks {
       using SinkVector = std::shared_ptr<std::vector<std::weak_ptr<IEventSink>>>;
   public:
        EventSinks()
            : _event_sinks(std::make_shared<std::vector<std::weak_ptr<IEventSink>>>())
        {
        }
    public:
       auto read() {
           std::lock_guard<std::mutex> lock(_mutex);

           return _event_sinks;
       }
       void write(const SinkVector& event_sinks) {
           std::lock_guard<std::mutex> lock(_mutex);

           _event_sinks = event_sinks;
       }

    private:
       std::mutex _mutex;
       SinkVector _event_sinks;
    };

public:
    ClockEventSinkRegistry() = default;

    ~ClockEventSinkRegistry() = default;

    void registerSink(const std::weak_ptr<IEventSink>& sink)
    {
        const auto sink_ptr = sink.lock();
        if (sink_ptr)
        {
            {
                std::lock_guard<std::mutex> lock_guard(_sinks_modification_mutex);

                std::shared_ptr<std::vector<std::weak_ptr<IEventSink>>> event_sinks = _event_sinks.read();

                for (auto& current_sink : *event_sinks)
                {
                    auto current_sink_ptr = current_sink.lock();
                    if (!current_sink_ptr || current_sink_ptr == sink_ptr)
                    {
                        return;
                    }
                }

                auto event_sinks_tmp = std::make_shared<std::vector<std::weak_ptr<IEventSink>>>();
                *event_sinks_tmp = *event_sinks;

                event_sinks_tmp->push_back(sink);

                _event_sinks.write(event_sinks_tmp);
            }

            FEP3_LOG_DEBUG("Registered event sink at the clock event sink registry.");
        }
        else
        {
            FEP3_LOG_WARNING("Registration of invalid event sink at the clock event sink registry failed.");
        }
    }

    void unregisterSink(const std::weak_ptr<IEventSink>& sink)
    {
        auto sink_ptr = sink.lock();
        if (sink_ptr)
        {

            const auto delete_event_sink_locked = [&]() -> bool
            {
                std::lock_guard<std::mutex> lock_guard(_sinks_modification_mutex);

                std::shared_ptr<std::vector<std::weak_ptr<IEventSink>>> event_sinks = _event_sinks.read();

                auto event_sinks_tmp = std::make_shared<std::vector<std::weak_ptr<IEventSink>>>();
                *event_sinks_tmp = *event_sinks;

                const auto it = std::remove_if(
                    event_sinks_tmp->begin(),
                    event_sinks_tmp->end(),
                    [&sink_ptr](const std::weak_ptr<IEventSink>& event_sink)
                        {
                            const auto event_sink_ptr = event_sink.lock();
                            if (event_sink_ptr)
                            {
                                return sink_ptr == event_sink_ptr;
                            }
                            return false;
                        });

                if (it != event_sinks_tmp->end())
                {
                    event_sinks_tmp->erase(it);

                    _event_sinks.write(event_sinks_tmp);

                    return true;
                }

                return false;
            };

            if (delete_event_sink_locked())
            {
                FEP3_LOG_DEBUG("Unregistered event sink from the clock event sink registry.");
            }
            else
            {
                FEP3_LOG_WARNING("Deregistration of event sink from the clock event sink registry failed. Event sink not found in the registry.");
            }
        }
        else
        {
            FEP3_LOG_WARNING("Deregistration of invalid event sink from the clock event sink registry failed.");
        }
    }

private:
    void timeUpdateBegin(Timestamp old_time, Timestamp new_time) override
    {
        FEP3_LOG_DEBUG(a_util::strings::format(
            "Distributing 'timeUpdateBegin' events. Old time '%lld', new time '%lld'.", old_time, new_time));

        auto event_sinks = _event_sinks.read();

        for (auto& event_sink : *event_sinks)
        {
            auto sink_ptr = event_sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeUpdateBegin(old_time, new_time);
            }
            else
            {
                FEP3_LOG_DEBUG("Expired event sink addressed during 'timeUpdateBegin' event. Unregistering it from Event sink registry.");

                unregisterSink(event_sink);
            }
        }
    }
    void timeUpdating(Timestamp new_time) override
    {
        FEP3_LOG_DEBUG(a_util::strings::format(
            "Distributing 'timeUpdating' events. New time '%lld'.", new_time));

        auto event_sinks = _event_sinks.read();

        for (auto& event_sink : *event_sinks)
        {
            auto sink_ptr = event_sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeUpdating(new_time);
            }
            else
            {
                FEP3_LOG_DEBUG("Expired event sink addressed during 'timeUpdating' event. Unregistering it from Event sink registry.");

                unregisterSink(event_sink);
            }
        }
    }
    void timeUpdateEnd(Timestamp new_time) override
    {
        FEP3_LOG_DEBUG(a_util::strings::format(
            "Distributing 'timeUpdateEnd' events. New time '%lld'.", new_time));

        auto event_sinks = _event_sinks.read();

        for (auto& event_sink : *event_sinks)
        {
            auto sink_ptr = event_sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeUpdateEnd(new_time);
            }
            else
            {
                FEP3_LOG_DEBUG("Expired event sink addressed during 'timeUpdateEnd' event. Unregistering it from Event sink registry.");

                unregisterSink(event_sink);
            }
        }
    }
    void timeResetBegin(Timestamp old_time, Timestamp new_time) override
    {

        FEP3_LOG_DEBUG(a_util::strings::format(
            "Distributing 'timeResetBegin' events. Old time '%lld', new time '%lld'.", old_time, new_time));

        auto event_sinks = _event_sinks.read();

        for (auto& event_sink : *event_sinks)
        {
            auto sink_ptr = event_sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeResetBegin(old_time, new_time);
            }
            else
            {
                FEP3_LOG_DEBUG("Expired event sink addressed during 'timeResetBegin' event. Unregistering it from Event sink registry.");

                unregisterSink(event_sink);
            }
        }
    }
    void timeResetEnd(Timestamp new_time) override
    {
        FEP3_LOG_DEBUG(a_util::strings::format(
            "Distributing 'timeResetEnd' events. New time '%lld'.", new_time));

        auto event_sinks = _event_sinks.read();

        for (auto& event_sink : *event_sinks)
        {
            auto sink_ptr = event_sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeResetEnd(new_time);
            }
            else
            {
                FEP3_LOG_DEBUG("Expired event sink addressed during 'timeResetEnd' event. Unregistering it from Event sink registry.");

                unregisterSink(event_sink);
            }
        }
    }

private:
    mutable std::mutex _sinks_modification_mutex;
    EventSinks _event_sinks;
};

class RPCClockSyncMaster
    : public rpc::RPCService<rpc_stubs::RPCClockSyncMasterServiceStub, rpc::arya::IRPCClockSyncMasterDef>
    , public base::EasyLogging
{
public:
    explicit RPCClockSyncMaster(LocalClockService& service)
    : _service(service)
    {
    }

protected:
    int registerSyncSlave(int event_id_flag, const std::string& slave_name) override
    {
        const auto result = _service.masterRegisterSlave(slave_name, event_id_flag);
        if (fep3::isOk(result))
        {
            FEP3_LOG_DEBUG(a_util::strings::format(
                    "Successfully registered timing slave '%s'.", slave_name.c_str()));

            return 0;
        }

        FEP3_LOG_WARNING(a_util::strings::format(
            "Failure during registration of timing slave '%s': '%d - %s'",
            slave_name.c_str(), result.getErrorCode(), result.getDescription()));

        return -1;
    }

    int unregisterSyncSlave(const std::string& slave_name) override
    {
        const auto result = _service.masterUnregisterSlave(slave_name);
        if (fep3::isOk(result))
        {
            FEP3_LOG_DEBUG(a_util::strings::format(
                "Successfully unregistered timing slave '%s'.", slave_name.c_str()));

            return 0;
        }

        FEP3_LOG_WARNING(a_util::strings::format(
            "Failure during deregistration of timing slave '%s': '%d - %s'",
            slave_name.c_str(), result.getErrorCode(), result.getDescription()));

        return -1;
    }

    int slaveSyncedEvent(const std::string& new_time, const std::string& slave_name) override
    {
        if (fep3::isOk(
            _service.masterSlaveSyncedEvent(slave_name, Timestamp{ a_util::strings::toInt64(new_time) })))
        {
            return 0;
        }
        return -1;
    }

    std::string getMasterTime() override
    {
        auto current_time = a_util::strings::toString(_service.getTime().count());

        FEP3_LOG_DEBUG(a_util::strings::format(
                "Retrieved master time request. Responding '%s'.", current_time.c_str()));

        return current_time;
    }

    int getMasterType() override
    {
        const auto main_clock_type = static_cast<int>(_service.getType());

        FEP3_LOG_DEBUG(a_util::strings::format(
                "Retrieved master clock type request. Responding '%d' (%s).",
            main_clock_type, 0 == main_clock_type ? "continuous" : "discrete"));

        return main_clock_type;
    }

private:
    LocalClockService& _service;
};
std::string RPCClockService::getClockNames()
{
    auto return_list = _service.getClockNames();
    auto first = true;
    std::string return_string;
    for (auto& clockname : return_list)
    {
        if (first)
        {
            return_string = clockname;
            first = false;
        }
        else
        {
            return_string += "," + clockname;
        }
    }
    return return_string;
}

std::string RPCClockService::getMainClockName()
{
    return _service.getMainClockName();
}

std::string RPCClockService::getTime(const std::string& clock_name)
{
    if (clock_name.empty())
    {
        return a_util::strings::toString(_service.getTime().count());
    }
    else
    {
        auto current_time = _service.getTime(clock_name);
        if (current_time.has_value())
        {
            return a_util::strings::toString(current_time.value().count());
        }
        else
        {
            return "-1";
        }
    }
}

int RPCClockService::getType(const std::string& clock_name)
{
    if (clock_name.empty())
    {
        return static_cast<int>(_service.getType());
    }
    else
    {
        auto clock_type = _service.getType(clock_name);
        if (clock_type.has_value())
        {
            return static_cast<int>(clock_type.value());
        }
        else
        {
            return -1;
        }
    }
}

ClockServiceConfiguration::ClockServiceConfiguration()
    : Configuration(FEP3_CLOCK_SERVICE_CONFIG)
{
}

fep3::Result ClockServiceConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_main_clock_name, FEP3_MAIN_CLOCK_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_time_update_timeout, FEP3_TIME_UPDATE_TIMEOUT_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_clock_sim_time_time_factor, FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_clock_sim_time_step_size, FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY));

    return {};
}

fep3::Result ClockServiceConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_main_clock_name, FEP3_MAIN_CLOCK_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_time_update_timeout, FEP3_TIME_UPDATE_TIMEOUT_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_clock_sim_time_time_factor, FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_clock_sim_time_step_size, FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY));

    return {};
}

fep3::Result ClockServiceConfiguration::validateSimClockConfiguration(
    const std::shared_ptr<ILogger>& logger) const
{
    FEP3_RETURN_IF_FAILED(validateSimClockConfigurationProperties(logger));

    if (FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE == _clock_sim_time_time_factor)
    {
        return {};
    }

    return validateWallClockStepSize(logger);
}

fep3::Result ClockServiceConfiguration::validateSimClockConfigurationProperties(
        const std::shared_ptr<ILogger>& logger) const
{
    constexpr int64_t sim_time_step_size_min_value = FEP3_CLOCK_SIM_TIME_STEP_SIZE_MIN_VALUE;
	constexpr int64_t sim_time_step_size_max_value = FEP3_CLOCK_SIM_TIME_STEP_SIZE_MAX_VALUE;
    constexpr double sim_time_step_size_afap_value = FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE;

    if (sim_time_step_size_min_value > _clock_sim_time_step_size
        || sim_time_step_size_max_value < _clock_sim_time_step_size)
    {
        const auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                R"(Invalid clock service configuration: Invalid clock step size of '%f' ns. Clock step size has to be >= %lld and <= %lld.)",
                static_cast<double>(_clock_sim_time_step_size),
                sim_time_step_size_min_value,
                sim_time_step_size_max_value)
            .c_str());

        FEP3_ARYA_LOGGER_LOG_RESULT(logger, result);

        return result;
    }
    else if (sim_time_step_size_afap_value > _clock_sim_time_time_factor)
    {
        const auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                R"(Invalid clock service configuration: Invalid clock time factor of '%f'. Clock time factor has to be >= %f.)",
                static_cast<double>(_clock_sim_time_time_factor),
                sim_time_step_size_afap_value)
            .c_str());

        FEP3_ARYA_LOGGER_LOG_RESULT(logger, result);

        return result;
    }

    return {};
}

fep3::Result ClockServiceConfiguration::validateWallClockStepSize(
        const std::shared_ptr<ILogger>& logger) const
{
    constexpr int64_t wall_clock_step_size_min_value = FEP3_CLOCK_WALL_CLOCK_STEP_SIZE_MIN_VALUE;
    constexpr int64_t wall_clock_step_size_max_value = FEP3_CLOCK_WALL_CLOCK_STEP_SIZE_MAX_VALUE;
    const double wall_clock_step_size = static_cast<double>(_clock_sim_time_step_size) / _clock_sim_time_time_factor;

    if (wall_clock_step_size < static_cast<double>(wall_clock_step_size_min_value)
        || wall_clock_step_size > static_cast<double>(wall_clock_step_size_max_value))
    {
        const auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                R"(Invalid clock service configuration: Invalid wall clock step size of '%f' ns resulting by dividing configured step size '%f' by time factor '%f'. Wall clock step size has to be >= %lld ns and <= %lld ns.)",
                wall_clock_step_size,
                static_cast<double>(_clock_sim_time_step_size),
                static_cast<double>(_clock_sim_time_time_factor),
                wall_clock_step_size_min_value,
                wall_clock_step_size_max_value)
            .c_str());

        FEP3_ARYA_LOGGER_LOG_RESULT(logger, result);

        return result;
    }

    return {};
}

LocalClockService::LocalClockService()
    : _is_started(false)
    , _is_tensed(false)
    , _local_system_real_clock(std::make_shared<LocalSystemRealClock>())
    , _local_system_sim_clock(std::make_shared<LocalSystemSimClock>())
    , _current_clock(_local_system_real_clock)
    , _clock_event_sink_registry(std::make_shared<ClockEventSinkRegistry>())
{
}

fep3::Result LocalClockService::create()
{
    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get logging and configuration interface");
    }

    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration service is not registered");
    }

    FEP3_RETURN_IF_FAILED(_configuration.initConfiguration(*configuration_service));

    const auto service_bus = components->getComponent<IServiceBus>();
    if (!service_bus)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }

    FEP3_RETURN_IF_FAILED(setupLogger(*components));

    FEP3_RETURN_IF_FAILED(setupClockMaster(*service_bus));

    const auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }

    FEP3_RETURN_IF_FAILED(setupRPCClockSyncMaster(*rpc_server));
    FEP3_RETURN_IF_FAILED(setupRPCClockService(*rpc_server));
    FEP3_RETURN_IF_FAILED(setupRPCLogger(*components));
    FEP3_RETURN_IF_FAILED(registerDefaultClocks());

    return {};
}

fep3::Result LocalClockService::destroy()
{
    deinitLogger();
    _clock_event_sink_registry->deinitLogger();
    _rpc_impl_master->deinitLogger();
    _clock_registry.deinitLogger();

    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get configuration interface");
    }

    auto res = unregisterServices(*components);

    _configuration.deinitConfiguration();

    return res;
}

fep3::Result LocalClockService::initialize()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    // make sure the local clock service is in a defined state
    deinitialize();

    return {};
}

fep3::Result LocalClockService::tense()
{
    _configuration.updatePropertyVariables();

    {
        std::lock_guard<std::mutex> lock_guard(_select_main_clock_mutex);

        FEP3_RETURN_IF_FAILED(selectMainClock(_configuration._main_clock_name));
    }

    try
    {
        FEP3_RETURN_IF_FAILED(_clock_master->updateTimeout(Duration(_configuration._time_update_timeout)));
    }
    catch(const std::exception& exception)
    {
        FEP3_LOG_ERROR(a_util::strings::format("Exception during update of clock master timeout configuration: %s", exception.what()));
        RETURN_ERROR_DESCRIPTION(ERR_EMPTY, exception.what());
    }

    if (FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME == static_cast<std::string>(_configuration._main_clock_name))
    {
        FEP3_RETURN_IF_FAILED(_configuration.validateSimClockConfiguration(getLogger()));

        _local_system_sim_clock->updateConfiguration(
            Duration(_configuration._clock_sim_time_step_size),
            _configuration._clock_sim_time_time_factor);
    }

    _is_tensed = true;
    return {};
}

fep3::Result LocalClockService::relax()
{
    _is_tensed = false;
    return {};
}

fep3::Result LocalClockService::start()
{
    const auto current_clock = getClockLocked();

    FEP3_LOG_DEBUG(a_util::strings::format(
        "Clock '%s' is configured as main clock.", current_clock->getName().c_str()));

    current_clock->start(_clock_event_sink_registry);

    _is_started = true;

    return {};
}

fep3::Result LocalClockService::stop()
{
    const auto current_clock = getClockLocked();

    current_clock->stop();
    _is_started = false;

    return {};
}

std::string LocalClockService::getMainClockName() const
{
    if (!_is_tensed)
    {
        _configuration.updatePropertyVariables();

        return _configuration._main_clock_name.toString();
    }

    const auto current_clock = getClockLocked();

    return current_clock->getName();
}

Timestamp LocalClockService::getTime() const
{
    if (!_is_started)
    {
        return Timestamp{0};
    }

    const auto current_clock = getClockLocked();

    return current_clock->getTime();
}

Optional<Timestamp> LocalClockService::getTime(const std::string& clock_name) const
{
    const auto clock_found = _clock_registry.findClock(std::string(clock_name));
    if (clock_found)
    {
        return { clock_found->getTime() };
    }
    else
    {
        FEP3_LOG_WARNING(a_util::strings::format(
            "Receiving clock time failed. A clock with the name %s is not registered.",
            clock_name.c_str()));

        return {};
    }
}

IClock::ClockType LocalClockService::getType() const
{
    if (!_is_tensed)
    {
        _configuration.updatePropertyVariables();

        const auto clock_type = getType(_configuration._main_clock_name.toString());

        if (clock_type.has_value())
        {
            return clock_type.value();
        }
    }

    const auto current_clock = getClockLocked();

    return current_clock->getType();
}

Optional<IClock::ClockType> LocalClockService::getType(const std::string& clock_name) const
{
    const auto clock_found = _clock_registry.findClock(std::string(clock_name));

    if (clock_found)
    {
        return clock_found->getType();
    }
    else
    {
        FEP3_LOG_WARNING(a_util::strings::format(
            "Receiving clock type failed. A clock with the name %s is not registered.",
            clock_name.c_str())
            );

        return {};
    }
}

fep3::Result LocalClockService::selectMainClock(const std::string& clock_name)
{
    if (_is_started)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
            a_util::strings::format(
                "Setting main clock %s failed. Can not reset main clock after start of clock service.",
                clock_name.c_str())
            .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    const auto set_main_clock_locked = [&]() -> bool
    {
        {
            auto current_clock = _clock_registry.findClock(clock_name);

            std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
            _current_clock = std::move(current_clock);
        }

        if (!_current_clock)
        {
            auto current_clock = _clock_registry.findClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);

            std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
            _current_clock = std::move(current_clock);

            return false;
        }

        return true;
    };

    if (!set_main_clock_locked())
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_NOT_FOUND,
            a_util::strings::format(
                "Setting main clock failed. A clock with the name %s is not registered. Resetting to default.",
                clock_name.c_str())
            .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    FEP3_RETURN_IF_FAILED(fep3::base::setPropertyValue(*_configuration.getNode()->getChild(FEP3_MAIN_CLOCK_PROPERTY), clock_name));

    FEP3_LOG_DEBUG(a_util::strings::format("Clock '%s' set as main clock of the clock service.", clock_name.c_str()));

    return {};
}

fep3::Result LocalClockService::registerClock(const std::shared_ptr<IClock>& clock)
{
    if (_is_started)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
            a_util::strings::format(
                "Registering clock %s failed. Can not register clock after start of clock service.",
                clock->getName().c_str())
            .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    return _clock_registry.registerClock(clock);
}

fep3::Result LocalClockService::unregisterClock(const std::string& clock_name)
{
    if (_is_started)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
            a_util::strings::format(
                "Unregistering clock %s failed. Can not unregister clock after start of clock service.",
                clock_name.c_str())
            .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    FEP3_RETURN_IF_FAILED(_clock_registry.unregisterClock(clock_name));

    const auto is_main_clock_unregistered = [&] () -> std::pair<fep3::Result, bool>
    {
        std::lock_guard<std::mutex> lock_guard(_select_main_clock_mutex);

        if (getMainClockName() == clock_name)
        {
            const auto result = selectMainClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);

            return {result, true};
        }

        return {{}, false};
    };

    const auto result = is_main_clock_unregistered();

    FEP3_RETURN_IF_FAILED(result.first);

    if (result.second)
    {
        FEP3_RETURN_IF_FAILED(selectMainClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));

        FEP3_LOG_WARNING(a_util::strings::format(
            "Unregistered main clock %s. Reset main clock to default value %s.",
            clock_name.c_str(),
            FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));
    }

    return {};
}

std::list<std::string> LocalClockService::getClockNames() const
{
    return _clock_registry.getClockNames();
}

std::shared_ptr<IClock> LocalClockService::findClock(const std::string& clock_name) const
{
    return _clock_registry.findClock(clock_name);
}

fep3::Result LocalClockService::registerEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink)
{
    if (clock_event_sink.expired())
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_POINTER,
            a_util::strings::format(
                "Registering event sink failed. Event sink does not exist")
            .c_str());

        FEP3_LOG_RESULT(result);
        return result;
    }

    _clock_event_sink_registry->registerSink(clock_event_sink);

    return {};
}

fep3::Result LocalClockService::unregisterEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink)
{
    if (clock_event_sink.expired())
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_POINTER,
            a_util::strings::format(
                "Unregistering event sink failed. Event sink does not exist")
            .c_str());

        FEP3_LOG_RESULT(result);
        return result;
    }

    _clock_event_sink_registry->unregisterSink(clock_event_sink);

    return {};
}

fep3::Result LocalClockService::setupLogger(const IComponents& components)
{
    FEP3_RETURN_IF_FAILED(initLogger(components, "clock_service.component"));
    FEP3_RETURN_IF_FAILED(_clock_registry.initLogger(components, "clock_registry.clock_service.component"));
    FEP3_RETURN_IF_FAILED(_clock_event_sink_registry->initLogger(components, "event_sink_registry.clock_service.component"));

    return {};
}

fep3::Result LocalClockService::setupRPCLogger(const IComponents& components)
{
    FEP3_RETURN_IF_FAILED(_rpc_impl_master->initLogger(components, "clock_master.clock_service.component"));

    return {};
}

fep3::Result LocalClockService::unregisterServices(const IComponents& components) const
{
    const auto service_bus = components.getComponent<IServiceBus>();
    if (!service_bus)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Service bus is not available");
    }
    auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "RPC server is not available");
    }
    rpc_server->unregisterService(rpc::IRPCClockSyncMasterDef::getRPCDefaultName());
    rpc_server->unregisterService(rpc::IRPCClockServiceDef::getRPCDefaultName());

    return {};
}

fep3::Result LocalClockService::registerDefaultClocks()
{
    FEP3_RETURN_IF_FAILED(_clock_registry.registerClock(_local_system_real_clock));
    FEP3_RETURN_IF_FAILED(_clock_registry.registerClock(_local_system_sim_clock));

    return{};
}

fep3::Result LocalClockService::setupClockMaster(const IServiceBus& service_bus)
{
    const auto get_rpc_requester_by_name = [&service_bus](const std::string& service_participant_name) {
        return service_bus.getRequester(service_participant_name);
    };

    try
    {
        _clock_master = std::make_shared<rpc::ClockMaster>(
            getLogger()
            , Duration(_configuration._time_update_timeout)
            , get_rpc_requester_by_name);
    }
    catch (const std::runtime_error& ex)
    {
        FEP3_LOG_ERROR(a_util::strings::format("Exception during setup of clock master: %s", ex.what()));

        RETURN_ERROR_DESCRIPTION(ERR_EMPTY, ex.what());
    }

    _clock_event_sink_registry->registerSink(_clock_master);

    return {};
}

fep3::Result LocalClockService::setupRPCClockSyncMaster(IServiceBus::IParticipantServer& rpc_server)
{
    if (_rpc_impl_master == nullptr)
    {
        _rpc_impl_master = std::make_shared<RPCClockSyncMaster>(*this);
    }
    FEP3_RETURN_IF_FAILED(rpc_server.registerService(rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
        _rpc_impl_master));

    return {};
}

fep3::Result LocalClockService::setupRPCClockService(IServiceBus::IParticipantServer& rpc_server)
{
    if (_rpc_impl_service == nullptr)
    {
        _rpc_impl_service = std::make_shared<RPCClockService>(*this);
    }

    FEP3_RETURN_IF_FAILED(rpc_server.registerService(rpc::IRPCClockServiceDef::getRPCDefaultName(),
        _rpc_impl_service));

    return {};
}

std::shared_ptr<IClock> LocalClockService::getClockLocked() const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    return _current_clock;
}

fep3::Result LocalClockService::masterRegisterSlave(const std::string& slave_name,
    const int event_id_flag) const
{
    return _clock_master->registerSlave(slave_name, event_id_flag);
}

fep3::Result LocalClockService::masterUnregisterSlave(const std::string& slave_name) const
{
    return _clock_master->unregisterSlave(slave_name);
}

fep3::Result LocalClockService::masterSlaveSyncedEvent(const std::string& slave_name,
    const Timestamp time) const
{
    return _clock_master->receiveSlaveSyncedEvent(slave_name, time);
}

} // namespace native
} // namespace fep3
