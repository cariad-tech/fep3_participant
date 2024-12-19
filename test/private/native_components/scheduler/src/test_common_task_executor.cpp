/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/native_components/scheduler/clock_based/simulation_clock/synchronous_task_executor.h>
#include <fep3/native_components/scheduler/clock_based/system_clock/asynchronous_task_executor.h>

#include <boost/thread/barrier.hpp>
#include <boost/thread/latch.hpp>

#include <algorithm>
#include <common/gtest_asserts.h>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ::testing;
using namespace fep3::native;
using namespace fep3::mock;

template <typename Tuple>
struct TestTaskExecutor : public ::testing::Test {
    using ProcessorType = std::tuple_element_t<0, Tuple>;
    using ThreadPoolType = std::tuple_element_t<1, Tuple>;

    void SetUp() override
    {
        this->pool = std::make_unique<ThreadPoolType>(5);
        this->processor = std::make_unique<ProcessorType>(*(this->pool));
        this->pool->start();
    }

    void TearDown() override
    {
        this->pool.reset();
        this->processor.reset();
    }

    void startIfBlockingProcessor()
    {
        if constexpr (!std::is_same_v<ProcessorType, SyncTaskExecutor>) {
            this->processor->start();
        }
    }

    void run(const fep3::Timestamp current_time,
             std::optional<const fep3::Timestamp> next_time = std::nullopt)
    {
        if constexpr (!std::is_same_v<ProcessorType, SyncTaskExecutor>) {
            (void)next_time; // prevent gcc error: parameter next_time set but not used
                             // [-Werror=unused-but-set-parameter]
            this->processor->run(current_time);
        }
        else {
            this->processor->run(current_time, next_time);
        }
    }

    std::unique_ptr<ThreadPoolType> pool;
    std::unique_ptr<ProcessorType> processor;
};

template <typename Tuple>
struct TestThreadedTimerQueueProcessor : TestTaskExecutor<Tuple> {
};

using TestingTypes = ::testing::Types<std::tuple<SyncTaskExecutor, ThreadPoolExecutor>,
                                      std::tuple<AsyncTaskExecutor, MockThreadPoolExecutor>>;

using ThreadingTestingTypes = ::testing::Types<std::tuple<SyncTaskExecutor, ThreadPoolExecutor>,
                                               std::tuple<AsyncTaskExecutor, ThreadPoolExecutor>>;

TYPED_TEST_SUITE(TestTaskExecutor, TestingTypes);
TYPED_TEST_SUITE(TestThreadedTimerQueueProcessor, ThreadingTestingTypes);

TYPED_TEST(TestTaskExecutor, run_noTasks)
{
    this->startIfBlockingProcessor();
    this->run(0ns, 10ns);
}

TYPED_TEST(TestThreadedTimerQueueProcessor, run_Max5TasksExecutedInOwnThread)
{
    const size_t num_of_tasks = 5;
    std::vector<NiceMock<ScheduledTaskMock>> tasks{num_of_tasks};
    fep3::Timestamp next_instant = 0ms;
    fep3::Duration period = 10ms;
    fep3::Duration initial_delay = 10ms;

    boost::barrier _b(num_of_tasks);
    boost::latch _threads_ready(num_of_tasks);

    int i = 0;
    for (auto& task: tasks) {
        this->processor->addTask(
            [&](fep3::Timestamp time) {
                // barrier is important, otherwise a thread can already be ready
                _b.count_down_and_wait();
                task(time);
                _threads_ready.count_down();
            },
            "task" + std::to_string(i),
            next_instant,
            period,
            initial_delay);
        ++i;
    }

    this->startIfBlockingProcessor();
    this->run(0ns, 10ms);
    _threads_ready.wait();

    std::vector<std::thread::id> thread_ids;

    std::transform(tasks.begin(), tasks.end(), std::back_inserter(thread_ids), [](const auto& a) {
        return a._thread_id;
    });

    auto it = std::unique(thread_ids.begin(), thread_ids.end());

    thread_ids.erase(it, thread_ids.end());
    EXPECT_THAT(thread_ids, SizeIs(num_of_tasks));
}

TYPED_TEST(TestTaskExecutor, run_oneTask)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 100ns;
    fep3::Duration initial_delay = 0ns;
    ScheduledTaskMock task;

    this->processor->addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);
    this->startIfBlockingProcessor();
    {
        ::Sequence s;
        EXPECT_CALL(task, run(0ns));
        this->run(0ns, 50ns);
        this->run(50ns, 100ns);
        EXPECT_CALL(task, run(100ns));
        this->run(100ns, 150ns);
        this->run(150ns, 200ns);
    }
}

TYPED_TEST(TestTaskExecutor, run_twoTasksDifferentPeriods_OneTaskIsCalled)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period1 = 50ns;
    fep3::Duration period2 = 100ns;
    fep3::Duration initial_delay = 0ns;

    ScheduledTaskMock task1;
    ScheduledTaskMock task2;
    this->processor->addTask(
        [&](fep3::Timestamp time) { task1(time); }, "task1", next_instant, period1, initial_delay);
    this->processor->addTask(
        [&](fep3::Timestamp time) { task2(time); }, "task2", next_instant, period2, initial_delay);

    // NOTE: SyncTaskExecutor:
    // For this test case we don't use next time to wait for each task until it is completed.
    // Otherwise it could happen that task2 is still not processed and EXPECT_CALL(task2,
    // ..).Times(0) will fail.
    this->startIfBlockingProcessor();
    {
        EXPECT_CALL(task1, run(0ns));
        EXPECT_CALL(task2, run(0ns));
        this->run(0ns);
    }
    {
        EXPECT_CALL(task1, run(50ns));
        EXPECT_CALL(task2, run(_)).Times(0);
        this->run(50ns);
    }
    {
        EXPECT_CALL(task1, run(100ns));
        EXPECT_CALL(task2, run(100ns));
        this->run(100ns);
    }
}

TYPED_TEST(TestTaskExecutor, run_periodicAndOneshotTask)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 100ns;
    fep3::Duration initial_delay = 0ns;

    ScheduledTaskMock task;
    ScheduledTaskMock taskOneShot;
    this->processor->addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);
    this->processor->addTask([&](fep3::Timestamp time) { taskOneShot(time); },
                             "taskOneShot",
                             next_instant,
                             0ns,
                             initial_delay);
    this->startIfBlockingProcessor();

    {
        EXPECT_CALL(task, run(0ns));
        EXPECT_CALL(taskOneShot, run(0ns));
        this->run(0ns, 50ns);
        this->run(50ns, 100ns);
    }

    {
        EXPECT_CALL(task, run(100ns));
        this->run(100ns, 150ns);
        this->run(150ns, 200ns);
    }
}

// a bit extreme scenario, happens only if the first call to run is not 0ns
TYPED_TEST(TestTaskExecutor, run_periodicAndOneshotTask_oneShotCalledWithSubStepTimestamp)
{
    fep3::Timestamp next_instant = 25ns;
    fep3::Duration period = 100ns;
    fep3::Duration initial_delay = 0ns;

    ScheduledTaskMock task;
    ScheduledTaskMock taskOneShot;

    this->processor->addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);
    this->processor->addTask([&](fep3::Timestamp time) { taskOneShot(time); },
                             "taskOneShot",
                             next_instant,
                             0ns,
                             initial_delay);
    this->startIfBlockingProcessor();

    {
        EXPECT_CALL(task, run(25ns));
        EXPECT_CALL(taskOneShot, run(25ns));
        this->run(25ns, 125ns);
    }
}

// currently we do not know the clock tick, so we cannot postpone executing
// one shot task in the correct timestamp
TYPED_TEST(TestTaskExecutor, run_onlyOneshotTask_nextInstantIgnored)
{
    fep3::Timestamp next_instant = 100ns;
    fep3::Duration period = 0ns;
    fep3::Duration initial_delay = 0ns;

    ScheduledTaskMock taskOneShot;

    this->processor->addTask(
        [&](fep3::Timestamp time) { taskOneShot(time); }, "taskOneShot", next_instant, 0ns, 0ns);
    this->startIfBlockingProcessor();

    {
        EXPECT_CALL(taskOneShot, run(0ns));
        this->run(0ns, 50ns);
    }
    {
        this->run(50ns, 100ns);
        this->run(100ns, 150ns);
    }
}

TYPED_TEST(TestTaskExecutor, resetAfterRun_oneTask_calledWithOffsetTime)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 100ns;
    fep3::Timestamp reset_old_time = 0ns;
    fep3::Timestamp reset_new_time = 25ns;
    fep3::Duration initial_delay = 0ns;

    ScheduledTaskMock task;

    this->processor->addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);
    this->startIfBlockingProcessor();

    {
        EXPECT_CALL(task, run(0ns));
        this->run(0ns, 100ns);
    }
    this->processor->timeReset(reset_old_time, reset_new_time);
    // task will be scheduled for 125ns, and not 100ns
    {
        this->run(100ns, 200ns);
    }

    {
        EXPECT_CALL(task, run(125ns));
        this->run(125ns, 225ns);
    }
}

TYPED_TEST(TestTaskExecutor, resetTimeBackwards_oneTask_calledAfterReset)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 100ns;
    fep3::Timestamp reset_old_time = 100ns;
    fep3::Timestamp reset_new_time = 50ns;
    fep3::Duration initial_delay = 0ns;

    ScheduledTaskMock task;

    this->processor->addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);
    this->startIfBlockingProcessor();

    {
        ::Sequence s;
        EXPECT_CALL(task, run(0ns));
        this->run(0ns, 100ns);
        EXPECT_CALL(task, run(100ns));
        this->run(100ns, 200ns);
    }

    // task will be scheduled for 150ns, and not 200ns
    this->processor->timeReset(reset_old_time, reset_new_time);
    {
        EXPECT_CALL(task, run(150ns));
        this->run(150ns, 250ns);
    }
}

TYPED_TEST(TestTaskExecutor, resetNegativeTimeBackwards_oneTask_calledAfterReset)
{
    fep3::Timestamp next_instant = 0ns;
    fep3::Duration period = 50ns;
    fep3::Timestamp reset_old_time = 200ns;
    fep3::Timestamp reset_new_time = 100ns;
    fep3::Duration initial_delay = 0ns;

    ScheduledTaskMock task;

    this->processor->addTask(
        [&](fep3::Timestamp time) { task(time); }, "task", next_instant, period, initial_delay);
    this->startIfBlockingProcessor();

    {
        ::Sequence s;
        EXPECT_CALL(task, run(0ns));
        this->run(0ns, 50ns);
    }

    // task will be scheduled for 100ns, and not 50ns
    this->processor->timeReset(200ns, 100ns);
    {
        EXPECT_CALL(task, run(100ns));
        this->run(100ns, 150ns);
    }
}
