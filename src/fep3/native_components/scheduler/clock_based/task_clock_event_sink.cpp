/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "task_clock_event_sink.h"

#include "scheduler_factory.h"

namespace fep3::native {
TaskClockEventSink::~TaskClockEventSink()
{
    stop();
}

fep3::Result TaskClockEventSink::start()
{
    _task_scheduler->start();
    return {};
}

fep3::Result TaskClockEventSink::stop()
{
    _task_scheduler->stop();
    //_task_scheduler is not destructed since can still receive time updates
    // or resets, implementations will discard them
    return {};
}

fep3::Result TaskClockEventSink::addTask(std::function<void(fep3::Timestamp)> task,
                                         const std::string& name,
                                         Duration period,
                                         Duration initial_delay)
{
    return _task_scheduler->addTask(
        task, name, _time_getter() + initial_delay, period, initial_delay);
}

void TaskClockEventSink::timeResetBegin(Timestamp old_time, Timestamp new_time)
{
    _task_scheduler->timeReset(old_time, new_time);
}

void TaskClockEventSink::timeResetEnd(Timestamp /*new_time*/)
{
}

void TaskClockEventSink::timeUpdateBegin(Timestamp /*old_time*/, Timestamp /*new_time*/)
{
}

void TaskClockEventSink::timeUpdating(Timestamp new_time, std::optional<arya::Timestamp> next_tick)
{
    _task_scheduler->timeUpdating(new_time, next_tick);
}

void TaskClockEventSink::timeUpdateEnd(Timestamp /*new_time*/)
{
}

} // namespace fep3::native
