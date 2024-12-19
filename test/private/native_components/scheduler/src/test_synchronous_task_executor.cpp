/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/native_components/scheduler/clock_based/simulation_clock/synchronous_task_executor.h>

#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ::testing;
using namespace fep3::native;
using namespace fep3::mock;

TEST(TestSyncTaskExecutor, run_TasksBlockUntilCompletion)
{
    const size_t num_of_tasks = 5;
    std::array<bool, num_of_tasks> tasks_ready{false};

    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 5ns;
    fep3::Duration initial_delay = 0ns;
    ThreadPoolExecutor pool(5);
    SyncTaskExecutor executor(pool);
    pool.start();

    boost::barrier _b(num_of_tasks);
    std::atomic<bool> run_returned{false};

    for (auto i = 0; i < 5; ++i) {
        executor.addTask(
            [&, i](fep3::Timestamp) {
                // barrier is important, otherwise a thread can already be ready
                _b.count_down_and_wait();
                tasks_ready[i] = true;
                ASSERT_FALSE(run_returned)
                    << "run method cannot return while a task is in progress";
            },
            "task" + std::to_string(i),
            next_instant,
            period,
            initial_delay);
    }

    std::future<void> run_finished = std::async(std::launch::async, [&]() {
        // period of all tasks is equal to the next simulation time step --> all tasks will block
        // the executor
        executor.run(0ns, 5ns);
        run_returned = true;
        EXPECT_THAT(tasks_ready, Each(Eq(true)));
    });

    run_finished.get();
}

TEST(TestSyncTaskExecutor, run_TasksBlockUntilCompletion_CurrentTimeEqualToNextTime)
{
    const size_t num_of_tasks = 5;
    std::array<bool, num_of_tasks> tasks_ready{false};

    fep3::Timestamp next_instant = 5ns;
    fep3::Duration period = 5ns;
    fep3::Duration initial_delay = 0ms;
    ThreadPoolExecutor pool(num_of_tasks);
    SyncTaskExecutor executor(pool);
    pool.start();

    boost::barrier _b(num_of_tasks);
    std::atomic<bool> run_returned{false};

    for (auto i = 0; i < 5; ++i) {
        executor.addTask(
            [&, i](fep3::Timestamp) {
                // barrier is important, otherwise a thread can already be ready
                _b.count_down_and_wait();
                tasks_ready[i] = true;
                ASSERT_FALSE(run_returned)
                    << "run method cannot return while a task is in progress";
            },
            "task" + std::to_string(i),
            next_instant,
            period,
            initial_delay);
    }

    std::future<void> run_finished = std::async(std::launch::async, [&]() {
        // current time is equal to next time --> all tasks will block the executor
        executor.run(5ns, 5ns);
        run_returned = true;
        EXPECT_THAT(tasks_ready, Each(Eq(true)));
    });

    run_finished.get();
}

TEST(TestSyncTaskExecutor, run_TasksBlockUntilCompletion_CurrentTimeGreaterThanNextTime)
{
    const size_t num_of_tasks = 5;
    std::array<bool, num_of_tasks> tasks_ready{false};

    fep3::Timestamp next_instant = 5ns;
    fep3::Duration period = 5ns;
    fep3::Duration initial_delay = 0ns;
    ThreadPoolExecutor pool(5);
    SyncTaskExecutor executor(pool);
    pool.start();

    boost::barrier _b(num_of_tasks);
    std::atomic<bool> run_returned{false};

    for (auto i = 0; i < 5; ++i) {
        executor.addTask(
            [&, i](fep3::Timestamp) {
                // barrier is important, otherwise a thread can already be ready
                _b.count_down_and_wait();
                tasks_ready[i] = true;
                ASSERT_FALSE(run_returned)
                    << "run method cannot return while a task is in progress";
            },
            "task" + std::to_string(i),
            next_instant,
            period,
            initial_delay);
    }

    std::future<void> run_finished = std::async(std::launch::async, [&]() {
        // current time is greater than next time --> all tasks will block the executor
        executor.run(5ns, 0ns);
        run_returned = true;
        EXPECT_THAT(tasks_ready, Each(Eq(true)));
    });

    run_finished.get();
}

TEST(TestSyncTaskExecutor, run_TasksBlockUntilCompletion_NegativeNextTime)
{
    const size_t num_of_tasks = 5;
    std::array<bool, num_of_tasks> tasks_ready{false};

    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 5ns;
    fep3::Duration initial_delay = 0ns;
    ThreadPoolExecutor pool(num_of_tasks);
    pool.start();

    SyncTaskExecutor executor(pool);
    boost::barrier _b(num_of_tasks);
    std::atomic<bool> run_returned{false};

    for (auto i = 0; i < 5; ++i) {
        executor.addTask(
            [&, i](fep3::Timestamp) {
                // barrier is important, otherwise a thread can already be ready
                _b.count_down_and_wait();
                tasks_ready[i] = true;
                ASSERT_FALSE(run_returned)
                    << "run method cannot return while a task is in progress";
            },
            "task" + std::to_string(i),
            next_instant,
            period,
            initial_delay);
    }

    std::future<void> run_finished = std::async(std::launch::async, [&]() {
        // current time is greater than next time --> all tasks will block the executor
        executor.run(0ns, fep3::Timestamp{-1});
        run_returned = true;
        EXPECT_THAT(tasks_ready, Each(Eq(true)));
    });

    run_finished.get();
}

TEST(TestSyncTaskExecutor, run_TasksBlockUntilCompletion_NoNextTime)
{
    const size_t num_of_tasks = 5;
    std::array<bool, num_of_tasks> tasks_ready{false};

    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 5ns;
    fep3::Duration initial_delay = 0ns;
    ThreadPoolExecutor pool(num_of_tasks);
    pool.start();

    SyncTaskExecutor executor(pool);
    boost::barrier _b(num_of_tasks);
    std::atomic<bool> run_returned{false};

    for (auto i = 0; i < 5; ++i) {
        executor.addTask(
            [&, i](fep3::Timestamp) {
                // barrier is important, otherwise a thread can already be ready
                _b.count_down_and_wait();
                tasks_ready[i] = true;
                ASSERT_FALSE(run_returned)
                    << "run method cannot return while a task is in progress";
            },
            "task" + std::to_string(i),
            next_instant,
            period,
            initial_delay);
    }

    std::future<void> run_finished = std::async(std::launch::async, [&]() {
        // current time is greater than next time --> all tasks will block the executor
        executor.run(0ns);
        run_returned = true;
        EXPECT_THAT(tasks_ready, Each(Eq(true)));
    });

    run_finished.get();
}

TEST(TestSyncTaskExecutor, run_TaskFinishedWithinPeriod)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period1 = 5ns;
    fep3::Duration period2 = 20ns;
    fep3::Duration initial_delay = 0ns;
    boost::latch _threads_ready(4);

    ThreadPoolExecutor pool(2);
    SyncTaskExecutor executor(pool);
    pool.start();

    ScheduledTaskMock task1;
    ScheduledTaskMock task2;

    bool task2_ready = false;

    executor.addTask(
        [&](fep3::Timestamp time) {
            task1(time);
            _threads_ready.count_down();
        },
        "task1",
        next_instant,
        period1,
        initial_delay);

    executor.addTask(
        [&](fep3::Timestamp time) {
            task2(time);
            _threads_ready.wait();
            task2_ready = true;
        },
        "task2",
        next_instant,
        period2,
        initial_delay);

    {
        ::Sequence s;
        // run task1 and wait for completeness , run task2 (block task2 thread; executor is not
        // blocked)
        EXPECT_CALL(task1, run(0ns));
        EXPECT_CALL(task2, run(0ns));
        executor.run(0ns, 5ns);
        EXPECT_EQ(task2_ready, false);

        // run task1 and wait for completeness, task2 thread is still blocked (executor is not
        // blocked)
        EXPECT_CALL(task1, run(5ns));
        executor.run(5ns, 10ns);
        EXPECT_EQ(task2_ready, false);

        // run task1 and wait for completeness, task2 thread is still blocked (executor is not
        // blocked)
        EXPECT_CALL(task1, run(10ns));
        executor.run(10ns, 15ns);
        EXPECT_EQ(task2_ready, false);

        // run task1 and wait for completeness -> release task2 thread
        EXPECT_CALL(task1, run(15ns));
        executor.run(15ns, 20ns);
        EXPECT_EQ(task2_ready, true);
    }
}

TEST(TestSyncTaskExecutor, run_periodicTaskperiod_runTwice)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 50ns;
    fep3::Duration initial_delay = 0ms;
    ThreadPoolExecutor pool;
    SyncTaskExecutor executor(pool);
    pool.start();

    ScheduledTaskMock task;

    executor.addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);

    EXPECT_CALL(task, run(0ns));
    // run task and wait for its completion
    executor.run(0ns, 50ns);

    {
        ::Sequence s;
        EXPECT_CALL(task, run(50ns));
        EXPECT_CALL(task, run(100ns));
        // run task twice and wait for its completion
        executor.run(100ns, 150ns);
    }
}

TEST(TestSyncTaskExecutor, run_taskCalledWithNextInstanceTime)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 20ns;
    fep3::Duration initial_delay = 0ms;
    ThreadPoolExecutor pool;
    SyncTaskExecutor executor(pool);
    pool.start();

    ScheduledTaskMock task;

    executor.addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);

    EXPECT_CALL(task, run(0ns));
    // run task twice and wait for its completion
    executor.run(0ns, 28ns);

    {
        ::Sequence s;
        EXPECT_CALL(task, run(20ns));
        // run task twice and wait for its completion
        executor.run(28ns, 53ns);

        EXPECT_CALL(task, run(40ns));
        // run task twice and wait for its completion
        executor.run(53ns, 82ns);
    }
}
