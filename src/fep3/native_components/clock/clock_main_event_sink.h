/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "clock_main_event_sink_intf.h"

#include <fep3/fep3_result_decl.h>
#include <fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h>

#include <chrono>
#include <future>
#include <map>
#include <queue>
#include <string>

namespace fep3::arya {
class IServiceBus;
class ILogger;
class IRPCRequester;
} // namespace fep3::arya

namespace fep3::rpc {
class RPCClockSyncClient;
} // namespace fep3::rpc

namespace fep3 {

namespace rpc {

class AsyncExecutor {
public:
    AsyncExecutor();
    ~AsyncExecutor();
    AsyncExecutor(AsyncExecutor&) = delete;
    AsyncExecutor(AsyncExecutor&&) = delete;
    AsyncExecutor& operator=(AsyncExecutor&) = delete;
    AsyncExecutor& operator=(AsyncExecutor&&) = delete;

    template <typename F, typename R = std::result_of_t<F && ()>>
    std::future<void> enqueueTask(F&& f)
    {
        auto task = std::packaged_task<R()>(std::forward<F>(f));
        auto future = task.get_future();

        {
            std::unique_lock<std::mutex> lock(_tasks_mutex);
            _tasks.push(std::move(task));
        }
        _condition_sync_start.notify_one();

        return future;
    }

private:
    void executionLoop();

private:
    std::thread _sync_thread{};
    std::condition_variable _condition_sync_start{};
    std::mutex _tasks_mutex{};
    std::queue<std::packaged_task<void()>> _tasks;
    std::atomic_bool _stop{false};
};

class ClockMainEventSink : public IClockMainEventSink {
public:
    ClockMainEventSink(const std::shared_ptr<const fep3::arya::ILogger>& logger,
                       std::chrono::nanoseconds rpc_timeout,
                       std::function<const std::shared_ptr<fep3::arya::IRPCRequester>(
                           const std::string& service_participant_name)> get_rpc_requester_by_name);

public:
    fep3::Result registerClient(const std::string& client_name, int event_id_flag) override;
    fep3::Result unregisterClient(const std::string& client_name) override;
    fep3::Result receiveClientSyncedEvent(const std::string& client_name, Timestamp time) override;
    fep3::Result updateTimeout(std::chrono::nanoseconds rpc_timeout) override;

public:
    void timeUpdateBegin(Timestamp old_time, Timestamp new_time) override;
    void timeUpdating(Timestamp new_time, std::optional<Timestamp> next_tick) override;
    void timeUpdateEnd(Timestamp new_time) override;
    void timeResetBegin(Timestamp old_time, Timestamp new_time) override;
    void timeResetEnd(Timestamp new_time) override;

    class ClientEntry {
    public:
        explicit ClientEntry(std::shared_ptr<RPCClockSyncClient> client)
            : _client(std::move(client))
        {
        }

        ~ClientEntry() = default;

        ClientEntry(ClientEntry&) = delete;
        ClientEntry(ClientEntry&&) = delete;
        ClientEntry& operator=(ClientEntry&) = delete;
        ClientEntry& operator=(ClientEntry&&) = delete;

    public:
        std::shared_ptr<RPCClockSyncClient> _client;
        AsyncExecutor _async_executor;
    };

    class MultipleClientsSynchronizer {
    public:
        MultipleClientsSynchronizer(std::chrono::nanoseconds timeout,
                                    const std::shared_ptr<const fep3::arya::ILogger>& logger);

        void synchronize(
            const std::map<std::string, std::unique_ptr<ClockMainEventSink::ClientEntry>>& slaves,
            std::function<void(fep3::rpc::RPCClockSyncClient&)> sync_func,
            IRPCClockSyncMasterDef::EventIDFlag event_id_flag) const;

        MultipleClientsSynchronizer(MultipleClientsSynchronizer&) = delete;
        MultipleClientsSynchronizer(MultipleClientsSynchronizer&&) = delete;
        MultipleClientsSynchronizer& operator=(MultipleClientsSynchronizer&) = delete;
        MultipleClientsSynchronizer& operator=(MultipleClientsSynchronizer&&) = delete;

    private:
        void waitUntilSyncFinish(std::vector<std::pair<ClientEntry&, std::future<void>>>&
                                     current_synchronizations) const;

    public:
        std::chrono::nanoseconds _time_update_timeout;

    private:
        std::shared_ptr<const fep3::arya::ILogger> _logger;
    };

private:
    void createUpdateFunctions();
    void synchronizeEvent(const std::function<void(RPCClockSyncClient&)>& sync_func,
                          const IRPCClockSyncMasterDef::EventIDFlag event_id_flag,
                          const std::string& message) const;

private:
    std::shared_ptr<fep3::arya::IServiceBus> _service_bus;
    std::shared_ptr<const fep3::arya::ILogger> _logger;
    std::map<std::string, std::unique_ptr<ClientEntry>> _clients;
    std::chrono::nanoseconds _time_update_timeout;
    MultipleClientsSynchronizer _clients_synchronizer;
    std::mutex _clients_mutex;
    const std::function<const std::shared_ptr<fep3::arya::IRPCRequester>(
        const std::string& service_participant_name)>
        _get_rpc_requester_by_name;

    std::function<void(RPCClockSyncClient&, Timestamp, Timestamp)> _func_time_update_begin;
    std::function<void(RPCClockSyncClient&, Timestamp, std::optional<Timestamp>)>
        _func_time_updating;
    std::function<void(RPCClockSyncClient&, Timestamp)> _func_time_update_end;
    std::function<void(RPCClockSyncClient&, Timestamp, Timestamp)> _func_time_reset_begin;
};
} // namespace rpc
} // namespace fep3
