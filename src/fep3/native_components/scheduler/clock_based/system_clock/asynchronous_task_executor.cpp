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

#include "asynchronous_task_executor.h"

namespace fep3::native {

fep3::Timestamp getContinousTaskNextTimestamp(fep3::Timestamp next_instant,
                                              fep3::Timestamp current_time,
                                              Duration period)
{
    assert(current_time >= next_instant &&
           "Simulation time should be greater or equal to the jobs's next execution time");

    assert(period >= Duration{0} && "Jobs's period should be greater than zero");

    auto integer_steps_until_next = (current_time - next_instant) / period;
    auto next_calculated_instant = next_instant + period * integer_steps_until_next;
    // we have an integer division, in case of a fraction we advance one period
    if (next_calculated_instant <= current_time) {
        next_calculated_instant += period;
    }

    assert(next_calculated_instant > current_time);
    return next_calculated_instant;
}

AsyncTaskExecutor::AsyncTaskExecutor(IThreadPoolExecutor& threaded_executor,
                                     std::shared_ptr<const fep3::ILogger> logger)
    : _threaded_executor(threaded_executor), _logger(logger)

{
}

AsyncTaskExecutor::~AsyncTaskExecutor()
{
    stop();
}

fep3::Result AsyncTaskExecutor::addTask(std::function<void(fep3::Timestamp)> task,
                                        const std::string& name,
                                        Timestamp next_instant,
                                        Duration period,
                                        Duration delay)
{
    _dispatched_tasks_running_status[name] = true;
    return _task_storage.addTask(task, name, next_instant, period, delay);
}

fep3::Duration AsyncTaskExecutor::run(Timestamp current_time)
{
    using namespace std::chrono_literals;

    if (!_running) {
        FEP3_ARYA_LOGGER_LOG_DEBUG(
            _logger,
            "Scheduling run without the scheduler being started, no jobs will be scheduled")
        return 0ns;
    }

    auto& timers_list = _task_storage.get();

    if (timers_list.empty()) {
        // emit a warning or something
        FEP3_ARYA_LOGGER_LOG_DEBUG(_logger, "Scheduling does not have any jobs to schedule")
        return 1s;
    }

    // get the tasks to execute
    auto tasks_to_be_executed = boost::adaptors::filter(timers_list, [&](const auto& t) {
        return t.getPeriod() == fep3::Timestamp{0} || t.getNextInstant() <= current_time;
    });

    for (auto& scheduler_task: tasks_to_be_executed) {
        std::atomic<bool>& bool_complete =
            _dispatched_tasks_running_status.at(scheduler_task.getName());

        if (bool_complete) {
            bool_complete = false;
            _threaded_executor.post([&, timer_info = scheduler_task, current_time]() mutable {
                timer_info.run(current_time);
                bool_complete = true;
            });
        }
        else {
            // put a warning that the old task is still running
            FEP3_ARYA_LOGGER_LOG_DEBUG(
                _logger,
                a_util::strings::format("Task '%s' not scheduled in time %lld ns, previous "
                                        "task call is not yet finished",
                                        scheduler_task.getName().c_str(),
                                        current_time.count()));
        }
    }

    // increment for the next iteration

    boost::range::for_each(tasks_to_be_executed, [&](auto& scheduler_task) {
        if (scheduler_task.getPeriod() > 0ns) {
            scheduler_task.setNextInstant(getContinousTaskNextTimestamp(
                scheduler_task.getNextInstant(), current_time, scheduler_task.getPeriod()));
        }
    });

    // remove one shot tasks
    // TODO check what happens on re initialize with one shot tasks
    boost::range::remove_erase_if(timers_list,
                                  [](const auto t) { return t.getPeriod() == fep3::Timestamp{0}; });

    return calculatWaitTimeToNextCycle(timers_list, current_time);
}

void AsyncTaskExecutor::start()
{
    size_t num_of_threads_in_pool = _task_storage.get().size();

    if (num_of_threads_in_pool > max_pool_size) {
        FEP3_ARYA_LOGGER_LOG_DEBUG(
            _logger,
            a_util::strings::format(
                "Maximum allowed number of threads in job execution pool is '%lld', "
                "total number of jobs is %lld.",
                max_pool_size,
                num_of_threads_in_pool));

        num_of_threads_in_pool = max_pool_size;
    }

    _running = true;
}

void AsyncTaskExecutor::stop()
{
    _running = false;
    // for any running tasks before returning
    //  is guaranteed that the boost
    //  thread pool does not have any further jobs,
    FEP3_ARYA_LOGGER_LOG_DEBUG(_logger, "Wait for jobs in scheduler thread pool to finish")
    FEP3_ARYA_LOGGER_LOG_DEBUG(
        _logger, "All jobs in scheduler thread pool are finished, no more jobs will be scheduled")
}

void AsyncTaskExecutor::prepareForNextStart()
{
    _task_storage.stop();
}

void AsyncTaskExecutor::timeReset(Timestamp old_time, Timestamp new_time)
{
    _task_storage.timeReset(old_time, new_time);
}
fep3::Duration AsyncTaskExecutor::calculatWaitTimeToNextCycle(
    const std::list<SchedulerTask>& timers_list, Timestamp current_time)
{
    using namespace std::chrono_literals;

    fep3::Duration time_to_wait{0};
    auto min_it = boost::range::min_element(timers_list);
    if (min_it != timers_list.end()) {
        time_to_wait = min_it->getNextInstant() - current_time;
    }
    else {
        // means is empty, add a warning
        FEP3_ARYA_LOGGER_LOG_DEBUG(
            _logger, a_util::strings::format("No jobs to schedule, scheduler job queue is empty"))
        return _wait_time_no_tasks;
    }

    if (!_running) {
        time_to_wait = _wait_time_not_running;
    }
    else {
        assert(time_to_wait >= Duration(0) &&
               "Clock based scheduler may not use a duration <= 0 as waiting time, this is a bug "
               "in AsyncTaskExecutor");
    }

    return time_to_wait;
}

} // namespace fep3::native
