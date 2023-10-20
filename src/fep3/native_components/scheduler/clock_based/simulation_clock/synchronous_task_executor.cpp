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

#include "synchronous_task_executor.h"

#include "threaded_executor.h"

#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/join.hpp>

#include <future>

namespace fep3::native {

bool taskHasTobeRun(const SchedulerTask& task, const std::optional<fep3::Timestamp>& next_timestep)
{
    if (task.getPeriod() == fep3::Timestamp{0}) {
        return true;
    }
    else if (next_timestep) {
        return task.getNextInstant() == next_timestep.value();
    }
    else {
        return false;
    }
}

SyncTaskExecutor::SyncTaskExecutor(IThreadPoolExecutor& threaded_executor)
    : _threaded_executor(threaded_executor)
{
}

fep3::Result SyncTaskExecutor::addTask(std::function<void(fep3::Timestamp)> task,
                                       const std::string& name,
                                       Timestamp next_instant,
                                       Duration period,
                                       Duration delay)
{
    return _task_storage.addTask(task, name, next_instant, period, delay);
}

// TODO: does it matter with what timestamp the one shots are called?
void SyncTaskExecutor::run(Timestamp current_time, std::optional<Timestamp> next_time)
{
    auto& timers_list = _task_storage.get();

    bool tasks_to_execute = true;
    // if this is a problem, we can add a stop method and a flag to break the while loop
    // but only for further executions, the thread pool wil execute what is on queue
    while (tasks_to_execute) {
        std::optional<fep3::Timestamp> next_timestep = getNearestSubStep(timers_list, current_time);

        auto tasks_to_be_executed = boost::adaptors::filter(
            timers_list, [&](const auto& t) { return taskHasTobeRun(t, next_timestep); });

        Timestamp executiont_time = current_time;
        if (next_timestep.has_value()) {
            executiont_time = next_timestep.value();
        }
        runTasksInQueue(tasks_to_be_executed, executiont_time);

        // increment for the next iteration
        boost::range::for_each(tasks_to_be_executed, [](auto& scheduler_task) {
            scheduler_task.setNextInstant(scheduler_task.getNextInstant() +
                                          scheduler_task.getPeriod());
        });

        // block scheduler if tasks are not finished in time
        waitForTasksInQueue(timers_list, current_time, next_time);

        // remove one shot tasks
        boost::range::remove_erase_if(
            timers_list, [](const auto t) { return t.getPeriod() == fep3::Timestamp{0}; });

        auto rest_tasks_for_current_run = boost::adaptors::filter(
            // all one shots are removed, no need to check
            timers_list,
            [&](const auto& t) { return t.getNextInstant() <= current_time; });

        tasks_to_execute = (boost::size(rest_tasks_for_current_run) > 0);
    }
}

template <typename T>
std::optional<fep3::Timestamp> SyncTaskExecutor::getNearestSubStep(
    const T& tasks, const fep3::Timestamp& current_time)
{
    // tasks with next instant smaller as the current time
    auto tasks_in_current_time = boost::adaptors::filter(
        tasks, [&](const auto& t) { return t.getNextInstant() <= current_time; });

    // get the nearest next instant
    auto min_it = boost::range::min_element(tasks_in_current_time);

    if (min_it != tasks_in_current_time.end()) {
        return min_it->getNextInstant();
    }
    else {
        return {};
    }
}

template <typename T>
void SyncTaskExecutor::runTasksInQueue(const T& tasks, const fep3::Timestamp& executiont_time)
{
    for (auto& scheduler_task: tasks) {
        _wait_tokens.emplace_back(std::ref(scheduler_task),
                                  _threaded_executor.postWithCompletionFuture(
                                      [scheduler_task, executiont_time]() mutable {
                                          scheduler_task.run(executiont_time);
                                      }));
    }
}

bool SyncTaskExecutor::taskToBeWaited(const SchedulerTask& task,
                                      const fep3::Timestamp& current_time,
                                      const std::optional<Timestamp>& next_time)
{
    bool checkTaskStatus = false;

    if (task.getPeriod() == fep3::Timestamp{0}) {
        // one shot task
        checkTaskStatus = true;
    }
    else if (next_time) {
        if (current_time >= next_time) {
            // invalid next_time
            checkTaskStatus = true;
        }
        else {
            // check if it is one simtime step before next task execution timepoint
            if (task.getNextInstant() <= next_time) {
                checkTaskStatus = true;
            }
        }
    }
    else {
        // default behaviour: block scheduler when task is not complete
        checkTaskStatus = true;
    }
    return checkTaskStatus;
}

template <typename T>
void SyncTaskExecutor::waitForTasksInQueue(const T& tasks,
                                           const fep3::Timestamp& current_time,
                                           const std::optional<Timestamp>& next_time)
{
    auto tasks_to_be_waited = boost::adaptors::filter(
        tasks, [&](const auto& t) { return taskToBeWaited(t, current_time, next_time); });

    boost::range::for_each(tasks_to_be_waited, [&](auto& task) {
        auto it = boost::range::find_if(
            _wait_tokens, [&](const auto& token) { return &(token.first.get()) == &task; });
        if (it != _wait_tokens.end()) {
            it->second.wait();
            _wait_tokens.erase(it);
        }
    });
}

void SyncTaskExecutor::waitForAllTasksInQueue()
{
    boost::range::for_each(_wait_tokens, [](const auto& t) { t.second.wait(); });
    _wait_tokens.clear();
}

void SyncTaskExecutor::timeReset(Timestamp old_time, Timestamp new_time)
{
    waitForAllTasksInQueue();
    _task_storage.timeReset(old_time, new_time);
}

void SyncTaskExecutor::stop()
{
    waitForAllTasksInQueue();
    _task_storage.stop();
}

} // namespace fep3::native
