/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "a_util/strings.h"

#include <fep3/components/clock_sync/clock_sync_service_intf.h>
#include <fep3/native_components/clock_sync/master_on_demand_clock_client.h>
using namespace std::chrono;

namespace fep3::rpc::arya {

int getEventIDFlags()
{
    return static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating) |
           static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset);
}

FarClockUpdater::FarClockUpdater(
    const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
    const std::string& local_participant_name,
    ClockServerEvent clock_event_callback)
    : _far_clock_master(IRPCClockSyncMasterDef::getRPCDefaultName(), participant_requester),
      _master_type(-1),
      _local_participant_name(local_participant_name),
      _clock_event_callback(clock_event_callback)

{
}

FarClockUpdater::~FarClockUpdater()
{
    if (!_disconnected) {
        stopRPC();
    }
}

void FarClockUpdater::startRPC()
{
    std::lock_guard<std::mutex> guard(_thread_mutex);
    registerToMaster();
    _disconnected = false;
}

void FarClockUpdater::stopRPC()
{
    std::lock_guard<std::mutex> guard(_thread_mutex);
    _disconnected = true;
    unregisterFromMaster();
}

void FarClockUpdater::registerToMaster()
{
    FEP3_LOG_DEBUG("Requesting timing master main clock type.");

    try {
        _master_type = _far_clock_master.getMasterType();

        FEP3_LOG_DEBUG(
            a_util::strings::format("Successfully retrieved timing master clock type '%d' (%s).",
                                    _master_type,
                                    0 == _master_type ? "continuous" : "discrete"));
    }
    catch (const std::exception& exception) {
        FEP3_LOG_WARNING(

            a_util::strings::format(
                "Retrieving timing master clock type failed during timing slave registration: '%s'",
                exception.what()));
    }

    FEP3_LOG_DEBUG("Requesting registration as timing slave at the timing master.");

    try {
        _far_clock_master.registerSyncSlave(getEventIDFlags(), _local_participant_name);

        FEP3_LOG_DEBUG("Successfully registered as timing slave at the timing master.");
    }
    catch (const std::exception& exception) {
        _master_type = -1;

        FEP3_LOG_WARNING(a_util::strings::format(
            "Failure during registration as timing slave at the timing master: '%s'",
            exception.what()));
    }
}

void FarClockUpdater::unregisterFromMaster()
{
    FEP3_LOG_DEBUG("Requesting deregistration as timing slave from the timing master.");

    try {
        _far_clock_master.unregisterSyncSlave(_local_participant_name);

        FEP3_LOG_DEBUG("Successfully deregistered as timing slave from the timing master.");
    }
    catch (const std::exception& exception) {
        FEP3_LOG_WARNING(a_util::strings::format(
            "Failure during deregistration as timing slave from the timing master: '%s'",
            exception.what()));
    }
}

std::string FarClockUpdater::syncTimeEvent(int event_id,
                                           const std::string& new_time,
                                           const std::string& next_tick,
                                           const std::string& old_time)
{
    std::string next_tick_info;
    // so that the lambda is called only if debug is enabled
    auto next_tick_info_message = [&]() {
        if (!next_tick.empty()) {
            next_tick_info = a_util::strings::format(", next tick '%s' ", next_tick.c_str());
        }
        return next_tick_info.c_str();
    };

    if (static_cast<uint8_t>(IRPCClockSyncMasterDef::EventID::time_update_before) == event_id) {
        FEP3_LOG_DEBUG(

            a_util::strings::format(
                "Received master time event. Event id '%d', old time '%s', new time '%s' %s.",
                event_id,
                old_time.c_str(),
                new_time.c_str(),
                next_tick_info_message()));
    }
    else {
        FEP3_LOG_DEBUG(
            a_util::strings::format("Received master time event. Event id '%d', new time '%s' %s.",
                                    event_id,
                                    new_time.c_str(),
                                    next_tick_info_message()));
    }

    std::optional<Timestamp> next_tick_time;
    if (!next_tick.empty()) {
        next_tick_time = Timestamp(a_util::strings::toInt64(next_tick));
    }
    const auto time =
        _clock_event_callback(static_cast<rpc::IRPCClockSyncMasterDef::EventID>(event_id),
                              Timestamp{a_util::strings::toInt64(new_time)},
                              Timestamp{a_util::strings::toInt64(old_time)},
                              next_tick_time);

    return std::to_string(time.count());
}

bool FarClockUpdater::isClientRegistered() const
{
    return _master_type != -1;
}

std::optional<fep3::Timestamp> FarClockUpdater::getTimeFromMaster()
{
    using namespace std::chrono;
    std::lock_guard<std::mutex> guard(_thread_mutex);
    // the rpc is stopped in deinitialize whereas the clock is stopped
    // in stop(), so this should not happen by design.
    if (_disconnected) {
        FEP3_LOG_INFO("Requested master time but the client RPC Clock Sync Service is "
                      "deregistered from timing master, master time cannot be requested")
        return {};
    }
    try {
        if (!isClientRegistered()) {
            registerToMaster();
        }
        FEP3_LOG_DEBUG("Requesting master time to synchronize local time with timing master.");
        std::string master_time = _far_clock_master.getMasterTime();
        FEP3_LOG_DEBUG(
            a_util::strings::format("Retrieved master time: '%s'.", master_time.c_str()));

        return Timestamp{a_util::strings::toInt64(master_time)};
    }
    catch (const std::exception& exception) {
        FEP3_LOG_WARNING(a_util::strings::format(
            "Failure during synchronization with timing master: '%s'", exception.what()));

        registerToMaster();
    }

    return {};
}
} // namespace fep3::rpc::arya

namespace fep3::native {
MasterOnDemandClockInterpolating::MasterOnDemandClockInterpolating(
    std::unique_ptr<fep3::IInterpolationTime> interpolation_time,
    std::function<std::optional<fep3::Timestamp>()> time_update,
    Duration on_demand_step_size)
    : _current_interpolation_time(std::move(interpolation_time)),
      _time_update(std::move(time_update)),
      _on_demand_step_size(on_demand_step_size),
      _stop_notification(true)
{
}

std::string MasterOnDemandClockInterpolating::getName() const
{
    return FEP3_CLOCK_SLAVE_MASTER_ONDEMAND;
}

fep3::arya::IClock::ClockType MasterOnDemandClockInterpolating::getType() const
{
    return fep3::arya::IClock::ClockType::continuous;
}

arya::Timestamp MasterOnDemandClockInterpolating::getTime() const
{
    return _current_interpolation_time->getTime();
}

Timestamp MasterOnDemandClockInterpolating::masterTimeEvent(
    const fep3::rpc::IRPCClockSyncMasterDef::EventID event_id,
    Timestamp new_time,
    Timestamp /**old_time*/,
    std::optional<Timestamp> /*next_tick*/)
{
    if (event_id == fep3::rpc::IRPCClockSyncMasterDef::EventID::time_reset) {
        {
            clock_reset.reset(new_time, [&](Timestamp reset_time) { resetInternal(reset_time); });
        }
    }

    return getTime();
}

MasterOnDemandClockInterpolating::~MasterOnDemandClockInterpolating()
{
    stop();
}

void MasterOnDemandClockInterpolating::reset(fep3::arya::Timestamp)
{
    FEP3_LOG_WARNING("Reseting a client system clock is not possible, clock can "
                     "only be reset from timing master");
}

void MasterOnDemandClockInterpolating::start(const std::weak_ptr<IEventSink>& event_sink)
{
    _event_sink_and_time.setEventSink(event_sink);
    _stop = false;
    clock_reset.start([&](Timestamp reset_time) { resetInternal(reset_time); }, getLogger().get());
    _worker = std::thread([&]() { work(); });
}

void MasterOnDemandClockInterpolating::stop()
{
    _stop = true;
    _stop_notification.notify();
    if (_worker.joinable())
        _worker.join();
    clock_reset.stop();
    _event_sink_and_time.setEventSink(std::weak_ptr<fep3::experimental::IClock::IEventSink>());
}

void MasterOnDemandClockInterpolating::work()
{
    using namespace std::chrono;
    std::chrono::time_point<std::chrono::steady_clock> next_request_gettime =
        time_point<steady_clock>{_initial_time};

    while (!_stop) {
        time_point<steady_clock> begin_request = steady_clock::now();
        auto current_time = _time_update();
        auto roundtrip_time = steady_clock::now() - begin_request;
        if (current_time) {
            _current_interpolation_time->setTime(current_time.value(), roundtrip_time);
        }

        next_request_gettime = steady_clock::now() + _on_demand_step_size;
        _stop_notification.waitForNotificationWithTimeout(_on_demand_step_size);
    }
}

void MasterOnDemandClockInterpolating::resetInternal(fep3::Timestamp new_time)
{
    _current_interpolation_time->resetTime(_initial_time);

    const auto old_time = _current_interpolation_time->getTime();
    auto _event_sink_pointer = _event_sink_and_time.getEventSink();

    if (_event_sink_pointer) {
        _event_sink_pointer->timeResetBegin(old_time, new_time);
        _event_sink_pointer->timeResetEnd(new_time);
    }
    else {
        // in this case the scheduler may not work
        // but clock_reset should never call reset with a null
        //_event_sink_pointer
        FEP3_LOG_WARNING(std::string(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND) +
                         " clock received reset event before the clock start,"
                         "set the start priority of the timing_master lower as the client clocks, "
                         "timing master should be started last,"
                         "ignoring this warning could lead to errors in clock and scheduling");
    }
}

MasterOnDemandClockDiscrete::MasterOnDemandClockDiscrete()
{
    _event_sink_and_time.setCurrentTime(_initial_time);
}

std::string MasterOnDemandClockDiscrete::getName() const
{
    return FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE;
}

fep3::arya::IClock::ClockType MasterOnDemandClockDiscrete::getType() const
{
    return fep3::arya::IClock::ClockType::discrete;
}

arya::Timestamp MasterOnDemandClockDiscrete::getTime() const
{
    return _event_sink_and_time.getCurrentTime();
}

void MasterOnDemandClockDiscrete::reset(fep3::arya::Timestamp)
{
    FEP3_LOG_WARNING("Reseting a client simulation clock is not possible, clock can "
                     "only be reset from timing master");
}

void MasterOnDemandClockDiscrete::start(const std::weak_ptr<IEventSink>& event_sink)
{
    _event_sink_and_time.setEventSink(event_sink);
    _started = true;
}

void MasterOnDemandClockDiscrete::stop()
{
    _started = false;
    _event_sink_and_time.setEventSink(std::weak_ptr<fep3::experimental::IClock::IEventSink>());
    _reset = false;
}

Timestamp MasterOnDemandClockDiscrete::masterTimeEvent(
    const fep3::rpc::IRPCClockSyncMasterDef::EventID event_id,
    const Timestamp new_time,
    const Timestamp old_time,
    std::optional<Timestamp> next_tick)
{
    if (!_started) {
        std::string time_update_msg;
        // avoids creating the string if debug is not enabled
        auto event_log = [&]() {
            time_update_msg = a_util::strings::format(
                "Event id '%d', %snew time '%lld'.",
                event_id,
                (fep3::rpc::IRPCClockSyncMasterDef::EventID::time_update_before == event_id) ?
                    a_util::strings::format("old time '%lld', ", old_time.count()).c_str() :
                    "",
                new_time.count());
            return time_update_msg.c_str();
        };

        FEP3_LOG_WARNING(a_util::strings::format(
            "Client clock is not started yet (participant didn't get start "
            "transition command),  make sure the init and start priority "
            "of the timing master is set correctly"
            "Received master time event will not be forwarded to client clock. %s",
            event_log()));
        return getTime();
    }

    if (event_id == fep3::rpc::IRPCClockSyncMasterDef::EventID::time_reset) {
        resetEvent(new_time);
    }
    else if (event_id == fep3::rpc::IRPCClockSyncMasterDef::EventID::time_updating) {
        timeUpdateEvent(new_time, next_tick);
    }

    return getTime();
}

void MasterOnDemandClockDiscrete::resetEvent(const Timestamp new_time)
{
    const auto old_time = _event_sink_and_time.getCurrentTime();
    auto _event_sink_pointer = _event_sink_and_time.getEventSink();

    if (_event_sink_pointer) {
        _event_sink_pointer->timeResetBegin(old_time, new_time);
    }

    _event_sink_and_time.setCurrentTime(new_time);

    if (_event_sink_pointer) {
        _event_sink_pointer->timeResetEnd(new_time);
    }
    _reset = true;
}

void MasterOnDemandClockDiscrete::timeUpdateEvent(const Timestamp new_time,
                                                  const std::optional<Timestamp> next_tick)
{
    if (!_reset) {
        // log warning
        FEP3_LOG_WARNING(
            "Reset was not received from timing master before the time update event, make sure the "
            "init and start priority of the timing master is set correctly");
        // we got to set our own reset if we missed the one from the server.
        resetEvent(_event_sink_and_time.getCurrentTime());
    }

    const auto old_time = _event_sink_and_time.getCurrentTime();
    auto event_sink_pointer = _event_sink_and_time.getEventSink();

    if (event_sink_pointer) {
        event_sink_pointer->timeUpdateBegin(old_time, new_time);
    }

    _event_sink_and_time.setCurrentTime(new_time);

    if (event_sink_pointer) {
        event_sink_pointer->timeUpdating(new_time, next_tick);
    }
    if (event_sink_pointer) {
        event_sink_pointer->timeUpdateEnd(new_time);
    }
}

} // namespace fep3::native
