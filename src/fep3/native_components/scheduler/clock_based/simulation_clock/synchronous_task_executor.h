/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include "../task_executor_intf.h"
#include "../task_storage.h"

#include <optional>

namespace fep3::native {

struct IThreadPoolExecutor;

class SyncTaskExecutor {
public:
    SyncTaskExecutor(IThreadPoolExecutor& threaded_executor);

    void run(Timestamp current_time, std::optional<Timestamp> next_time = std::nullopt);
    void timeReset(Timestamp old_time, Timestamp new_time);

    fep3::Result addTask(std::function<void(fep3::Timestamp)> task,
                         const std::string& name,
                         Timestamp next_instant,
                         Duration period,
                         Duration delay);
    void stop();

private:
    template <typename T>
    void runTasksInQueue(const T& tasks, const fep3::Timestamp& executiont_time);

    template <typename T>
    void waitForTasksInQueue(const T& tasks,
                             const fep3::Timestamp& current_time,
                             const std::optional<Timestamp>& next_time);

    bool taskToBeWaited(const SchedulerTask& task,
                        const fep3::Timestamp& current_time,
                        const std::optional<Timestamp>& next_time);
    void waitForAllTasksInQueue();

    template <typename T>
    std::optional<fep3::Timestamp> getNearestSubStep(const T& tasks,
                                                     const fep3::Timestamp& current_time);

    TaskStorage _task_storage;
    IThreadPoolExecutor& _threaded_executor;
    const fep3::Timestamp _limit_time{0};

    std::vector<std::pair<std::reference_wrapper<SchedulerTask>, std::future<void>>> _wait_tokens;
};

} // namespace fep3::native
