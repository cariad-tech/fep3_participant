/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "threaded_executor.h"

#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/logging/logger_intf.h>
#include <fep3/fep3_duration.h>
#include <fep3/fep3_result_decl.h>
#include <fep3/fep3_timestamp.h>

#include <functional>
#include <memory>

namespace fep3::native {

class ITaskExecutorInvoker {
public:
    virtual ~ITaskExecutorInvoker() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void timeUpdating(Timestamp new_time, std::optional<arya::Timestamp> next_time) = 0;
    virtual void timeReset(Timestamp old_time, Timestamp new_time) = 0;
    virtual fep3::Result addTask(std::function<void(fep3::Timestamp)> task,
                                 const std::string& name,
                                 Timestamp next_instant,
                                 Duration period,
                                 Duration initial_delay) = 0;
};

struct ISchedulerFactory {
    virtual ~ISchedulerFactory() = default;

    virtual std::unique_ptr<ITaskExecutorInvoker> createSchedulerProcessor(
        IThreadPoolExecutor& threaded_executor,
        fep3::arya::IClock::ClockType clock_type,
        std::function<fep3::Timestamp()> time_getter,
        std::shared_ptr<const fep3::ILogger> logger) const = 0;
};

} // namespace fep3::native
