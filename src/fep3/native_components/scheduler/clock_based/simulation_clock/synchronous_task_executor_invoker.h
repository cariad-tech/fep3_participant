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

#pragma once
#include "../task_executor_intf.h"
#include "notification_waiting.h"

#include <fep3/fep3_duration.h>
#include <fep3/fep3_timestamp.h>

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <optional>

namespace fep3::native {

template <typename ProcessorType>
class SyncTaskExecutorInvoker : public ITaskExecutorInvoker {
public:
    using Factory = std::function<ProcessorType()>;

    SyncTaskExecutorInvoker(Factory f) : _timer_queue_processor(f())
    {
    }

    void start() override
    {
        _running = true;
    }

    void stop() override
    {
        std::unique_lock<std::mutex> lock(_mutex_processing_lock);
        _running = false;
        _timer_queue_processor.stop();
    }

    void timeUpdating(Timestamp new_time, std::optional<arya::Timestamp> next_time) override
    {
        // blocks on incoming stop and protects the timers_list from TimeResetBegin
        std::unique_lock<std::mutex> lock(_mutex_processing_lock);
        if (_running) {
            processQueueSynchron(new_time, next_time);
        }
    }

    void timeReset(Timestamp old_time, Timestamp new_time) override
    {
        std::unique_lock<std::mutex> lock(_mutex_processing_lock);
        //_last_execution_time = fep3::Timestamp{-1};
        _timer_queue_processor.timeReset(old_time, new_time);
    }

    fep3::Result addTask(std::function<void(fep3::Timestamp)> task,
                         const std::string& name,
                         Timestamp next_instant,
                         Duration period,
                         Duration delay) override
    {
        return _timer_queue_processor.addTask(task, name, next_instant, period, delay);
    }

private:
    void processQueueSynchron(Timestamp current_time, std::optional<Timestamp> next_time)
    {
        _timer_queue_processor.run(current_time, next_time);
    }

    ProcessorType _timer_queue_processor;
    const fep3::Timestamp _restart_time{-1};
    // protects the _task_storage, _running and _last_execution_time and halts processQueueSynchron
    // when time reset or stop are called
    std::mutex _mutex_processing_lock;
    fep3::Timestamp _last_execution_time{_restart_time};
    bool _running{false};
};
} // namespace fep3::native
