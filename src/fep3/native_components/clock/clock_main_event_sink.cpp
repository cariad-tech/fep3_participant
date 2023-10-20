/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "clock_main_event_sink.h"

#include "rpc_clock_sync_client.h"

#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/logger_intf.h>

#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>

namespace fep3 {
namespace rpc {
using namespace std::chrono;
using namespace a_util::strings;

const nanoseconds validateTimeout(const ILogger& logger, const nanoseconds time_update_timeout)
{
    if (time_update_timeout < nanoseconds(FEP3_TIME_UPDATE_TIMEOUT_MIN_VALUE)) {
        if (logger.isWarningEnabled()) {
            logger.logWarning(format("Configured time_update_timeout of '%lld'ns is below minimum "
                                     "of '%lld'ns. Using minimum value instead.",
                                     time_update_timeout,
                                     nanoseconds(FEP3_TIME_UPDATE_TIMEOUT_MIN_VALUE)));
        }

        return nanoseconds(FEP3_TIME_UPDATE_TIMEOUT_MIN_VALUE);
    }

    return time_update_timeout;
}

ClockMainEventSink::ClockMainEventSink(
    const std::shared_ptr<const ILogger>& logger,
    nanoseconds time_update_timeout,
    const std::function<const std::shared_ptr<IRPCRequester>(
        const std::string& service_participant_name)> get_rpc_requester_by_name)
    : _logger(logger),
      _time_update_timeout(validateTimeout(*logger, time_update_timeout)),
      _clients_synchronizer(_time_update_timeout, logger),
      _get_rpc_requester_by_name(get_rpc_requester_by_name)

{
    createUpdateFunctions();
}

void ClockMainEventSink::createUpdateFunctions()
{
    _func_time_update_begin =
        [](RPCClockSyncClient& client, const Timestamp new_time, const Timestamp old_time) -> void {
        toInt64(client.syncTimeEvent(
            static_cast<int>(IRPCClockSyncMasterDef::EventID::time_update_before),
            toString(new_time.count()),
            toString(old_time.count()),
            ""));
    };

    _func_time_updating = [](RPCClockSyncClient& client,
                             const Timestamp new_time,
                             std::optional<Timestamp> next_tick) -> void {
        std::string next_tick_string;
        if (next_tick) {
            next_tick_string = toString(next_tick.value().count());
        }
        toInt64(
            client.syncTimeEvent(static_cast<int>(IRPCClockSyncMasterDef::EventID::time_updating),
                                 toString(new_time.count()),
                                 next_tick_string,
                                 toString(0)));
    };

    _func_time_update_end = [](RPCClockSyncClient& client, const Timestamp new_time) -> void {
        toInt64(client.syncTimeEvent(
            static_cast<int>(IRPCClockSyncMasterDef::EventID::time_update_after),
            toString(new_time.count()),
            toString(0),
            ""));
    };

    _func_time_reset_begin =
        [](RPCClockSyncClient& client, const Timestamp new_time, const Timestamp old_time) -> void {
        toInt64(client.syncTimeEvent(static_cast<int>(IRPCClockSyncMasterDef::EventID::time_reset),
                                     toString(new_time.count()),
                                     toString(old_time.count()),
                                     ""));
    };
}

fep3::Result ClockMainEventSink::registerClient(const std::string& client_name, int event_id_flag)
{
    std::lock_guard<std::mutex> lock(_clients_mutex);

    auto rpc_requester = _get_rpc_requester_by_name(client_name);
    if (!rpc_requester) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Requester not found");
    }

    auto it = _clients.find(client_name);
    if (it != _clients.end()) {
        it->second->_client->setEventIDFlag(event_id_flag);
        it->second->_client->activate();
    }
    else {
        auto client = std::make_unique<ClientEntry>(
            (std::make_shared<RPCClockSyncClient>(client_name, rpc_requester, event_id_flag)));

        _clients[client_name] = std::move(client);
        _clients[client_name]->_client->activate();
    }

    return {};
}

fep3::Result ClockMainEventSink::unregisterClient(const std::string& client_name)
{
    std::lock_guard<std::mutex> lock(_clients_mutex);

    auto it = _clients.find(client_name);
    if (it != _clients.end()) {
        it->second->_client->deactivate();
        return {};
    }

    RETURN_ERROR_DESCRIPTION(
        ERR_NOT_FOUND,
        format("a client with name '%s' was not found", client_name.c_str()).c_str());
}

fep3::Result ClockMainEventSink::receiveClientSyncedEvent(const std::string& /*client_name*/,
                                                          Timestamp /*time*/)
{
    return {};
}

fep3::Result ClockMainEventSink::updateTimeout(const nanoseconds time_update_timeout)
{
    _time_update_timeout = validateTimeout(*_logger, time_update_timeout);
    _clients_synchronizer._time_update_timeout = _time_update_timeout;

    createUpdateFunctions();

    return {};
}

void ClockMainEventSink::timeUpdateBegin(Timestamp old_time, Timestamp new_time)
{
    std::lock_guard<std::mutex> lock(_clients_mutex);

    auto func_wrapper = [&](RPCClockSyncClient& client) {
        _func_time_update_begin(client, new_time, old_time);
    };

    synchronizeEvent(func_wrapper,
                     IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_before,
                     format("an error occured during time_update_before at time %lld", new_time));
}

void ClockMainEventSink::timeUpdating(Timestamp new_time, std::optional<Timestamp> next_tick)
{
    std::lock_guard<std::mutex> lock(_clients_mutex);

    auto func_wrapper = [&](RPCClockSyncClient& client) {
        _func_time_updating(client, new_time, next_tick);
    };

    synchronizeEvent(func_wrapper,
                     IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating,
                     format("an error occured during time_updating at time %lld", new_time));
}

void ClockMainEventSink::timeUpdateEnd(Timestamp new_time)
{
    std::lock_guard<std::mutex> lock(_clients_mutex);

    auto func_wrapper = [&](RPCClockSyncClient& client) {
        _func_time_update_end(client, new_time);
    };

    synchronizeEvent(func_wrapper,
                     IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_after,
                     format("an error occured during time_update_after at time %lld", new_time));
}

void ClockMainEventSink::timeResetBegin(Timestamp old_time, Timestamp new_time)
{
    std::lock_guard<std::mutex> lock(_clients_mutex);

    auto func_wrapper = [&](RPCClockSyncClient& client) {
        _func_time_reset_begin(client, new_time, old_time);
    };

    synchronizeEvent(func_wrapper,
                     IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset,
                     format("an error occured during time_reset at old time %lld", old_time));
}

void ClockMainEventSink::timeResetEnd(Timestamp /*new_time*/)
{
    // ignore
}

void ClockMainEventSink::synchronizeEvent(const std::function<void(RPCClockSyncClient&)>& sync_func,
                                          const IRPCClockSyncMasterDef::EventIDFlag event_id_flag,
                                          const std::string& message) const
{
    try {
        _clients_synchronizer.synchronize(_clients, sync_func, event_id_flag);
    }
    catch (const std::exception& ex) {
        const auto actual_message = std::string(message + ": " + ex.what());
        _logger->logError(actual_message);
    }
}

AsyncExecutor::AsyncExecutor()
{
    _sync_thread = std::thread(&AsyncExecutor::executionLoop, this);
}

AsyncExecutor::~AsyncExecutor()
{
    {
        std::lock_guard<std::mutex> lock(_tasks_mutex);
        _stop = true;
        _condition_sync_start.notify_one();
    }

    if (_sync_thread.joinable()) {
        _sync_thread.join();
    }
}

void AsyncExecutor::executionLoop()
{
    while (!_stop) {
        {
            std::unique_lock<std::mutex> lock(_tasks_mutex);
            // It should wakeup in 2 cases, both need to be guarded in the predicate in case of
            // spurious wakeup:
            //   1. If tasks are enqueued
            //   2. If executor has to stop
            _condition_sync_start.wait(lock, [this]() { return !_tasks.empty() || _stop == true; });
        }

        /// we check here again because we can be woken up by destructor
        if (!_stop) {
            std::packaged_task<void()> task;

            {
                std::unique_lock<std::mutex> lock(_tasks_mutex);
                if (!_tasks.empty()) {
                    task = std::move(_tasks.front());
                    _tasks.pop();
                }
            }

            if (task.valid()) {
                (task)();
            }
        }
    }
}

ClockMainEventSink::MultipleClientsSynchronizer::MultipleClientsSynchronizer(
    nanoseconds timeout, const std::shared_ptr<const ILogger>& logger)
    : _time_update_timeout(timeout), _logger(logger)
{
}

void ClockMainEventSink::MultipleClientsSynchronizer::synchronize(
    const std::map<std::string, std::unique_ptr<ClockMainEventSink::ClientEntry>>& clients,
    std::function<void(RPCClockSyncClient&)> sync_func,
    const IRPCClockSyncMasterDef::EventIDFlag event_id_flag) const
{
    std::vector<std::pair<ClientEntry&, std::future<void>>> synchronizations;

    for (const auto& it: clients) {
        const auto& client_entry = it.second;
        auto clock_client = it.second->_client;
        auto& client_synchronizer = it.second->_async_executor;

        if (!clock_client->isActive()) {
            continue;
        }

        if (clock_client->isSet(event_id_flag)) {
            auto sync_func_client_binded = [clock_client, sync_func]() {
                sync_func(*clock_client);
            };

            synchronizations.emplace_back(*client_entry,
                                          client_synchronizer.enqueueTask(sync_func_client_binded));
        }
    }

    waitUntilSyncFinish(synchronizations);
}

void ClockMainEventSink::MultipleClientsSynchronizer::waitUntilSyncFinish(
    std::vector<std::pair<ClientEntry&, std::future<void>>>& current_synchronizations) const
{
    const auto timeout_until = system_clock::now() + _time_update_timeout;

    for (auto& synchronization: current_synchronizations) {
        const auto client_name = synchronization.first._client->getName();

        const auto status = synchronization.second.wait_until(timeout_until);
        if (status == std::future_status::timeout) {
            const auto message =
                format("a timeout occured while synchronizing the client '%s'. "
                       "The client might take too long to respond or be unreachable.",
                       client_name.c_str());

            _logger->logError(message);
        }
        else if (status == std::future_status::ready) {
            try {
                synchronization.second.get();
            }
            catch (const jsonrpc::JsonRpcException& ex) {
                const auto message =
                    format("an error occured during synchronization of client '%s'. "
                           "Invalid response received. Client will be deactivated: %s",
                           client_name.c_str(),
                           ex.what());

                _logger->logError(message);

                synchronization.first._client->deactivate();
            }
        }
        else {
            const auto message =
                format("synchronization thread for client '%s' was deferred.", client_name.c_str());
            throw std::runtime_error(message);
        }
    }
}

} // namespace rpc
} // namespace fep3
