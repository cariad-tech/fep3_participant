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
#include "scheduler_task.h"

#include <fep3/fep3_duration.h>
#include <fep3/fep3_errors.h>
#include <fep3/fep3_timestamp.h>

#include <future>
#include <list>
#include <map>
#include <string>

namespace fep3::native {

// maybe timer is not the right name for these classes
class TaskStorage {
public:
    void timeReset(Timestamp old_time, Timestamp new_time);
    void stop();

    fep3::Result addTask(std::function<void(fep3::Timestamp)> task,
                         const std::string& name,
                         Timestamp next_instant,
                         Duration period,
                         Duration delay);

    std::list<SchedulerTask>& get();

private:
    bool taskNameExists(const std::string& name);
    const fep3::Timestamp _limit_time{0};
    std::list<SchedulerTask> _task_list;
};
} // namespace fep3::native
