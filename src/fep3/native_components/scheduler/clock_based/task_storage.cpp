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

#include "task_storage.h"

#include "task_clock_event_sink.h"

#include <boost/range/algorithm.hpp>

namespace fep3::native {

void TaskStorage::timeReset(Timestamp old_time, Timestamp new_time)
{
    const Timestamp time_diff = new_time - old_time;

    for (auto& scheduler_task: _task_list) {
        scheduler_task.setNextInstant(scheduler_task.getNextInstant() + time_diff);
        if (scheduler_task.getNextInstant() < new_time) {
            scheduler_task.setNextInstant(new_time + scheduler_task.getInitialDelay());
        }
    }
}

void TaskStorage::stop()
{
    // when stopping we have to go back one step
    //  assuming job has 100 ms step
    //  job gets called at 0ms, 100ms, 200ms (next instant is 300ms)
    //  when job reset comes will be old time 200ms, new time 0ms
    //  time diff is 200ms, so next instanst calculated from timeReset becomes 100ms
    //  so on the next start job will get called at 100ms and not 0ms

    for (auto& scheduler_task: _task_list) {
        if (scheduler_task.getNextInstant() >= scheduler_task.getPeriod()) {
            scheduler_task.setNextInstant(scheduler_task.getNextInstant() -
                                          scheduler_task.getPeriod());
        }
    }
}

fep3::Result TaskStorage::addTask(std::function<void(fep3::Timestamp)> task,
                                  const std::string& name,
                                  Timestamp next_instant,
                                  Duration period,
                                  Duration delay)
{
    if (taskNameExists(name)) {
        RETURN_ERROR_DESCRIPTION(
            ERR_FAILED,
            "Job with name %s already exists and cannot be added to the scheduler",
            name.c_str());
    }
    if (period < fep3::Timestamp{0}) {
        RETURN_ERROR_DESCRIPTION(
            ERR_FAILED,
            "Invalid period for job with name %s, period value: %lld ns is negative",
            name.c_str(),
            period.count());
    }

    _task_list.push_back(SchedulerTask(std::move(task), name, next_instant, period, delay));
    return {};
}

bool TaskStorage::taskNameExists(const std::string& name)
{
    auto it = std::find_if(_task_list.begin(), _task_list.end(), [&](const auto& task) {
        return task.getName() == name;
    });
    return it != _task_list.end();
}

std::list<SchedulerTask>& TaskStorage::get()
{
    return _task_list;
}

} // namespace fep3::native
