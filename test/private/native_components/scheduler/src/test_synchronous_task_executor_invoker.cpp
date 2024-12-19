/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "threaded_executor.h"

#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/native_components/scheduler/clock_based/scheduler_factory.h>
#include <fep3/native_components/scheduler/clock_based/simulation_clock/synchronous_task_executor_invoker.h>

#include <boost/thread/barrier.hpp>
#include <boost/thread/latch.hpp>

#include <algorithm>
#include <common/gtest_asserts.h>
#include <future>
#include <helper/job_registry_helper.h>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ::testing;
using namespace fep3::native;
using namespace fep3::mock;

struct TestSyncTaskExecutorInvoker : public ::testing::Test {
    void SetUp() override
    {
        SchedulerFactory factory;

        thread_pool = std::make_unique<ThreadPoolExecutor>(5);
        thread_pool->start();

        blocking_scheduler = factory.createSchedulerProcessor(
            *thread_pool,
            fep3::arya::IClock::ClockType::discrete,
            []() { return fep3::Timestamp(0); },
            nullptr);
    }

    void TearDown() override
    {
        thread_pool.reset();
    }

    std::unique_ptr<ITaskExecutorInvoker> blocking_scheduler;
    std::unique_ptr<ThreadPoolExecutor> thread_pool;
};

TEST_F(TestSyncTaskExecutorInvoker,
       timeUpdating_startThenTimeResetThenTimeUpdate_processorRunCalled)
{
    fep3::Duration task_period = 10ns;
    fep3::Duration initial_delay = 0ns;

    ScheduledTaskMock mock_task;
    blocking_scheduler->addTask(
        [&](fep3::Timestamp time) { mock_task(time); }, "task", 0ns, task_period, initial_delay);

    EXPECT_CALL(mock_task, run(0ns)).Times(1);
    EXPECT_CALL(mock_task, run(10ns)).Times(1);
    EXPECT_CALL(mock_task, run(20ns)).Times(1);

    blocking_scheduler->start();
    blocking_scheduler->timeReset(0ns, 0ns);
    blocking_scheduler->timeUpdating(10ns, 10ns);
    blocking_scheduler->timeUpdating(20ns, 30ns);
}

TEST_F(TestSyncTaskExecutorInvoker, timeUpdating_pastTimestamp_processorRunNotCalled)
{
    fep3::Duration task_period = 10ns;
    fep3::Duration initial_delay = 0ns;
    ScheduledTaskMock mock_task;
    blocking_scheduler->addTask(
        [&](fep3::Timestamp time) { mock_task(time); }, "task", 0ns, task_period, initial_delay);
    {
        EXPECT_CALL(mock_task, run(0ns)).Times(1);
        EXPECT_CALL(mock_task, run(10ns)).Times(1);
        EXPECT_CALL(mock_task, run(20ns)).Times(1);

        blocking_scheduler->start();
        blocking_scheduler->timeReset(0ns, 0ns);

        blocking_scheduler->timeUpdating(20ns, 30ns);
    }
    blocking_scheduler->timeUpdating(10ns, 20ns);
}

TEST_F(TestSyncTaskExecutorInvoker, timeUpdating_restart_processorRunCalled)
{
    fep3::Duration task_period = 10ns;
    fep3::Duration initial_delay = 0ms;
    ScheduledTaskMock mock_task;
    blocking_scheduler->addTask(
        [&](fep3::Timestamp time) { mock_task(time); }, "task", 0ns, task_period, initial_delay);
    {
        EXPECT_CALL(mock_task, run(0ns)).Times(1);
        EXPECT_CALL(mock_task, run(10ns)).Times(1);
        EXPECT_CALL(mock_task, run(20ns)).Times(1);

        blocking_scheduler->start();
        blocking_scheduler->timeReset(0ns, 0ns);
        blocking_scheduler->timeUpdating(0ns, 10ns);
        blocking_scheduler->timeUpdating(10ns, 20ns);
        blocking_scheduler->timeUpdating(20ns, 30ns);
        blocking_scheduler->stop();
    }
    {
        EXPECT_CALL(mock_task, run(0ns)).Times(1);
        EXPECT_CALL(mock_task, run(10ns)).Times(1);
        EXPECT_CALL(mock_task, run(20ns)).Times(1);

        blocking_scheduler->start();
        blocking_scheduler->timeReset(20ns, 0ns);
        blocking_scheduler->timeUpdating(10ns, 20ns);
        blocking_scheduler->timeUpdating(20ns, 30ns);
    }
}

TEST_F(TestSyncTaskExecutorInvoker, timeUpdating_restart_pastTimestamp_processorRunNotCalled)
{
    fep3::Duration task_period = 10ns;
    fep3::Duration initial_delay = 0ms;
    ScheduledTaskMock mock_task;
    blocking_scheduler->addTask(
        [&](fep3::Timestamp time) { mock_task(time); }, "task", 0ns, task_period, initial_delay);
    {
        EXPECT_CALL(mock_task, run(0ns)).Times(1);
        EXPECT_CALL(mock_task, run(10ns)).Times(1);
        EXPECT_CALL(mock_task, run(20ns)).Times(1);

        blocking_scheduler->start();
        blocking_scheduler->timeReset(0ns, 0ns);
        blocking_scheduler->timeUpdating(20ns, 30ns);
        blocking_scheduler->stop();
    }
    {
        EXPECT_CALL(mock_task, run(0ns)).Times(1);
        EXPECT_CALL(mock_task, run(10ns)).Times(1);
        EXPECT_CALL(mock_task, run(20ns)).Times(1);

        blocking_scheduler->start();
        blocking_scheduler->timeReset(20ns, 0ns);
        blocking_scheduler->timeUpdating(20ns, 30ns);
    }
    blocking_scheduler->timeUpdating(10ns, 20ns);
}

TEST_F(TestSyncTaskExecutorInvoker, timeUpdating_stopWaitsForTimeUpdatingToFinish)
{
    fep3::Duration task_period = 10ns;
    fep3::Duration initial_delay = 0ms;
    ScheduledTaskMock mock_task;
    blocking_scheduler->addTask(
        [&](fep3::Timestamp time) { mock_task(time); }, "task", 0ns, task_period, initial_delay);

    boost::barrier barrier(2);
    std::atomic<bool> _stopFinished{false};

    EXPECT_CALL(mock_task, run(0ns)).Times(1);
    EXPECT_CALL(mock_task, run(10ns)).Times(1);
    EXPECT_CALL(mock_task, run(20ns)).WillOnce(InvokeWithoutArgs([&]() {
        barrier.count_down_and_wait();
        ASSERT_FALSE(_stopFinished) << "Stop should not return until timeUpdating is in progress";
    }));

    blocking_scheduler->start();
    blocking_scheduler->timeReset(0ns, 0ns);

    auto future = std::async([&]() {
        barrier.count_down_and_wait();
        blocking_scheduler->stop();
        _stopFinished = true;
    });

    blocking_scheduler->timeUpdating(20ns, 10ns);

    future.get();
}

TEST_F(TestSyncTaskExecutorInvoker, timeUpdating_noTaskScheduledBeforeStart)
{
    fep3::Duration task_period = 10ns;
    fep3::Duration initial_delay = 0ms;
    ScheduledTaskMock mock_task;
    blocking_scheduler->addTask(
        [&](fep3::Timestamp time) { mock_task(time); }, "task", 0ns, task_period, initial_delay);

    EXPECT_CALL(mock_task, run(0ns)).Times(0);

    blocking_scheduler->timeReset(0ns, 0ns);
    blocking_scheduler->timeUpdating(20ns, 30ns);
}

TEST_F(TestSyncTaskExecutorInvoker, timeUpdating_noTaskScheduledAfterStop)
{
    fep3::Duration task_period = 10ns;
    fep3::Duration initial_delay = 0ms;
    ScheduledTaskMock mock_task;
    blocking_scheduler->addTask(
        [&](fep3::Timestamp time) { mock_task(time); }, "task", 0ns, task_period, initial_delay);

    EXPECT_CALL(mock_task, run(0ns)).Times(1);
    EXPECT_CALL(mock_task, run(10ns)).Times(0);

    blocking_scheduler->start();
    blocking_scheduler->timeReset(0ns, 0ns);
    blocking_scheduler->timeUpdating(0ns, 10ns);
    blocking_scheduler->stop();
    blocking_scheduler->timeUpdating(10ns, 20ns);
}
