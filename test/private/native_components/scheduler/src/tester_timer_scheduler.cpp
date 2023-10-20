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

#include <fep3/components/clock/mock/mock_clock_service_addons.h>
#include <fep3/components/job_registry/mock_job.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/native_components/scheduler/clock_based/local_clock_based_scheduler.h>

#include <common/gtest_asserts.h>
#include <helper/job_registry_helper.h>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ::testing;
using namespace fep3;
using namespace fep3::native;

using Strategy = fep3::JobConfiguration::TimeViolationStrategy;
using JobMock = NiceMock<fep3::mock::Job>;

struct SchedulerTestEnv {
    SchedulerTestEnv() : _logger(std::make_shared<fep3::mock::LoggerWithDefaultBehavior>())
    {
    }

    template <typename T = fep3::native::JobRunner, typename... Args>
    std::unique_ptr<T> makeChecker(Args&&... args)
    {
        std::unique_ptr<T> runtime_checker =
            std::make_unique<fep3::native::JobRunner>(std::forward<Args>(args)..., *_logger);
        return runtime_checker;
    }

    std::unique_ptr<fep3::native::JobRunner> makeDefaultChecker()
    {
        auto runtime_checker = std::make_unique<fep3::native::JobRunner>(
            "my_runtime_checker", Strategy::ignore_runtime_violation, 10ms, _logger);
        return runtime_checker;
    }

    std::shared_ptr<fep3::mock::LoggerWithDefaultBehavior> _logger;
};

/**
 * @brief TimerScheduler is created and one Timer is added and afterwards removed
 */
TEST(TimerScheduler, AddRemoveTimer)
{
    fep3::mock::DiscreteSteppingClockService clock_service;

    auto timer_scheduler = std::make_shared<TimerScheduler>(clock_service);
    fep3::mock::Job my_job{};

    SchedulerTestEnv scheduler_tester;

    auto runtime_checker = scheduler_tester.makeDefaultChecker();
    auto timer = std::make_shared<TimerThread>("thread_name",
                                               my_job,
                                               clock_service,
                                               duration_cast<Duration>(1us),
                                               duration_cast<Duration>(0us),
                                               *timer_scheduler,
                                               *runtime_checker);

    ASSERT_FEP3_NOERROR(timer_scheduler->addTimer(
        *timer, duration_cast<Duration>(1us), duration_cast<Duration>(0us)));
    ASSERT_FEP3_NOERROR(timer_scheduler->removeTimer(*timer));
    ASSERT_FEP3_RESULT(timer_scheduler->removeTimer(*timer), fep3::ERR_NOT_FOUND);
}

struct TimerSchedulerFixture : public ::testing::Test {
    TimerSchedulerFixture()
    {
    }

    /**
     * Will create a scheduler and add a TimerThread that will execute @p my_job
     */
    void setupSchedulerByJob(fep3::test::helper::TestJob& my_job)
    {
        SchedulerTestEnv runtime_job_tester;

        timer_scheduler = std::make_shared<TimerScheduler>(clock_service);
        auto runtime_checker = runtime_job_tester.makeDefaultChecker();
        timer_thread = std::make_shared<TimerThread>(
            "thread_name",
            my_job,
            clock_service,
            duration_cast<Duration>(my_job._job_config._cycle_sim_time),
            duration_cast<Duration>(0us),
            *timer_scheduler,
            *runtime_checker);

        ASSERT_FEP3_NOERROR(timer_scheduler->addTimer(
            *timer_thread, my_job._job_config._cycle_sim_time, duration_cast<Duration>(0us)));
    }

    /**
     * Will push the clock_service in steps of my_job._cycle_time until max_time is reached.
     * Will wait after every time push that a job is executed
     */
    std::future<void> createSimulateJobUntilMaxTimeFuture(fep3::test::helper::TestJob& my_job,
                                                          Timestamp max_time)
    {
        std::future<void> simulate_until_max_time =
            std::async(std::launch::async, [this, &my_job, max_time]() {
                // simulate t == 0
                my_job._expected_call_time = 0ns;
                my_job.waitForExpectedCallTime(1s);

                // simulate until max_time
                auto clock_time = clock_service.getTime();
                while (clock_time < max_time) {
                    ASSERT_LT(my_job._job_config._cycle_sim_time, 1ms)
                        << "time_increment has to be less than 1 ms. Otherwise TimerScheduler will "
                           "wait forever in this Ttstcase";

                    clock_service.incrementTime(my_job._job_config._cycle_sim_time);
                    my_job._expected_call_time += my_job._job_config._cycle_sim_time;

                    my_job.waitForExpectedCallTime(1s);
                    clock_time = clock_service.getTime();
                }

                timer_scheduler->stop();
            });
        return simulate_until_max_time;
    }

    NiceMock<fep3::mock::DiscreteSteppingClockService> clock_service;
    std::shared_ptr<TimerScheduler> timer_scheduler;
    std::shared_ptr<TimerThread> timer_thread;
};

/**
 * @brief TimerScheduler shall not trigger jobs because of time reset events.
 */
TEST_F(TimerSchedulerFixture, NoExecuteOnTimeReset)
{
    EXPECT_CALL(clock_service, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::discrete));

    auto job_mock = std::make_shared<JobMock>();

    SchedulerTestEnv runtime_job_tester;

    timer_scheduler = std::make_shared<TimerScheduler>(clock_service);
    auto runtime_checker = runtime_job_tester.makeDefaultChecker();
    timer_thread = std::make_shared<TimerThread>("thread_name",
                                                 *job_mock,
                                                 clock_service,
                                                 Duration(0),
                                                 Duration(0),
                                                 *timer_scheduler,
                                                 *runtime_checker);

    ASSERT_FEP3_NOERROR(timer_scheduler->addTimer(*timer_thread, Duration(0), Duration(0)));

    fep3::IClock::IEventSink& scheduler_as_event_sink = *timer_scheduler;

    // actual test
    {
        ASSERT_FEP3_NOERROR(timer_scheduler->start());
        ASSERT_FEP3_NOERROR(timer_thread->start());

        EXPECT_CALL(*job_mock, executeDataIn(_)).Times(0);
        EXPECT_CALL(*job_mock, execute(_)).Times(0);
        EXPECT_CALL(*job_mock, executeDataOut(_)).Times(0);

        scheduler_as_event_sink.timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_as_event_sink.timeResetEnd(Timestamp(0));

        timer_scheduler->stop();
    }
}

/**
 * @brief One Job is added to a TimerScheduler and gets executed by the continuous interface
 *
 * @detail The TimerScheduler executes one job triggered by a clock mock
 * that advances the clock discretly until a defined max_time is reached.
 * The actual call times have to be in job_cycle_time resolution
 */
TEST_F(TimerSchedulerFixture, ExecuteOneJobContinuous)
{
    EXPECT_CALL(clock_service, getType())
        .WillRepeatedly(Return(fep3::IClock::ClockType::continuous));

    const auto max_time = 10ms;
    const auto job_cycle_time = 500us;

    fep3::test::helper::TestJob my_job("my_job", duration_cast<Duration>(job_cycle_time));

    setupSchedulerByJob(my_job);
    fep3::IClock::IEventSink& scheduler_as_event_sink = *timer_scheduler;

    // actual test
    {
        ASSERT_FEP3_NOERROR(timer_thread->start());
        ASSERT_FEP3_NOERROR(timer_scheduler->start());

        /// scheduling will only start if timeReset is emitted
        scheduler_as_event_sink.timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_as_event_sink.timeResetEnd(Timestamp(0));

        // this will start the scheduler
        std::future<void> call_execute = std::async(std::launch::async, [&]() {
            fep3::IJob& scheduler_as_job = *timer_scheduler;
            ASSERT_FEP3_NOERROR(scheduler_as_job.execute(clock_service.getTime()));
        });

        // this will push the clock until 10ms in 500us steps
        std::future<void> simulate_until_max_time =
            createSimulateJobUntilMaxTimeFuture(my_job, max_time);

        simulate_until_max_time.get();
        call_execute.get();

        my_job.assertCallTimeResolution();
        my_job.assertNumberOfCalls(max_time);

        timer_scheduler->stop();
    }
}

/**
 * @brief One Job is added to a TimerScheduler and gets executed by the discrete interface (calls to
 * IEventSink)
 */
TEST_F(TimerSchedulerFixture, ExecuteOneJobDiscrete)
{
    EXPECT_CALL(clock_service, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::discrete));

    const auto max_time = 10ms;
    const auto job_cycle_time = 500us;

    fep3::test::helper::TestJob my_job("my_job", duration_cast<Duration>(job_cycle_time));

    setupSchedulerByJob(my_job);
    fep3::IClock::IEventSink& scheduler_as_event_sink = *timer_scheduler;

    // actual test
    {
        ASSERT_FEP3_NOERROR(timer_scheduler->start());
        ASSERT_FEP3_NOERROR(timer_thread->start());

        /// scheduling will only start if timeReset is emitted
        scheduler_as_event_sink.timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_as_event_sink.timeResetEnd(Timestamp(0));

        // drive scheduler discrete by calling timeUpdating
        auto time = Timestamp(0);
        while (time < max_time) {
            time += job_cycle_time;
            scheduler_as_event_sink.timeUpdating(time);
        }

        my_job.assertCallTimeResolution();
        my_job.assertNumberOfCalls(max_time);

        timer_scheduler->stop();
    }
}

/**
 * @brief One shot job is added to a TimerScheduler and gets executed by the discrete interface
 * exactly once (calls to IEventSink)
 */
TEST_F(TimerSchedulerFixture, ExecuteOneShotJob)
{
    EXPECT_CALL(clock_service, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::discrete));

    const auto job_cycle_time = 0ns, job_initial_delay = 1ns;

    auto job_mock = std::make_shared<JobMock>();

    SchedulerTestEnv runtime_job_tester;

    timer_scheduler = std::make_shared<TimerScheduler>(clock_service);
    auto runtime_checker = runtime_job_tester.makeDefaultChecker();
    timer_thread = std::make_shared<TimerThread>("thread_name",
                                                 *job_mock,
                                                 clock_service,
                                                 job_cycle_time,
                                                 job_initial_delay,
                                                 *timer_scheduler,
                                                 *runtime_checker);

    ASSERT_FEP3_NOERROR(
        timer_scheduler->addTimer(*timer_thread, job_cycle_time, job_initial_delay));

    fep3::IClock::IEventSink& scheduler_as_event_sink = *timer_scheduler;

    // actual test
    {
        ASSERT_FEP3_NOERROR(timer_scheduler->start());
        ASSERT_FEP3_NOERROR(timer_thread->start());

        EXPECT_CALL(*job_mock, executeDataIn(_)).Times(0);
        EXPECT_CALL(*job_mock, execute(_)).Times(0);
        EXPECT_CALL(*job_mock, executeDataOut(_)).Times(0);

        scheduler_as_event_sink.timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_as_event_sink.timeResetEnd(Timestamp(0));

        scheduler_as_event_sink.timeUpdating(Timestamp(0));

        // we expect the one shot job to trigger once at time update of 10us
        EXPECT_CALL(*job_mock, executeDataIn(Timestamp(job_initial_delay))).Times(1);
        EXPECT_CALL(*job_mock, execute(Timestamp(job_initial_delay))).Times(1);
        EXPECT_CALL(*job_mock, executeDataOut(Timestamp(job_initial_delay))).Times(1);

        scheduler_as_event_sink.timeUpdating(Timestamp(job_initial_delay));
        scheduler_as_event_sink.timeUpdating(Timestamp(job_initial_delay * 2));

        timer_scheduler->stop();
    }
}