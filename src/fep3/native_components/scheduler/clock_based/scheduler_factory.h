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
#include "task_executor_intf.h"

#include <fep3/components/clock/clock_intf.h>
#include <fep3/fep3_timestamp.h>

#include <functional>
#include <memory>

namespace fep3::native {

struct SchedulerFactory : public ISchedulerFactory {
    std::unique_ptr<ITaskExecutorInvoker> createSchedulerProcessor(
        IThreadPoolExecutor& threaded_executor,
        fep3::arya::IClock::ClockType clock_type,
        std::function<fep3::Timestamp()> time_getter,
        std::shared_ptr<const fep3::ILogger> logger) const;
};

} // namespace fep3::native
