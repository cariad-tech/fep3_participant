/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "../task_executor_intf.h"
#include "../task_storage.h"

#include <fep3/components/logging/easy_logger.h>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/join.hpp>

#include <cassert>
#include <cmath>
#include <map>

namespace fep3::native {

fep3::Timestamp getContinousTaskNextTimestamp(fep3::Timestamp next_instant,
                                              fep3::Timestamp current_time,
                                              Duration period);

class AsyncTaskExecutor {
public:
    AsyncTaskExecutor(IThreadPoolExecutor& threaded_executor,
                      std::shared_ptr<const fep3::ILogger> logger = nullptr);
    ~AsyncTaskExecutor();

    fep3::Result addTask(std::function<void(fep3::Timestamp)> task,
                         const std::string& name,
                         Timestamp next_instant,
                         Duration period,
                         Duration delay);

    fep3::Duration run(Timestamp current_time);

    void start();
    void stop();
    void prepareForNextStart();
    void timeReset(Timestamp old_time, Timestamp new_time);

private:
    fep3::Duration calculatWaitTimeToNextCycle(const std::list<SchedulerTask>& timers_list,
                                               Timestamp current_time);

    IThreadPoolExecutor& _threaded_executor;
    TaskStorage _task_storage;
    const fep3::Timestamp _limit_time{0};
    std::map<std::string, std::atomic<bool>> _dispatched_tasks_running_status;
    std::atomic<bool> _running{false};
    std::shared_ptr<const fep3::ILogger> _logger;
    const fep3::Duration _wait_time_not_running = std::chrono::nanoseconds(0);
    const fep3::Duration _wait_time_no_tasks = std::chrono::milliseconds(500);
    const size_t max_pool_size = 5;
};

} // namespace fep3::native
