/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "asynchronous_task_executor.h"
#include "notification_waiting.h"

#include <fep3/fep3_duration.h>
#include <fep3/fep3_timestamp.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

namespace fep3::native {

class TaskStorage;

template <typename WaitingType, typename ProcessorType>
class AsyncTaskExecutorInvoker : public ITaskExecutorInvoker {
public:
    using Factory = std::function<ProcessorType()>;

    AsyncTaskExecutorInvoker(std::function<fep3::Timestamp()> get_time,
                             Factory f,
                             std::shared_ptr<const fep3::ILogger> logger = nullptr)
        : _async_timer_queue_processor(f()),
          _get_time(get_time),
          _reset_or_stop_notification(true),
          _logger(logger)
    {
    }

    ~AsyncTaskExecutorInvoker()
    {
        if (_running) {
            stop();
        }
    }

    // destructor removes copy semantics (if needed)
    AsyncTaskExecutorInvoker(AsyncTaskExecutorInvoker<WaitingType, ProcessorType>&&) = default;

    AsyncTaskExecutorInvoker<WaitingType, ProcessorType>& operator=(
        AsyncTaskExecutorInvoker<WaitingType, ProcessorType>&&) = default;

    void start() override
    {
        _running = true;
        _async_timer_queue_processor.start();
        _scheduling_thread = std::thread([&]() { processInMainLoop(); });
    }

    void stop() override
    {
        // we stop any further executions of processSchedulerQueueAsynchron
        _running = false;
        // we stop any waiting
        _reset_or_stop_notification.notify();
        // wait until the actual processing cycle is over (if any running)
        if (_scheduling_thread.joinable()) {
            _scheduling_thread.join();
        }
        // stop any active scheduling in the async processor
        // processor will not be run again since the thread is joined
        // and wait for running tasks to finish
        _async_timer_queue_processor.stop();

        // now neither main loop or jobs in thread pool run
        // task next times can be set
        _async_timer_queue_processor.prepareForNextStart();
        // prepare for the next start()
        _reset_or_stop_notification.reset();
    }

    void timeUpdating(Timestamp, std::optional<arya::Timestamp>) override
    {
        FEP3_ARYA_LOGGER_LOG_DEBUG(_logger, "Received time update event");
    }

    void timeReset(Timestamp old_time, Timestamp new_time) override
    {
        FEP3_ARYA_LOGGER_LOG_DEBUG(_logger, "Received time reset event");
        std::unique_lock<std::mutex> lock(_mutex_processing_lock);
        _async_timer_queue_processor.timeReset(old_time, new_time);
        _reset_or_stop_notification.notify();
    }

    fep3::Result addTask(std::function<void(fep3::Timestamp)> task,
                         const std::string& name,
                         Timestamp next_instant,
                         Duration period,
                         Duration initial_delay) override
    {
        return _async_timer_queue_processor.addTask(
            task, name, next_instant, period, initial_delay);
    }

private:
    void processInMainLoop()
    {
        using namespace std::chrono_literals;

        // we wait for the first reset before the loop goes forward
        FEP3_ARYA_LOGGER_LOG_INFO(_logger, "Waiting for clock reset event to start scheduling");
        _reset_or_stop_notification.waitForNotification();
        FEP3_ARYA_LOGGER_LOG_INFO(_logger, "Clock reset event received, starting scheduling");
        // is start always called here when we arrive in the while?, yes see:
        //    FEP3_RETURN_IF_FAILED(_task_executor->start());
        // FEP3_RETURN_IF_FAILED(_service_thread->start());
        while (_running) {
            Duration time_to_wait{100ms};
            {
                std::unique_lock<std::mutex> lock(_mutex_processing_lock);
                const auto current_time = _get_time();
                FEP3_ARYA_LOGGER_LOG_DEBUG(
                    _logger, "Asynchronous executor, running clock triggered jobs scheduling");
                time_to_wait = _async_timer_queue_processor.run(current_time);
            }
            if (time_to_wait < 1ms) {
                // timespan is too short for wait. so just yield the execution.
                std::this_thread::yield();
            }
            else {
                // the next execution is more or equal 1ms.
                // Wait that time or until a time reset comes.
                _reset_or_stop_notification.waitForNotificationWithTimeout(time_to_wait);
            }
        }
    }

    ProcessorType _async_timer_queue_processor;
    std::function<fep3::Timestamp()> _get_time;
    WaitingType _reset_or_stop_notification;

    std::mutex _mutex_processing_lock;
    std::atomic<bool> _running{false};
    std::thread _scheduling_thread;
    std::shared_ptr<const fep3::ILogger> _logger;
};
} // namespace fep3::native
