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


#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <string>

#include <fep3/fep3_optional.h>
#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include "fep3/components/service_bus/rpc/fep_rpc_stubs_service.h"
#include "fep3/rpc_services/clock/clock_service_stub.h"
#include "fep3/rpc_services/clock/clock_service_rpc_intf_def.h"
#include "local_clock_registry.h"
#include "local_system_clock.h"
#include "local_system_clock_discrete.h"
#include <fep3/native_components/clock/local_clock_service_master.h>
#include "fep3/components/logging/easy_logger.h"

namespace fep3
{
namespace native
{

class ClockEventSinkRegistry;
class RPCClockSyncMaster;
class LocalClockService;

class RPCClockService : public rpc::RPCService<rpc_stubs::RPCClockServiceStub, rpc::IRPCClockServiceDef>
{
public:
    explicit RPCClockService(LocalClockService& service)
    : _service(service)
    {
    }

protected:
    std::string getClockNames() override;
    std::string getMainClockName() override;
    std::string getTime(const std::string& clock_name) override;
    int getType(const std::string& clock_name) override;

private:
    LocalClockService& _service;
};

/**
* @brief Configuration for the LocalClockService
*/
struct ClockServiceConfiguration : public base::Configuration
{
    ClockServiceConfiguration();
    ~ClockServiceConfiguration() = default;

    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;
    fep3::Result validateSimClockConfiguration(const std::shared_ptr<ILogger>& logger) const;

private:
    fep3::Result validateSimClockConfigurationProperties(const std::shared_ptr<ILogger>& logger) const;
    fep3::Result validateWallClockStepSize(const std::shared_ptr<ILogger>& logger) const;

public:
    base::PropertyVariable<std::string>       _main_clock_name{ FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME };
    base::PropertyVariable<int64_t>           _time_update_timeout{ FEP3_TIME_UPDATE_TIMEOUT_DEFAULT_VALUE };
    base::PropertyVariable<double>            _clock_sim_time_time_factor{ FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE };
    base::PropertyVariable<int64_t>           _clock_sim_time_step_size{ FEP3_CLOCK_SIM_TIME_STEP_SIZE_DEFAULT_VALUE };
};

/**
* @brief Native implementation of a clock service.
*/
class LocalClockService
    : public base::Component<IClockService>
    , public base::EasyLogging
{
public:
    LocalClockService();
    ~LocalClockService() = default;

public: // inherited via IClockService
    Timestamp getTime() const override;
    Optional<Timestamp> getTime(const std::string& clock_name) const override;
    IClock::ClockType getType() const override;
    Optional<IClock::ClockType> getType(const std::string& clock_name) const override;
    std::string getMainClockName() const override;

    fep3::Result registerEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink) override;
    fep3::Result unregisterEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink) override;

public: // inherited via IClockRegistry
    fep3::Result registerClock(const std::shared_ptr<IClock>& clock) override;
    fep3::Result unregisterClock(const std::string& clock_name) override;
    std::list<std::string> getClockNames() const override;
    std::shared_ptr<IClock> findClock(const std::string& clock_name) const override;

public: // inherited via base::Component
    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;
    fep3::Result tense() override;
    fep3::Result relax() override;
    fep3::Result start() override;
    fep3::Result stop() override;

public: // for Sync Master support
    fep3::Result masterRegisterSlave(const std::string& slave_name, int event_id_flag) const;
    fep3::Result masterUnregisterSlave(const std::string& slave_name) const;
    fep3::Result masterSlaveSyncedEvent(const std::string& slave_name, Timestamp time) const;

private:
    fep3::Result setupLogger(const IComponents& components);
    fep3::Result setupRPCLogger(const IComponents& components);
    fep3::Result unregisterServices(const IComponents& components) const;
    fep3::Result registerDefaultClocks();
    fep3::Result setupClockMaster(const IServiceBus& service_bus);
    fep3::Result setupRPCClockSyncMaster(IServiceBus::IParticipantServer& rpc_server);
    fep3::Result setupRPCClockService(IServiceBus::IParticipantServer& rpc_server);
    fep3::Result selectMainClock(const std::string& clock_name);
    std::shared_ptr<IClock> getClockLocked() const;

private:
    mutable std::recursive_mutex                                _recursive_mutex;
    std::mutex                                                  _select_main_clock_mutex;

    LocalClockRegistry                                          _clock_registry;
    ClockServiceConfiguration                                   _configuration;

    std::atomic<bool>                                           _is_started;
    std::atomic<bool>                                           _is_tensed;

    std::shared_ptr<LocalSystemRealClock>                       _local_system_real_clock;
    std::shared_ptr<LocalSystemSimClock>                        _local_system_sim_clock;
    std::shared_ptr<IClock>                                     _current_clock;

    std::shared_ptr<ClockEventSinkRegistry>                     _clock_event_sink_registry;

    std::shared_ptr<RPCClockSyncMaster>                         _rpc_impl_master{nullptr};
    std::shared_ptr<fep3::rpc::ClockMaster>                     _clock_master;
    std::function<fep3::Result()>                               _set_participant_to_error_state;
    std::shared_ptr<RPCClockService>                            _rpc_impl_service{ nullptr };
};

} // namespace native
} // namespace fep3
