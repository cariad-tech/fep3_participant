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

#include "fep3/native_components/scheduler/clock_based/clock_based_scheduler.h"
#include "fep3/native_components/scheduler/clock_based/data_triggered_receiver.h"
#include "fep3/native_components/scheduler/clock_based/scheduler_factory.h"
#include "fep3/native_components/scheduler/clock_based/simulation_clock/synchronous_task_executor.h"
#include "fep3/native_components/scheduler/clock_based/simulation_clock/synchronous_task_executor_invoker.h"
#include "fep3/native_components/scheduler/clock_based/system_clock/asynchronous_task_executor.h"
#include "fep3/native_components/scheduler/clock_based/system_clock/asynchronous_task_executor_invoker.h"
#include "threaded_executor.h"

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/base/sample/data_sample.h>
#include <fep3/components/clock/mock/mock_clock_service_addons.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/components/data_registry/mock_data_registry.h>
#include <fep3/components/health_service/mock_health_service.h>
#include <fep3/components/job_registry/mock_job_registry.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/core/mock/mock_core.h>

#include <common/gtest_asserts.h>
#include <helper/gmock_async_helper.h>
#include <helper/job_registry_helper.h>

using namespace ::testing;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace fep3::test;
using namespace fep3;
using namespace fep3::native;

#include <boost/thread/latch.hpp>

// this factory uses a mock notification waiting and a mock thread pool
// otherwise job completion in the thread pool and waiting times are not deterministic
// AsyncTaskExecutor unit test also perform tests with the "production" thread pool
struct TestSchedulerFactory : public ISchedulerFactory {
    TestSchedulerFactory(IThreadPoolExecutor& mock_pool) : _mock_pool(mock_pool)
    {
    }
    using AsyncTaskExecutorTestType = fep3::native::AsyncTaskExecutor;
    using NotificationMock = NiceMock<fep3::mock::NotificationWaitingWithMockTimeout<false>>;
    // this is the type used in production with the exception that the thread pool executes the
    // tasks on post() and waiting on timeout just continues
    using TestAsyncTaskExecutorInvoker =
        fep3::native::AsyncTaskExecutorInvoker<NotificationMock, AsyncTaskExecutorTestType>;
    // this is the type used in production
    using TestSyncTaskExecutorInvoker =
        fep3::native::SyncTaskExecutorInvoker<fep3::native::SyncTaskExecutor>;

    std::unique_ptr<fep3::native::ITaskExecutorInvoker> createSchedulerProcessor(
        IThreadPoolExecutor& threaded_executor,
        fep3::arya::IClock::ClockType clock_type,
        std::function<fep3::Timestamp()> time_getter,
        std::shared_ptr<const fep3::ILogger> logger) const override
    {
        switch (clock_type) {
        case (fep3::arya::IClock::ClockType::discrete):
            return std::make_unique<TestSyncTaskExecutorInvoker>(
                [&]() { return fep3::native::SyncTaskExecutor(threaded_executor); });

        case (fep3::arya::IClock::ClockType::continuous):

            return std::make_unique<TestAsyncTaskExecutorInvoker>(
                std::move(time_getter),
                [&, logger]() { return AsyncTaskExecutorTestType(_mock_pool, logger); });
        }
        return nullptr;
    }

private:
    IThreadPoolExecutor& _mock_pool;
};

struct ClockBasedSchedulerTest : public ::testing::Test {
    void SetUp() override
    {
        EXPECT_CALL(_clock_service, registerEventSink(_))
            .WillRepeatedly(
                Invoke([this](const std::weak_ptr<fep3::arya::IClock::IEventSink>& event_sink) {
                    scheduler_event_sink = event_sink;
                    return fep3::Result{};
                }));

        EXPECT_CALL(*_logger, isDebugEnabled()).WillRepeatedly(Return(true));
        EXPECT_CALL(*_logger, logDebug(_)).WillRepeatedly(Return(a_util::result::Result()));
    }

    void setUpClock(fep3::Duration clock_period)
    {
        _clock_period = clock_period;

        ON_CALL(_clock_service, getType()).WillByDefault(Return(_clock_type));

        ON_CALL(_clock_service, getTime()).WillByDefault((InvokeWithoutArgs([&]() {
            std::lock_guard<std::mutex> lock(_max_sim_time_mutex);

            auto clock_time = _clock_time;
            if (_clock_time < _max_simulation_time) {
                _clock_time = _clock_time + _clock_period;
            }
            return clock_time;
        })));
    }

    auto createClockTriggeredJob(const fep3::Duration job_cycle_time,
                                 fep3::Duration delay_time = 0ns)
    {
        const helper::SimpleJobBuilder builder("my_job", job_cycle_time, delay_time);

        auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
        my_job->setDefaultBehaviour();

        _jobs =
            fep3::arya::Jobs{{builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}}};
        return my_job;
    }

    void setClockTime(fep3::Timestamp clock_time)
    {
        std::lock_guard<std::mutex> lock(_max_sim_time_mutex);
        _clock_time = clock_time;
    }
    void setMaxSimTime(fep3::Duration max_simulation_time)
    {
        std::lock_guard<std::mutex> lock(_max_sim_time_mutex);
        _max_simulation_time = max_simulation_time;
    }

    void wait_for_simulation_end()
    {
        if (_clock_type == fep3::arya::IClock::ClockType::continuous) {
            simulation_end.wait();
        }
        else {
            bool run_loop = true;
            while (run_loop) {
                auto time_update = _clock_service.getTime();
                // clock did not increment, means simulation end reached
                if (time_update == _clock_time)
                    run_loop = false;
                scheduler_event_sink.lock()->timeUpdating(time_update);
            }
        }
    }

    std::weak_ptr<fep3::arya::IClock::IEventSink> scheduler_event_sink;
    NiceMock<fep3::mock::arya::ClockService> _clock_service;
    fep3::Timestamp _clock_time{0};
    fep3::Duration _clock_period{10ns};
    fep3::arya::Jobs _jobs;
    std::shared_ptr<fep3::mock::MockThreadPoolExecutor> _mock_pool =
        std::make_shared<fep3::mock::MockThreadPoolExecutor>();
    std::shared_ptr<TestSchedulerFactory> _factory =
        std::make_shared<TestSchedulerFactory>(*_mock_pool);
    std::shared_ptr<fep3::mock::LoggerWithDefaultBehavior> _logger =
        std::make_shared<fep3::mock::LoggerWithDefaultBehavior>();

    fep3::Duration _max_simulation_time;
    std::mutex _max_sim_time_mutex;
    boost::latch simulation_end{1};
    fep3::arya::IClock::ClockType _clock_type = fep3::arya::IClock::ClockType::continuous;
};

struct ClockBasedSchedulerCommonTest
    : public ClockBasedSchedulerTest,
      public ::testing::WithParamInterface<fep3::arya::IClock::ClockType> {
    void SetUp() override
    {
        _clock_type = GetParam();
        ClockBasedSchedulerTest::SetUp();
    }
};

struct ContinuousClockSchedulerTest : public ClockBasedSchedulerTest {
    void SetUp() override
    {
        _clock_type = fep3::arya::IClock::ClockType::continuous;
        ClockBasedSchedulerTest::SetUp();
    }
};

struct DiscreteClockSchedulerTest : public ClockBasedSchedulerTest {
    void SetUp() override
    {
        _clock_type = fep3::arya::IClock::ClockType::discrete;
        ClockBasedSchedulerTest::SetUp();
    }
};

INSTANTIATE_TEST_SUITE_P(CommonSchedulerTests,
                         ClockBasedSchedulerCommonTest,
                         ::testing::Values(fep3::arya::IClock::ClockType::continuous,
                                           fep3::arya::IClock::ClockType::discrete),
                         [](const auto& param) -> std::string {
                             if (param.param == fep3::arya::IClock::ClockType::continuous)
                                 return "ContinuousScheduler";
                             else
                                 return "DiscreteSchduler";
                         });

/**
 * @brief Test Scheduler without jobs
 */
TEST_P(ClockBasedSchedulerCommonTest, EmptyJobs)
{
    fep3::native::ClockBasedScheduler scheduler(_logger);
    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(scheduler.stop(), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(scheduler.deinitialize(), fep3::ERR_NOERROR);
}

// * @brief A scheduler is executed for 50ms with a job cycle time of 10ms. Job has to be called 6
// * times.
// * @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2286, FEPSDK-2468
// */
TEST_P(ClockBasedSchedulerCommonTest, Scheduling)
{
    const auto job_cycle_time = milliseconds(10);
    const auto clock_period = 10ms;
    _max_simulation_time = milliseconds(50);

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    ::testing::Sequence s;

    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = 0ns;
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

    EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(30ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));

    wait_for_simulation_end();
    scheduler.stop();
}

TEST_P(ClockBasedSchedulerCommonTest, SchedulingWithRestart)
{
    const auto job_cycle_time = milliseconds(10);
    const auto clock_period = 10ms;
    _max_simulation_time = milliseconds(50);

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);
    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = 0ns;
    {
        ::testing::Sequence s;
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(30ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms)))
            .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                            Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));

        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);
        wait_for_simulation_end();
        scheduler.stop();
    }

    // reset the start time
    _clock_time = 0ns;
    simulation_end.reset(1);
    scheduler_event_sink.lock()->timeResetBegin(50ms, 0ms);
    {
        ::testing::Sequence s;
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(30ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms)))
            .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                            Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);
        wait_for_simulation_end();
        scheduler.stop();
    }

    ASSERT_FEP3_RESULT(scheduler.deinitialize(), fep3::ERR_NOERROR);
}

//
///**
// * @brief It is tested that scheduling works even if timeResetBegin and timeResetEnd
// * are called before the scheduler was started.
// */
TEST_P(ClockBasedSchedulerCommonTest, ResetBeforeStart)
{
    const auto job_cycle_time = milliseconds(10);
    const auto clock_period = 10ms;
    _max_simulation_time = milliseconds(50);

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);
    ::testing::Sequence s;

    EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(30ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
    // reset the start time
    _clock_time = 0ns;

    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

    wait_for_simulation_end();
    scheduler.stop();
}

//
//    /**
// * @brief The reset behaviour of continuous scheduling is tested
// * It will be simulated for 50ms, after that a reset event to 100ms will be emitted and simulation
// * will be executed for another 50ms.
// * @req_id FEPSDK-2467, FEPSDK-2472, FEPSDK-2468
// */
TEST_P(ClockBasedSchedulerCommonTest, TestReset)
{
    const auto job_cycle_time = milliseconds(10);
    const auto clock_period = 10ms;

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    // simulate until 50ms
    {
        _max_simulation_time = 50ms;
        /// THIS IS BUGGY
        /// CLOCK RUNS until 50ms
        /// Job cycle is 10ms
        /// Theoretically we could have multiple job call

        ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
        // reset the start time
        _clock_time = 0ns;
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(_)).Times(AnyNumber());
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(_max_simulation_time)))
            .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                            Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        wait_for_simulation_end();
    }

    {
        simulation_end.reset(1);

        ::testing::Sequence s;

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(110ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(120ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(130ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(140ms))).WillOnce(Return(fep3::ERR_NOERROR));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(150ms)))
            .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                            Return(fep3::ERR_NOERROR)));

        const auto reset_time = 100ms;

        // reset to 100ms
        scheduler_event_sink.lock()->timeResetBegin(_clock_time, reset_time);
        //
        setClockTime(reset_time);
        // allow the mock clock to run further
        setMaxSimTime(reset_time + _clock_time);

        wait_for_simulation_end();
        scheduler.stop();
    }
}

///**
// * @brief A continuous scheduler is executed for 15ms with a job cycle time of 10ms and offset of
// * 5ms. Job has to be called 2 times (5ms, 15ms).
// * @req_id FEPSDK-2286
// */
TEST_P(ClockBasedSchedulerCommonTest, SchedulingDelayWithRestart)
{
    const auto job_cycle_time = 10ms;
    const auto clock_period = 5ms;
    const auto delay_time = milliseconds(5);
    _max_simulation_time = 20ms;

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time, delay_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = 0ns;
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);
    {
        ::testing::Sequence s;
        EXPECT_CALL(*my_job, execute(_)).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(5ms)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(15ms)))
            .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                            Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        wait_for_simulation_end();
        scheduler.stop();
    }

    simulation_end.reset(1);
    scheduler_event_sink.lock()->timeResetBegin(20ms, 0ms);
    // reset the start time
    _clock_time = 0ns;

    ::testing::Sequence s;
    EXPECT_CALL(*my_job, execute(_)).Times(0);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(5ms)));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(15ms)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);
    wait_for_simulation_end();
    scheduler.stop();
}

///**
// * @brief A continuous scheduler is executed for 100ms with a job cycle time of 20ms and offset of
// * 20ms. Job has to be called 5 times (20ms, 40ms, 60ms, 80ms, 100ms).
// * @req_id FEPSDK-2286
// */
TEST_P(ClockBasedSchedulerCommonTest, SchedulingDelaySameAsCycleTime)
{
    const auto job_cycle_time = 20ms;
    const auto clock_period = 10ms;
    const auto delay_time = 20ms;
    _max_simulation_time = 100ms;

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time, delay_time);
    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = 0ns;
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

    EXPECT_CALL(*my_job, execute(_)).Times(0);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(60ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(80ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(100ms)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
    wait_for_simulation_end();
    scheduler.stop();
}

///**
// * @brief A discrete scheduler is executed for 100ms with a job cycle time of 10ms and offset of
// * 50ms. Job has to be called 6 times (50ms, 60ms, 70ms, 80ms, 90ms, 100ms).
// * @req_id FEPSDK-2286
// */
TEST_P(ClockBasedSchedulerCommonTest, SchedulingDelayLargerThanCycleTime)
{
    const auto job_cycle_time = 10ms;
    const auto clock_period = 5ms;
    const auto delay_time = 50ms;
    _max_simulation_time = 100ms;

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time, delay_time);
    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = 0ns;
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

    EXPECT_CALL(*my_job, execute(_)).Times(0);
    EXPECT_CALL(*my_job, execute(_)).Times(0);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms))).Times(1);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(60ms))).Times(1);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(70ms))).Times(1);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(80ms))).Times(1);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(90ms))).Times(1);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(100ms)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
    wait_for_simulation_end();
    scheduler.stop();
}

///**
// * @brief A continuous scheduler is executed for 9223372036850 ms with a job cycle time of 10ms
// and
// * offset of 9223372036800 ms. Job has to be called 6 times.
// * @req_id FEPSDK-2285
// */
TEST_P(ClockBasedSchedulerCommonTest, SchedulingLargeDelay)
{
    const auto job_cycle_time = milliseconds(10);
    const auto delay_time = fep3::Timestamp::max() - 6 * job_cycle_time;
    const auto clock_period = 10ms;
    // if we simulate unti max, the next step will be max + job_cycle_time
    _max_simulation_time = fep3::Timestamp::max() - job_cycle_time;

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time, delay_time);
    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = delay_time;
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

    EXPECT_CALL(*my_job, execute(_)).Times(0);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + job_cycle_time)))
        .WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + 2 * job_cycle_time)))
        .WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + 3 * job_cycle_time)))
        .WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + 4 * job_cycle_time)))
        .WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + 5 * job_cycle_time)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
    wait_for_simulation_end();
    scheduler.stop();
}

///**
// * @brief It will be tested that a continuous scheduler will no catch up if a job cycle time is
// * skipped. The continous clock will only provide the time of 20ms. Due to not catching up the job
// * at time 10ms is not beeing executed, but skipped.
// * @req_id FEPSDK-2471
// */
TEST_F(ContinuousClockSchedulerTest, NotCatchesUp)
{
    const auto job_cycle_time = 10ms;
    const auto clock_period = 20ms;
    _max_simulation_time = 20ms;

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);
    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = 0ns;
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

    ::testing::Sequence s;
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).Times(0);
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
    wait_for_simulation_end();
    scheduler.stop();
}

//
///**
// * @brief It will be tested that a reset alone won't schedule a job.
// * The scheduler has to be started to start scheduling of the job.
// */
TEST_F(ContinuousClockSchedulerTest, ResetOnlyWontSchedule)
{
    const auto job_cycle_time = milliseconds(10);
    const auto clock_period = 10ms;
    _max_simulation_time = milliseconds(0);

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    // we test that wait for reset is called before execute
    ::testing::Sequence s;

    EXPECT_CALL(*my_job, execute(0ns))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = 0ns;

    scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));

    scheduler.start();
    wait_for_simulation_end();
    scheduler.stop();
}

///**
// * @brief Discrete scheduling will be tested emitting reset and update events more than once
// */
TEST_F(DiscreteClockSchedulerTest, SameEventMoreThanOnce)
{
    const auto job_cycle_time = milliseconds(10);
    const auto clock_period = 10ms;
    _max_simulation_time = milliseconds(0);

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    {
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).Times(1);

        ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
        // reset the start time
        _clock_time = 0ns;
        scheduler.start();

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));
        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        scheduler_event_sink.lock()->timeUpdating(2ms);
        scheduler_event_sink.lock()->timeUpdating(3ms);
        scheduler_event_sink.lock()->timeUpdating(10ms);

        scheduler_event_sink.lock()->timeUpdating(10ms);
        scheduler_event_sink.lock()->timeUpdating(20ms);
        scheduler_event_sink.lock()->timeUpdating(20ms);
    }

    scheduler.stop();
}

///**
// * @brief It will be tested that a discrete scheduler will catch up if a job cycle time is
// skipped.
// * Only one time update event to 20ms will be emitted.
// * Due to the catching up the job at time 10ms is still beeing executed.
// * @req_id FEPSDK-2470
// */
TEST_F(DiscreteClockSchedulerTest, CatchesUp)
{
    const auto job_cycle_time = milliseconds(10);
    const auto clock_period = 20ms;
    _max_simulation_time = milliseconds(20);

    setUpClock(clock_period);
    auto my_job = createClockTriggeredJob(job_cycle_time);

    fep3::native::ClockBasedScheduler scheduler(_logger, _factory);

    ::testing::Sequence s;

    ASSERT_FEP3_RESULT(scheduler.initialize(_clock_service, _jobs), fep3::ERR_NOERROR);
    // reset the start time
    _clock_time = 0ns;
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

    EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).WillOnce(Return(fep3::ERR_NOERROR));
    EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { simulation_end.count_down(); }),
                        Return(fep3::ERR_NOERROR)));

    scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));

    wait_for_simulation_end();
    scheduler.stop();
}

struct SchedulingWithDataTriggered : public ::testing::Test {
    using JobRegistryComponent = NiceMock<fep3::mock::JobRegistry>;
    using LoggingService = NiceMock<fep3::mock::LoggingService>;
    using HealthServiceMock = StrictMock<fep3::mock::HealthService>;
    using ClockServiceMock = NiceMock<fep3::mock::ClockService>;
    using DataRegistryMock = NiceMock<fep3::mock::DataRegistry>;

    SchedulingWithDataTriggered()
    {
    }

    void SetUp() override
    {
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            _job_registry_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IHealthService>(
            _health_service, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            _logger_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(
            _component_registry->registerComponent<fep3::experimental::IClockService>(
                _clock_service_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IDataRegistry>(
            _data_registry_mock, _dummy_component_version_info));
    }

    const fep3::ComponentVersionInfo _dummy_component_version_info{"3.1.0", "dummyPath", "3.1.1"};
    std::shared_ptr<HealthServiceMock> _health_service{std::make_shared<HealthServiceMock>()};
    std::shared_ptr<ClockServiceMock> _clock_service_mock{std::make_shared<ClockServiceMock>()};
    std::shared_ptr<LoggingService> _logger_mock{std::make_shared<LoggingService>()};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{
        std::make_shared<fep3::ComponentRegistry>()};
    std::shared_ptr<JobRegistryComponent> _job_registry_mock{
        std::make_unique<JobRegistryComponent>()};
    std::shared_ptr<DataRegistryMock> _data_registry_mock{std::make_unique<DataRegistryMock>()};
    std::weak_ptr<fep3::experimental::IClock::IEventSink> _scheduler_event_sink;
    std::shared_ptr<fep3::mock::LoggerWithDefaultBehaviour> _logger{
        std::make_shared<fep3::mock::LoggerWithDefaultBehaviour>()};
};

struct SchedulingWithHealthService : public ::testing::Test {
    using JobRegistryComponent = NiceMock<fep3::mock::JobRegistry>;
    using LoggingService = NiceMock<fep3::mock::LoggingService>;
    using HealthServiceMock = StrictMock<fep3::mock::HealthService>;
    using ClockServiceMock = NiceMock<fep3::mock::DiscreteSteppingClockService>;
    using DataRegistryMock = NiceMock<fep3::mock::DataRegistry>;

    SchedulingWithHealthService()
    {
    }

    void SetUp() override
    {
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            _job_registry_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IHealthService>(
            _health_service, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            _logger_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(
            _component_registry->registerComponent<fep3::experimental::IClockService>(
                _clock_service_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IDataRegistry>(
            _data_registry_mock, _dummy_component_version_info));
    }

    const fep3::ComponentVersionInfo _dummy_component_version_info{"3.1.0", "dummyPath", "3.1.1"};
    std::shared_ptr<HealthServiceMock> _health_service{std::make_shared<HealthServiceMock>()};
    std::shared_ptr<ClockServiceMock> _clock_service_mock{std::make_shared<ClockServiceMock>()};
    std::shared_ptr<LoggingService> _logger_mock{std::make_shared<LoggingService>()};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{
        std::make_shared<fep3::ComponentRegistry>()};
    std::shared_ptr<JobRegistryComponent> _job_registry_mock{
        std::make_unique<JobRegistryComponent>()};
    std::shared_ptr<DataRegistryMock> _data_registry_mock{std::make_unique<DataRegistryMock>()};
    std::weak_ptr<fep3::experimental::IClock::IEventSink> _scheduler_event_sink;
    std::shared_ptr<fep3::mock::LoggerWithDefaultBehaviour> _logger{
        std::make_shared<fep3::mock::LoggerWithDefaultBehaviour>()};
};

/**
 * @brief A scheduler is executed for 50ms with a job cycle time of 10ms. Job has to be called 6
 * times.
 * @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2469
 */
TEST_F(SchedulingWithHealthService, SchedulingWithHealthServiceClockTriggered)
{
    const auto max_time = 50ms;
    const auto job_cycle_time = 10ms;

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration>(job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{{builder._job_name, {my_job, builder.makeJobInfoClockTriggered()}}};

    fep3::native::ClockBasedScheduler scheduler(_logger);

    {
        EXPECT_CALL(*_clock_service_mock,
                    registerEventSink(
                        Matcher<const std::weak_ptr<fep3::experimental::IClock::IEventSink>&>(_)))
            .WillOnce(Invoke(
                [&](const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink) {
                    _scheduler_event_sink = event_sink;
                    return fep3::Result{};
                }));

        EXPECT_CALL(*_job_registry_mock, getJobsCatelyn()).WillOnce(::testing::Return(jobs));

        EXPECT_CALL(*_clock_service_mock, getType())
            .WillRepeatedly(Return(fep3::arya::IClock::ClockType::discrete));
        // warnings because 2 time we return non zero error code in job execution.
        EXPECT_CALL(*_logger,
                    logWarning(HasSubstr(
                        "Execution of data processing step failed for this processing cycle")))
            .Times(2)
            .WillRepeatedly(Return(fep3::Result{}));

        using JobExecuteResult = fep3::IHealthService::JobExecuteResult;
        EXPECT_CALL(
            *_health_service,
            updateJobStatus("my_job", Field(&JobExecuteResult::result_execute, Eq(Result{}))))
            .Times(4)
            .WillRepeatedly(Return(Result{}));
        EXPECT_CALL(
            *_health_service,
            updateJobStatus(
                "my_job", Field(&JobExecuteResult::result_execute, Eq(Result{fep3::ERR_UNKNOWN}))))
            .WillOnce(Return(Result{}));
        EXPECT_CALL(*_health_service,
                    updateJobStatus(
                        "my_job",
                        Field(&JobExecuteResult::result_execute, Eq(Result{fep3::ERR_UNEXPECTED}))))
            .WillOnce(Return(Result{}));

        ASSERT_FEP3_RESULT(scheduler.initialize(*_component_registry), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(Return(Result{}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms)))
            .WillOnce(Return(Result{fep3::ERR_UNKNOWN}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms)))
            .WillOnce(Return(Result{fep3::ERR_UNEXPECTED}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(30ms))).WillOnce(Return(Result{}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(Return(Result{}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms))).WillOnce(Return(Result{}));

        _scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        _scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        auto time = Timestamp(0);
        while (time < max_time) {
            time += job_cycle_time;
            _scheduler_event_sink.lock()->timeUpdating(time, {});
        }
    }

    scheduler.stop();
}

/**
 * @brief A data triggered job with health service will not be scheduled with time update.
 */
TEST_F(SchedulingWithHealthService, SchedulingWithHealthServiceDataTriggered)
{
    const auto max_time = 50ms;
    const auto job_cycle_time = 10ms;
    const helper::SimpleJobBuilder builder("my_job", {"my_signal1", "my_signal2"});

    auto my_job = builder.makeDataTriggeredJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{{builder._job_name, {my_job, builder.makeJobInfoDataTriggered()}}};

    fep3::native::ClockBasedScheduler scheduler(_logger);

    {
        EXPECT_CALL(*_clock_service_mock,
                    registerEventSink(
                        Matcher<const std::weak_ptr<fep3::experimental::IClock::IEventSink>&>(_)))
            .WillOnce(Invoke(
                [&](const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink) {
                    _scheduler_event_sink = event_sink;
                    return fep3::Result{};
                }));

        EXPECT_CALL(*_job_registry_mock, getJobsCatelyn()).WillOnce(::testing::Return(jobs));

        EXPECT_CALL(*_clock_service_mock, getType())
            .WillRepeatedly(Return(fep3::arya::IClock::ClockType::discrete));
        EXPECT_CALL(*_data_registry_mock, registerDataReceiveListener("my_signal1", _))
            .WillOnce(Return(Result{}));
        EXPECT_CALL(*_data_registry_mock, registerDataReceiveListener("my_signal2", _))
            .WillOnce(Return(Result{}));

        ASSERT_FEP3_RESULT(scheduler.initialize(*_component_registry), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        _scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        _scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        {
            EXPECT_CALL(*my_job, execute(_)).Times(0);
            auto time = Timestamp(0);
            while (time < max_time) {
                time += job_cycle_time;
                _scheduler_event_sink.lock()->timeUpdating(time, {});
            }
        }
    }

    ASSERT_FEP3_RESULT(scheduler.stop(), fep3::ERR_NOERROR);
}

/**
 * @brief Test DataTriggeredReceiver
 */
TEST_F(SchedulingWithHealthService, DataTriggeredReceiverWithHealthService)
{
    const std::string my_signal_name = "my_signal";
    const helper::SimpleJobBuilder builder("my_job", {my_signal_name});

    auto my_job = builder.makeDataTriggeredJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const auto& config = builder._data_job_config;

    fep3::native::JobRunner job_runner("my_job",
                                       config._runtime_violation_strategy,
                                       config._max_runtime_real_time,
                                       _logger,
                                       *_health_service);

    fep3::mock::MockThreadPoolExecutor mock_pool;
    fep3::native::DataTriggeredExecutor data_triggered_executor(mock_pool);
    data_triggered_executor.start();

    auto data_trigered_receiver = std::make_shared<fep3::native::DataTriggeredReceiver>(
        [&]() { return _clock_service_mock->getTime(); },
        my_job,
        my_signal_name,
        job_runner,
        data_triggered_executor);

    {
        // Run data triggered job 6 times with different timestamps.
        using JobExecuteResult = fep3::IHealthService::JobExecuteResult;
        EXPECT_CALL(
            *_health_service,
            updateJobStatus("my_job", Field(&JobExecuteResult::result_execute, Eq(Result{}))))
            .Times(4)
            .WillRepeatedly(Return(Result{}));
        EXPECT_CALL(
            *_health_service,
            updateJobStatus(
                "my_job", Field(&JobExecuteResult::result_execute, Eq(Result{fep3::ERR_UNKNOWN}))))
            .WillOnce(Return(Result{}));
        EXPECT_CALL(*_health_service,
                    updateJobStatus(
                        "my_job",
                        Field(&JobExecuteResult::result_execute, Eq(Result{fep3::ERR_UNEXPECTED}))))
            .WillOnce(Return(Result{}));

        EXPECT_CALL(*_clock_service_mock, getTime())
            .Times(6)
            .WillOnce(Return(Timestamp(0ms)))
            .WillOnce(Return(Timestamp(10ms)))
            .WillOnce(Return(Timestamp(20ms)))
            .WillOnce(Return(Timestamp(30ms)))
            .WillOnce(Return(Timestamp(40ms)))
            .WillOnce(Return(Timestamp(50ms)));

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(Return(Result{}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms)))
            .WillOnce(Return(Result{fep3::ERR_UNKNOWN}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms)))
            .WillOnce(Return(Result{fep3::ERR_UNEXPECTED}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(30ms))).WillOnce(Return(Result{}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(Return(Result{}));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms))).WillOnce(Return(Result{}));

        uint32_t sample_value = 1;
        const data_read_ptr<const IDataSample> sample =
            std::make_shared<fep3::base::DataSampleType<uint32_t>>(sample_value);
        for (int i = 0; i < 6; ++i) {
            (*data_trigered_receiver)(sample);
        }
    }
    data_triggered_executor.stop();
}

/**
 * @brief A data triggered job will wait to be finished in state deinitialize
 */
TEST_F(SchedulingWithDataTriggered, DataTriggeredExecutorWaitForJobsDone)
{
    const helper::SimpleJobBuilder builder("my_job", {"my_signal1"});

    auto my_job = builder.makeDataTriggeredJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{{builder._job_name, {my_job, builder.makeJobInfoDataTriggered()}}};

    // Use the real thread pool.
    fep3::native::ClockBasedScheduler scheduler(_logger);

    EXPECT_CALL(*_job_registry_mock, getJobsCatelyn()).WillOnce(::testing::Return(jobs));

    std::shared_ptr<fep3::IDataRegistry::IDataReceiver> _data_listener;

    EXPECT_CALL(*_clock_service_mock, getType())
        .WillRepeatedly(Return(fep3::arya::IClock::ClockType::continuous));

    // Catch the real data receiver to input the data
    EXPECT_CALL(
        *_data_registry_mock,
        registerDataReceiveListener(
            "my_signal1", Matcher<const std::shared_ptr<fep3::IDataRegistry::IDataReceiver>&>(_)))
        .WillOnce(Invoke([&](const std::string&,
                             const std::shared_ptr<fep3::IDataRegistry::IDataReceiver>& listener) {
            _data_listener = listener;
            return fep3::Result{};
        }));

    ASSERT_FEP3_RESULT(scheduler.initialize(*_component_registry), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

    EXPECT_CALL(*_clock_service_mock, getTime()).Times(1).WillOnce(Return(Timestamp(0ms)));

    std::atomic_bool task_finished{false};
    boost::latch simulation_end(1);

    using JobExecuteResult = fep3::IHealthService::JobExecuteResult;
    EXPECT_CALL(
        *_health_service,
        updateJobStatus("my_job", Field(&JobExecuteResult::result_execute, Eq(fep3::Result{}))))
        .WillRepeatedly(Return(Result{}));

    EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms)))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() {
                            simulation_end.wait();
                            task_finished = true;
                        }),
                        Return(fep3::ERR_NOERROR)));

    uint32_t sample_value = 1;
    const data_read_ptr<const IDataSample> sample =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(sample_value);
    (*_data_listener)(sample);

    std::atomic_bool stop_finished{false};
    std::thread stop_t([&]() {
        scheduler.stop();
        stop_finished = true;
    });

    ASSERT_FALSE(task_finished);
    ASSERT_FALSE(stop_finished);

    simulation_end.count_down();
    stop_t.join();
    ASSERT_TRUE(task_finished);
    ASSERT_TRUE(stop_finished);
}
