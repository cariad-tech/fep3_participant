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


#include "master_on_demand_clock_client.h"

#include <exception>
#include <string>

#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>

#include <fep3/native_components/clock_sync/clock_sync_service.h>

using namespace std::chrono;

namespace fep3
{
namespace rpc
{
namespace arya
{

int getEventIDFlags(const bool before_and_after_event)
{
    if (before_and_after_event)
    {
        return static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_before) |
               static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating) |
               static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_after) |
               static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset);
    }
    else
    {
        return static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating) |
               static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset);
    }
}

FarClockUpdater::FarClockUpdater(
    Duration on_demand_step_size,
    const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
    const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
    bool before_and_after_event,
    const IComponents& components,
    const std::string& local_participant_name)
        : _before_and_after_event(before_and_after_event)
        , _far_clock_master(IRPCClockSyncMasterDef::getRPCDefaultName(), participant_requester)
        , _stop(true)
        , _started(false)
        , _master_type(-1)
        , _on_demand_step_size(on_demand_step_size)
        , _next_request_gettime(std::chrono::time_point<steady_clock>{Timestamp{ 0 }})
        , _participant_server(participant_server)
        , _local_participant_name(local_participant_name)
{
    initLogger(components, "slave_clock.clock_sync_service.component");
}

FarClockUpdater::~FarClockUpdater()
{
    stopWorkingIfStarted();
}

void FarClockUpdater::startRPC()
{
    registerToRPC();
    registerToMaster();

}

void FarClockUpdater::startWork()
{
    if (_master_type != static_cast<int>(IClock::ClockType::discrete))
    {
        startWorking();
    }
}

void FarClockUpdater::stopRPC()
{
    unregisterFromMaster();
    unregisterFromRPC();
}

void FarClockUpdater::registerToRPC()
{
    _participant_server->registerService(IRPCClockSyncSlaveDef::getRPCDefaultName(), shared_from_this());
}

void FarClockUpdater::unregisterFromRPC() const
{
    _participant_server->unregisterService(IRPCClockSyncSlaveDef::getRPCDefaultName());
}

void FarClockUpdater::startWorking()
{
    stopWorkingIfStarted(); //i will start afterwards

    {
        std::lock_guard<std::mutex> guard(_thread_mutex);

        _stop = false;
        _started = true;
        _next_request_gettime = time_point<steady_clock>{ Timestamp{ 0 } };
        _worker = std::thread([this] { work();  });
    }
}

bool FarClockUpdater::stopWorkingIfStarted()
{
    std::lock_guard<std::mutex> guard(_thread_mutex);

    _stop = true;
    if (_started)
    {
        if (_worker.joinable())
        {
            _worker.join();
        }

        _started = false;
        return true;
    }
    else
    {
        return false;
    }
}

void FarClockUpdater::registerToMaster()
{
    FEP3_LOG_DEBUG("Requesting timing master main clock type.");

    try
    {
        _master_type = _far_clock_master.getMasterType();

        FEP3_LOG_DEBUG(a_util::strings::format(
            "Successfully retrieved timing master clock type '%d' (%s).", _master_type, 0 == _master_type ? "continuous" : "discrete"));
    }
    catch (const std::exception& exception)
    {
        FEP3_LOG_WARNING(a_util::strings::format(
            "Retrieving timing master clock type failed during timing slave registration: '%s'", exception.what()));
    }

    FEP3_LOG_DEBUG("Requesting registration as timing slave at the timing master.");

    try
    {
        _far_clock_master.registerSyncSlave(
            getEventIDFlags(_before_and_after_event),
            _local_participant_name);

        FEP3_LOG_DEBUG("Successfully registered as timing slave at the timing master.");
    }
    catch (const std::exception& exception)
    {
        _master_type = -1;

        FEP3_LOG_WARNING(a_util::strings::format(
            "Failure during registration as timing slave at the timing master: '%s'", exception.what()));
    }
}

void FarClockUpdater::unregisterFromMaster()
{
    FEP3_LOG_DEBUG("Requesting deregistration as timing slave from the timing master.");

    try
    {
        _far_clock_master.unregisterSyncSlave(_local_participant_name);

        FEP3_LOG_DEBUG("Successfully deregistered as timing slave from the timing master.");
    }
    catch (const std::exception& exception)
    {
        FEP3_LOG_WARNING(a_util::strings::format(
            "Failure during deregistration as timing slave from the timing master: '%s'", exception.what()));
    }
}

std::string FarClockUpdater::syncTimeEvent(int event_id,
                                           const std::string& new_time,
                                           const std::string& old_time)
{
    if (static_cast<uint8_t>(IRPCClockSyncMasterDef::EventID::time_update_before) == event_id)
    {
        FEP3_LOG_DEBUG(a_util::strings::format(
            "Received master time event. Event id '%d', old time '%s', new time '%s'.", event_id, old_time.c_str(), new_time.c_str()));
    }
    else
    {
        FEP3_LOG_DEBUG(a_util::strings::format(
            "Received master time event. Event id '%d', new time '%s'.", event_id, new_time.c_str()));
    }

    const auto time = masterTimeEvent(
        static_cast<rpc::IRPCClockSyncMasterDef::EventID>(event_id),
        Timestamp{ a_util::strings::toInt64(new_time) },
        Timestamp{ a_util::strings::toInt64(old_time) });

    return a_util::strings::toString(time.count());
}

bool FarClockUpdater::isClientRegistered() const
{
    return _master_type != -1;
}

void FarClockUpdater::work()
{
    using namespace std::chrono;

    while (!_stop)
    {
        if (time_point<steady_clock>{Timestamp{ 0 }} == _next_request_gettime)
        {
            // go ahead
        }
        else
        {
            std::unique_lock<std::mutex> guard(_update_mutex);

            auto current_demand_time_diff = (
                _next_request_gettime - steady_clock::now());

            if (current_demand_time_diff > Timestamp{ 0 })
            {
                if (current_demand_time_diff > Timestamp{ 5 })

                {
                    _cycle_wait_condition.wait_for(guard, current_demand_time_diff);
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        }

        try
        {
            if (!isClientRegistered())
            {
                registerToMaster();
            }

            if (_master_type == static_cast<int>(IClock::ClockType::continuous))
            /// we only auto sync if the timing master clock is of type continuous
            {
                FEP3_LOG_DEBUG("Requesting master time to synchronize local time with timing master.");

                time_point<steady_clock> begin_request = steady_clock::now();
                std::string master_time = _far_clock_master.getMasterTime();

                FEP3_LOG_DEBUG(a_util::strings::format("Retrieved master time: '%s'.", master_time.c_str()));

                const Timestamp current_time{ a_util::strings::toInt64(master_time) };
                {
                    std::lock_guard<std::mutex> locked(_update_mutex);
                    updateTime(current_time, steady_clock::now() - begin_request);
                }
            }
            else
            {
                // Unknown type
            }
            _next_request_gettime = steady_clock::now() + _on_demand_step_size;
        }
        catch (const std::exception& exception)
        {
            FEP3_LOG_WARNING(a_util::strings::format(
                "Failure during synchronization with timing master: '%s'", exception.what()));

            if (!_stop)
            {
                registerToMaster();
            }
        }
    }
}

MasterOnDemandClockInterpolating::MasterOnDemandClockInterpolating(
    const Duration on_demand_step_size,
    const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
    const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
    const IComponents& components,
    std::unique_ptr<fep3::IInterpolationTime> interpolation_time,
    const std::string& local_participant_name)
    : FarClockUpdater(on_demand_step_size,
                        participant_server,
                        participant_requester,
                        false,
                        components,
                        local_participant_name)
    , ContinuousClock(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND)
    , _current_interpolation_time(std::move(interpolation_time))
{
}

Timestamp MasterOnDemandClockInterpolating::getNewTime() const
{
    return _current_interpolation_time->getTime();
}

Timestamp MasterOnDemandClockInterpolating::resetTime(Timestamp new_time)
{
    const bool was_stopped = stopWorkingIfStarted();
    _current_interpolation_time->resetTime(new_time);
    if (was_stopped)
    {
        startWorking();
    }

    return new_time;
}

void MasterOnDemandClockInterpolating::updateTime(const Timestamp new_time, const Duration roundtrip_time)
{
    return _current_interpolation_time->setTime(new_time, roundtrip_time);
}

Timestamp MasterOnDemandClockInterpolating::masterTimeEvent(
    const rpc::IRPCClockSyncMasterDef::EventID event_id,
    Timestamp new_time,
    Timestamp /**old_time*/)
{
    if (event_id == IRPCClockSyncMasterDef::EventID::time_reset)
    {
        reset(new_time);
    }
    return getTime();
}

void MasterOnDemandClockInterpolating::start(const std::weak_ptr<IEventSink>& event_sink)
{
    ContinuousClock::start(event_sink);
}

void MasterOnDemandClockInterpolating::stop()
{
    ContinuousClock::stop();
}

MasterOnDemandClockDiscrete::MasterOnDemandClockDiscrete(
    const Duration on_demand_step_size,
    const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
    const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
    const bool beforeAndAfterEvent,
    const IComponents& components,
    const std::string& local_participant_name)
        : FarClockUpdater(on_demand_step_size,
            participant_server,
            participant_requester,
            beforeAndAfterEvent,
            components,
            local_participant_name)
        , DiscreteClock(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE)
{
}

void MasterOnDemandClockDiscrete::resetOnEvent(Timestamp new_time)
{
    const auto was_stopped = stopWorkingIfStarted();

    DiscreteClock::reset(new_time);
    if (was_stopped)
    {
        startWorking();
    }
}

void MasterOnDemandClockDiscrete::updateTime(const Timestamp new_time, Duration /**roundtrip_time*/)
{
    DiscreteClock::setNewTime(new_time, true);
}

void MasterOnDemandClockDiscrete::start(const std::weak_ptr<IEventSink>& event_sink)
{
    DiscreteClock::start(event_sink);
}

void MasterOnDemandClockDiscrete::stop()
{
    DiscreteClock::stop();
}

Timestamp MasterOnDemandClockDiscrete::masterTimeEvent(
    const IRPCClockSyncMasterDef::EventID event_id,
    const Timestamp new_time,
    const Timestamp old_time)
{
    if (!ClockBase::_started)
    {
        auto event_log = a_util::strings::format("Event id '%d', %snew time '%lld'.",
            event_id,
            (IRPCClockSyncMasterDef::EventID::time_update_before == event_id) ?
            a_util::strings::format("old time '%lld', ", old_time.count()).c_str() : "",
            new_time.count());
        FEP3_LOG_WARNING(a_util::strings::format(
            "Client clock is not started yet (participant didn't get start transition command), received master time event is blocked. %s", event_log.c_str()));
        return getTime();
    }

    if (event_id == IRPCClockSyncMasterDef::EventID::time_reset)
    {
        if (new_time != old_time)
        {
            resetOnEvent(new_time);
        }
    }
    else if (event_id == IRPCClockSyncMasterDef::EventID::time_update_before)
    {
        std::lock_guard<std::mutex> guard(_update_mutex);
        auto sink_ptr = _event_sink.lock();
        if (sink_ptr)
        {
            sink_ptr->timeUpdateBegin(old_time, new_time);
        }
    }
    else if (event_id == IRPCClockSyncMasterDef::EventID::time_updating)
    {
        DiscreteClock::setNewTime(new_time, _before_and_after_event);
    }
    else if (event_id == IRPCClockSyncMasterDef::EventID::time_update_after)
    {
        std::lock_guard<std::mutex> guard(_update_mutex);
        auto sink_ptr = _event_sink.lock();
        if (sink_ptr)
        {
            sink_ptr->timeUpdateEnd(new_time);
        }
    }
    return getTime();
}

} // namespace arya
} // namespace rpc
} // namespace fep3
