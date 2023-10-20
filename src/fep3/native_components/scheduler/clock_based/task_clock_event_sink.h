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

#include "scheduler_factory.h"
#include "task_executor_intf.h"
#include "threaded_executor.h"

#include <fep3/components/logging/logger_intf.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>

#include <future>

namespace fep3::native {

// TODO check if this is an one shot object nad after stop() is not reusable
// scheduler creates a new one anyway

class TaskClockEventSink : public fep3::experimental::IClock::IEventSink {
public:
    TaskClockEventSink(arya::IClock::ClockType clock_type,
                       std::function<Timestamp()> time_getter,
                       std::shared_ptr<const fep3::ILogger> logger,
                       std::shared_ptr<const ISchedulerFactory> factory,
                       IThreadPoolExecutor& threaded_executor)
        : _task_scheduler_factory(std::move(factory)), _time_getter(time_getter)
    {
        _task_scheduler = _task_scheduler_factory->createSchedulerProcessor(
            threaded_executor, clock_type, _time_getter, logger);
    }

    virtual ~TaskClockEventSink();

    fep3::Result addTask(std::function<void(fep3::Timestamp)> task,
                         const std::string& name,
                         Duration period,
                         Duration initial_delay);

    fep3::Result start();

    fep3::Result stop();

    // IClock::IEventSink
    void timeUpdateBegin(Timestamp old_time, Timestamp new_time) override;
    void timeUpdating(Timestamp new_time, std::optional<arya::Timestamp> next_tick) override;
    void timeUpdateEnd(Timestamp new_time) override;
    void timeResetBegin(Timestamp old_time, Timestamp new_time) override;
    void timeResetEnd(Timestamp new_time) override;

private:
    const std::function<Timestamp()> _time_getter;

    std::unique_ptr<ITaskExecutorInvoker> _task_scheduler;
    std::shared_ptr<const ISchedulerFactory> _task_scheduler_factory;
};

} // namespace fep3::native
