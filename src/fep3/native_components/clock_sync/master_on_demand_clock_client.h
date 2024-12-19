/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "interpolation_time.h"
#include "notification_waiting.h"
#include "system_clock_client_reset.h"

#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/native_components/clock/clock_event_sink.h>
#include <fep3/rpc_services/clock_sync/clock_sync_master_client_stub.h>
#include <fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h>
#include <fep3/rpc_services/clock_sync/clock_sync_slave_service_stub.h>

#include <condition_variable>
#include <functional>
#include <optional>
#include <thread>

namespace fep3::rpc {
namespace arya {

class IRPCRequester;

class FarClockUpdater
    : public RPCService<fep3::rpc_stubs::RPCClockSyncSlaveServiceStub, IRPCClockSyncSlaveDef>,
      public base::EasyLogging {
public:
    using ClockServerEvent = std::function<Timestamp(IRPCClockSyncMasterDef::EventID,
                                                     Timestamp new_time,
                                                     Timestamp old_time,
                                                     std::optional<Timestamp> next_tick)>;
    explicit FarClockUpdater(
        const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
        const std::string& local_participant_name,
        ClockServerEvent clock_event_callback);

    ~FarClockUpdater();

    std::optional<fep3::Timestamp> getTimeFromMaster();
    void startRPC();
    void stopRPC();
    std::string syncTimeEvent(int event_id,
                              const std::string& new_time,
                              const std::string& next_tick,
                              const std::string& old_time) override;

private:
    bool isClientRegistered() const;

    void registerToMaster();
    void unregisterFromMaster();

private:
    RPCServiceClient<rpc_stubs::RPCClockSyncMasterClientStub, IRPCClockSyncMasterDef>
        _far_clock_master;
    int _master_type;

    std::string _local_participant_name;
    std::mutex _thread_mutex;
    bool _disconnected = false;
    ClockServerEvent _clock_event_callback;
};
} // namespace arya
using arya::FarClockUpdater;
} // namespace fep3::rpc

namespace fep3::native {

class MasterOnDemandClockInterpolating : public fep3::experimental::IClock,
                                         public base::EasyLogging {
public:
    explicit MasterOnDemandClockInterpolating(
        std::unique_ptr<fep3::IInterpolationTime> interpolation_time,
        std::function<std::optional<fep3::Timestamp>()> time_update,
        Duration on_demand_step_size);

    ~MasterOnDemandClockInterpolating();

    std::string getName() const override;

    fep3::arya::IClock::ClockType getType() const override;

    fep3::Timestamp getTime() const override;

    void reset(fep3::arya::Timestamp new_time) override;

    void start(const std::weak_ptr<IEventSink>& event_sink) override;

    void stop() override;

    Timestamp masterTimeEvent(fep3::rpc::IRPCClockSyncMasterDef::EventID event_id,
                              Timestamp new_time,
                              Timestamp old_time,
                              std::optional<Timestamp> next_tick);

private:
    void work();
    void resetInternal(fep3::Timestamp);

    mutable std::unique_ptr<fep3::IInterpolationTime> _current_interpolation_time;
    std::chrono::time_point<std::chrono::steady_clock> _next_request_gettime;
    std::function<std::optional<fep3::Timestamp>()> _time_update;
    const Duration _on_demand_step_size;
    std::thread _worker;

    std::atomic<bool> _stop = false;
    NotificationWaiting _stop_notification;
    ClockEventSink _event_sink_and_time;
    const fep3::Timestamp _initial_time = Timestamp{0};
    SystemClockClientReset clock_reset;
};

class MasterOnDemandClockDiscrete : public fep3::experimental::IClock, public base::EasyLogging {
public:
    explicit MasterOnDemandClockDiscrete();

    std::string getName() const override;

    fep3::arya::IClock::ClockType getType() const override;

    fep3::Timestamp getTime() const override;

    void reset(fep3::arya::Timestamp new_time) override;
    void start(const std::weak_ptr<IEventSink>& event_sink) override;
    void stop() override;

    Timestamp masterTimeEvent(fep3::rpc::IRPCClockSyncMasterDef::EventID event_id,
                              Timestamp new_time,
                              Timestamp old_time,
                              std::optional<Timestamp> next_tick);

private:
    void resetEvent(const Timestamp new_time);
    void timeUpdateEvent(const Timestamp new_time, const std::optional<Timestamp> next_tick);
    ClockEventSink _event_sink_and_time;
    std::atomic<bool> _started = false;
    std::atomic<bool> _reset = false;
    const fep3::Timestamp _initial_time = Timestamp{0};
};

} // namespace fep3::native
