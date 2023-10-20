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

#include "threaded_executor.h"

#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/native_components/scheduler/clock_based/system_clock/asynchronous_task_executor.h>

#include <boost/thread/latch.hpp>
#include <gtest/gtest.h>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ::testing;
using namespace fep3::native;
using namespace fep3::mock;

TEST(TestAsyncronousTimerQueueProcessor, run_DoesNotBlockOnTaskCompletion)
{
    const size_t num_of_tasks = 5;
    std::array<bool, num_of_tasks> tasks_ready{false};

    fep3::Timestamp next_instant = 0ms;
    fep3::Duration period = 10ms;
    fep3::Duration initial_delay = 0ms;

    ThreadPoolExecutor thread_pool(num_of_tasks);
    AsyncTaskExecutor processor(thread_pool);
    thread_pool.start();

    boost::barrier barrier(num_of_tasks + 1);
    boost::latch latch(num_of_tasks);

    for (auto i = 0; i < 5; ++i) {
        processor.addTask(
            [&, i](fep3::Timestamp) {
                // barrier is important, otherwise a thread can already be ready
                barrier.count_down_and_wait();
                tasks_ready[i] = true;
                latch.count_down();
            },
            "task" + std::to_string(i),
            next_instant,
            period,
            initial_delay);
    }

    processor.start();
    processor.run(0ns);
    EXPECT_THAT(tasks_ready, Each(Eq(false)));
    // the jobs can start working
    barrier.count_down_and_wait();
    // wait for all work, otherwise the barrier gets deleted and the threads hang
    // we can fix this by the order of detruction, but latch is the safer way
    latch.wait();
}

TEST(TestAsyncronousTimerQueueProcessor, stop_stopsFurtherScheduling)
{
    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool);

    fep3::Timestamp next_instant = 0ms;
    fep3::Duration period = 10ms;
    fep3::Duration initial_delay = 0ms;
    uint8_t times_executed{0};
    boost::latch latch(1);

    processor.addTask(
        [&](fep3::Timestamp) {
            // barrier is important, otherwise a thread can already be ready
            processor.stop();
            ++times_executed;
            latch.count_down();
        },
        "task",
        next_instant,
        period,
        initial_delay);

    processor.start();
    processor.run(0ns);
    latch.wait();
    ASSERT_EQ(times_executed, 1);
}

// even if a task could be run twice, it will run once
TEST(TestAsyncronousTimerQueueProcessor, run_laggingTaskRunsOnce)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 50ns;
    fep3::Duration initial_delay = 0ms;
    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool);

    ScheduledTaskMock task;

    processor.addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);

    EXPECT_CALL(task, run(0ns));
    processor.start();
    processor.run(0ns);

    {
        ::Sequence s;
        EXPECT_CALL(task, run(100ns));
        processor.run(100ns);
    }
}

TEST(TestAsyncronousTimerQueueProcessor, run_laggingTaskNextInstantIsAfterCurrentTime_1)
{
    fep3::Timestamp next_instant = 10ns;
    fep3::Duration period = 30ns;
    fep3::Duration initial_delay = 0ns;
    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool);

    ScheduledTaskMock task;

    processor.addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);

    {
        EXPECT_CALL(task, run(100ns));

        processor.start();
        processor.run(100ns);
    }
    {
        EXPECT_CALL(task, run(_)).Times(0);
        processor.run(101ns);
    }
    {
        EXPECT_CALL(task, run(_)).Times(0);
        processor.run(110ns);
    }
}

TEST(TestAsyncronousTimerQueueProcessor, run_laggingTaskNextInstantIsAfterCurrentTime_2)
{
    fep3::Timestamp next_instant = 50ns;
    fep3::Duration period = 25ns;
    fep3::Duration initial_delay = 0ns;

    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool);

    ScheduledTaskMock task;

    processor.addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);

    {
        EXPECT_CALL(task, run(130ns));

        processor.start();
        processor.run(130ns);
    }
    {
        EXPECT_CALL(task, run(_)).Times(0);
        processor.run(130ns);
    }
    {
        EXPECT_CALL(task, run(_));
        processor.run(150ns);
    }
}
// CHECK THAT TASKS ARE CALLED WITH THE ACTUAL CLOCK TIME AND NOT THE NEXT INSTANT TIME
TEST(TestAsyncronousTimerQueueProcessor, run_taskCalledWithRunTimeAndNotNextInstant)
{
    fep3::Timestamp next_instant = 50ns;
    fep3::Duration period = 25ns;
    fep3::Duration initial_delay = 0ns;
    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool);

    ScheduledTaskMock task;

    processor.addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);

    {
        Sequence s;
        EXPECT_CALL(task, run(130ns));
        EXPECT_CALL(task, run(199ns));
        EXPECT_CALL(task, run(225ns));

        processor.start();
        processor.run(130ns);
        processor.run(199ns);
        processor.run(225ns);
    }
}

TEST(TestAsyncronousTimerQueueProcessor, startRuntopResetRun)
{
    fep3::Timestamp next_instant = 50ns;
    fep3::Duration period = 25ns;
    fep3::Duration initial_delay = 0ns;
    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool);

    ScheduledTaskMock task;

    processor.addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);

    {
        Sequence s;
        EXPECT_CALL(task, run(130ns));
        EXPECT_CALL(task, run(199ns));
        EXPECT_CALL(task, run(225ns));

        processor.start();
        processor.run(130ns);
        processor.run(199ns);
        processor.run(225ns);
    }

    {
        EXPECT_CALL(task, run(_)).Times(0);
        processor.stop();
        processor.run(250ns);
    }

    EXPECT_CALL(task, run(130ns));
    processor.timeReset(250ns, 0ns);
    processor.start();
    processor.run(130ns);
}

TEST(TestAsyncronousTimerQueueProcessor, run_doesNotReExecuteUnfinishedTask)
{
    auto logger = std::make_shared<NiceMock<fep3::mock::LoggerWithDefaultBehavior>>();
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 25ns;
    fep3::Duration initial_delay = 0ns;
    ThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool, logger);
    pool.start();

    ScheduledTaskMock task;
    boost::barrier barrier(2);

    processor.addTask(
        [&](fep3::Timestamp time) {
            task(time);
            barrier.count_down_and_wait();
        },
        "task",
        next_instant,
        period,
        initial_delay);

    // assert that the info is given out
    EXPECT_CALL(*logger, logDebug(_)).Times(AnyNumber());
    EXPECT_CALL(task, run(0ns)).Times(1);
    EXPECT_CALL(task, run(25ns)).Times(0);

    processor.start();
    processor.run(0ns);
    processor.run(25ns);
    barrier.count_down_and_wait();
    processor.stop();
}

TEST(TestAsyncronousTimerQueueProcessor, run_DoesNotScheduleWithoutStart)
{
    auto logger = std::make_shared<NiceMock<fep3::mock::LoggerWithDefaultBehavior>>();
    fep3::Timestamp next_instant = 50ns;
    fep3::Duration period = 25ns;
    fep3::Duration initial_delay = 0ns;
    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool, logger);

    ScheduledTaskMock task;

    processor.addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);

    {
        // assert that the info is given out
        EXPECT_CALL(*logger, logDebug(_)).Times(AnyNumber());
        EXPECT_CALL(task, run(130ns)).Times(0);

        processor.run(130ns);
    }
}

TEST(TestAsyncronousTimerQueueProcessor, run_WaitTimePlusCurrentTimeIsMultipleOfPeriod_1)
{
    fep3::Timestamp next_instant = 50ns;
    fep3::Duration period = 25ns;
    fep3::Duration initial_delay = 0ns;
    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool);

    processor.start();
    processor.addTask([&](fep3::Timestamp) {}, "task", next_instant, period, initial_delay);

    ASSERT_EQ(processor.run(130ns), 20ns)
        << "Next execution time is 150ns, the scheduler should wait 20ns";
}

TEST(TestAsyncronousTimerQueueProcessor, run_WaitTimePlusCurrentTimeIsMultipleOfPeriod_2)
{
    fep3::Timestamp next_instant = 100ns;
    fep3::Duration period = 10ns;
    fep3::Duration initial_delay = 0ns;
    MockThreadPoolExecutor pool;
    AsyncTaskExecutor processor(pool);

    processor.addTask([&](fep3::Timestamp) {}, "task", next_instant, period, initial_delay);

    processor.start();
    ASSERT_EQ(processor.run(130ns), 10ns)
        << "Next execution time is 150ns, the scheduler should wait 10ns";
}
