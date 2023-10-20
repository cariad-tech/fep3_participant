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
#include "scheduler_factory.h"

#include "simulation_clock/synchronous_task_executor.h"
#include "simulation_clock/synchronous_task_executor_invoker.h"
#include "system_clock/asynchronous_task_executor.h"
#include "system_clock/asynchronous_task_executor_invoker.h"

namespace fep3::native {

std::unique_ptr<ITaskExecutorInvoker> SchedulerFactory::createSchedulerProcessor(
    IThreadPoolExecutor& threaded_executor,
    fep3::arya::IClock::ClockType clock_type,
    std::function<fep3::Timestamp()> time_getter,
    std::shared_ptr<const fep3::ILogger> logger) const
{
    using AsyncTaskExecutionWithThreadPool = AsyncTaskExecutor;
    using AsyncContinuousClockScheduler =
        AsyncTaskExecutorInvoker<NotificationWaiting, AsyncTaskExecutionWithThreadPool>;
    using SyncTaskExecutorInvokerType = SyncTaskExecutorInvoker<SyncTaskExecutor>;

    switch (clock_type) {
    case (fep3::arya::IClock::ClockType::discrete):
        return std::make_unique<SyncTaskExecutorInvokerType>(
            [&]() { return SyncTaskExecutor(threaded_executor); });

    case (fep3::arya::IClock::ClockType::continuous):
        return std::make_unique<AsyncContinuousClockScheduler>(
            std::move(time_getter),
            [&, logger]() { return AsyncTaskExecutionWithThreadPool(threaded_executor, logger); },
            logger);
    }
    return nullptr;
}

} // namespace fep3::native
