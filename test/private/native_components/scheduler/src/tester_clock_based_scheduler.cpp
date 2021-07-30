/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>
#include <chrono>
#include <iostream>

#include <fep3/native_components/scheduler/clock_based/local_clock_based_scheduler.h>
#include <fep3/components/job_registry/job_info.h>
#include <fep3/components/job_registry/job_intf.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/core/mock/mock_core.h>

#include <helper/job_registry_helper.h>
#include <testenvs/scheduler_envs.h>
#include <helper/gmock_async_helper.h>

using namespace ::testing;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace fep3::test;
using namespace fep3;

template <class T>
struct ClockBasedSchedulerFixture : public ::testing::Test
{
    ClockBasedSchedulerFixture()
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(clock_service, registerEventSink(_))
        .WillOnce(Invoke([this](const std::weak_ptr<IClock::IEventSink>& event_sink)
            {
                scheduler_event_sink = event_sink;
                return fep3::Result{};
            }));
    }

    NiceMock<T> clock_service;
    std::weak_ptr<fep3::IClock::IEventSink> scheduler_event_sink;
    env::SchedulerTestEnv scheduler_test;
    static const std::chrono::milliseconds timeout;
};

template <class T>
const std::chrono::milliseconds ClockBasedSchedulerFixture<T>::timeout = std::chrono::milliseconds(100);

struct ClockBasedSchedulerDiscrete
    : public ClockBasedSchedulerFixture<fep3::mock::DiscreteSteppingClockService>
{
    void SetUp() override
    {
        ClockBasedSchedulerFixture::SetUp();
        EXPECT_CALL(clock_service, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::discrete));
    };
};

struct ClockBasedSchedulerContinuous
    : public ClockBasedSchedulerFixture<fep3::mock::DiscreteSteppingClockService>
{
    void SetUp() override
    {
        ClockBasedSchedulerFixture::SetUp();
        EXPECT_CALL(clock_service, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::continuous));
    }
};

struct ClockBasedSchedulerContinuousChrono
    : public ClockBasedSchedulerFixture<fep3::mock::ChronoDrivenClockService>
{
    void SetUp() override
    {
        ClockBasedSchedulerFixture::SetUp();
        EXPECT_CALL(clock_service, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::continuous));
    }
};

/**
* @brief A scheduler is executed for 50ms with a job cycle time of 10ms. Job has to be called 6 times.
* @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2286, FEPSDK-2468
*/
TEST_F(ClockBasedSchedulerContinuous, Scheduling)
{
    const auto max_time = milliseconds(50);
    const auto job_cycle_time = milliseconds(10);

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ::test::helper::Notification called_first;
        ::test::helper::Notification called;

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(DoAll(Notify(&called_first), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(30ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

         // this will push the clock until 50ms in 10ms steps
        auto simulate_until_max_time = [&]() {
            called_first.waitForNotificationWithTimeout(std::chrono::seconds(1));

            while (clock_service.getTime() != max_time)
            {
                clock_service.incrementTime(job_cycle_time);
                ASSERT_TRUE(called.waitForNotificationWithTimeout(std::chrono::seconds(1)));
            }
        }; simulate_until_max_time();
    }

    scheduler.stop();
}

/**
 * @brief It is tested that scheduling works even if timeResetBegin and timeResetEnd
 * are called before the scheduler was started.
 */
TEST_F(ClockBasedSchedulerContinuous, ResetBeforeStart)
{
    const auto max_time = milliseconds(10);
    const auto job_cycle_time = milliseconds(10);

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ::test::helper::Notification called_first;
        ::test::helper::Notification called;

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(DoAll(Notify(&called_first), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

         // this will push the clock until 50ms in 10ms steps
        auto simulate_until_max_time = [&]() {
            called_first.waitForNotificationWithTimeout(std::chrono::seconds(1));

            while (clock_service.getTime() != max_time)
            {
                clock_service.incrementTime(job_cycle_time);
                ASSERT_TRUE(called.waitForNotificationWithTimeout(std::chrono::seconds(1)));
            }
        }; simulate_until_max_time();
    }

    scheduler.stop();
}

/**
 * @brief It will be tested that a reset alone won't schedule a job.
 * The scheduler has to be started to start scheduling of the job.
 *
 */
TEST_F(ClockBasedSchedulerContinuousChrono, ResetOnlyWontSchedule)
{
    const auto job_cycle_time = milliseconds(10);

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);
    ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
    ASSERT_FEP3_NOERROR(clock_service.start());

    {
        // resetting only => no job called
        {
            scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
            scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

            EXPECT_CALL(*my_job, execute(_)).Times(0);
            /// i have no better idea then wait here
            std::this_thread::sleep_for(200ms);
        }

        // starting scheduler => job will be called
        {
            ::test::helper::Notification called;
            EXPECT_CALL(*my_job, execute(_)).Times(1).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));

            scheduler.start();
            ASSERT_TRUE(called.waitForNotificationWithTimeout(200ms));
        }
    }

    scheduler.stop();
}

/**
 * @brief The reset behaviour of continuous scheduling is tested
 * It will be simulated for 50ms, after that a reset event to 100ms will be emitted and simulation
 * will be executed for another 50ms.
 * @req_id FEPSDK-2467, FEPSDK-2472, FEPSDK-2468
 */
TEST_F(ClockBasedSchedulerContinuous, TestReset)
{
    const auto max_time = milliseconds(50);
    const auto job_cycle_time = milliseconds(10);

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    // simulate until 50ms
    {
        ::test::helper::Notification called_max_time;

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(_)).Times(AnyNumber());
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(max_time))).WillOnce(DoAll(Notify(&called_max_time), Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        clock_service.setCurrentTime(max_time);
        ASSERT_TRUE(called_max_time.waitForNotificationWithTimeout(std::chrono::seconds(1)));
    }

    // reset to 100ms
    {
        const auto reset_time = 100ms;
        const auto new_max_time = reset_time + max_time;

        ::test::helper::Notification called_0ms;
        ::test::helper::Notification called;

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(100ms))).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(110ms))).WillOnce(DoAll(Notify(&called_0ms), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(120ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(130ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(140ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(150ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));

        // this is the actual reset
        scheduler_event_sink.lock()->timeResetBegin(max_time, reset_time);
        scheduler_event_sink.lock()->timeResetEnd(reset_time);

        /// we make sure that the reset time won't be scheduled (don't know any better way then waiting)
        clock_service.setCurrentTime(reset_time);
        std::this_thread::sleep_for(500ms);

        clock_service.setCurrentTime(reset_time + job_cycle_time);

        auto simulate_until_max_time = [&]() {
            ASSERT_TRUE(called_0ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));

            while (clock_service.getTime() != new_max_time)
            {
                clock_service.incrementTime(job_cycle_time);
                ASSERT_TRUE(called.waitForNotificationWithTimeout(std::chrono::seconds(1)));
            }
        }; simulate_until_max_time();
    }
     scheduler.stop();
}



/**
* @brief A scheduler is executed for 50ms with a job cycle time of 10ms. Job has to be called 6 times.
* @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2469
*/
TEST_F(ClockBasedSchedulerDiscrete, Scheduling)
{
    const auto max_time = 50ms;
    const auto job_cycle_time = 10ms;

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(30ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms))).Times(1);

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        auto time = Timestamp(0);
        while (time < max_time)
        {
            time += job_cycle_time;
            scheduler_event_sink.lock()->timeUpdating(time);
        }
    }

    scheduler.stop();
}


/**
 * @brief It is tested that scheduling works even if timeResetBegin and timeResetEnd
 * are called before the scheduler was started.
 */
TEST_F(ClockBasedSchedulerDiscrete, ResetBeforeStart)
{
    const auto max_time = 10ms;
    const auto job_cycle_time = 10ms;

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).Times(1);

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        auto time = Timestamp(0);
        while (time < max_time)
        {
            time += job_cycle_time;
            scheduler_event_sink.lock()->timeUpdating(time);
        }
    }

    scheduler.stop();
}


/**
 * @brief Discrete scheduling will be tested emitting reset and update events more than once
 */
TEST_F(ClockBasedSchedulerDiscrete, SameEventMoreThanOnce)
{
    const auto job_cycle_time = 10ms;
    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };
    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).Times(1);

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

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


/**
 * @brief The reset behaviour of discrete scheduling is tested
 * It will be simulated for 50ms, after that a reset event to 100ms will be emitted and simulation
 * will be executed for another 50ms.
 * @req_id FEPSDK-2467, FEPSDK-2472, FEPSDK-2469
 */
TEST_F(ClockBasedSchedulerDiscrete, TestReset)
{
    const auto max_time = 50ms;
    const auto job_cycle_time = 10ms;

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    // simulate until 50ms
    {
        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        scheduler_event_sink.lock()->timeUpdating(max_time);
    }

    // reset to 100ms
    {
        const auto reset_time = 100ms;
        const auto new_max_time = reset_time + max_time;

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(100ms))).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(110ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(120ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(130ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(140ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(150ms))).Times(1);

        scheduler_event_sink.lock()->timeResetBegin(max_time, reset_time);
        scheduler_event_sink.lock()->timeResetEnd(reset_time);

        auto time = reset_time;
        while (time < new_max_time)
        {
            time += job_cycle_time;
            scheduler_event_sink.lock()->timeUpdating(time);
        }
    }

    scheduler.stop();
}


/**
 * @brief It will be tested that a discrete scheduler will catch up if a job cycle time is skipped.
 * Only one time update event to 20ms will be emitted.
 * Due to the catching up the job at time 10ms is still beeing executed.
 * @req_id FEPSDK-2470
 */
TEST_F(ClockBasedSchedulerDiscrete, CatchesUp)
{
    const auto max_time = 20ms;
    const auto job_cycle_time = 10ms;

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).Times(1);

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        /// we are going directly to 20 ms (skipping 10 ms)
        scheduler_event_sink.lock()->timeUpdating(max_time);
    }

    scheduler.stop();
}

/**
 * @brief It will be tested that a continuous scheduler will no catch up if a job cycle time is skipped.
 * The continous clock will only provide the time of 20ms.
 * Due to not catching up the job at time 10ms is not beeing executed, but skipped.
 * @req_id FEPSDK-2471
 */
TEST_F(ClockBasedSchedulerContinuous, NotCatchesUp)
{
    const auto max_time = 20ms;
    const auto job_cycle_time = 10ms;

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ::test::helper::Notification called_0ms;
        ::test::helper::Notification called_20ms;

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(fep3::Timestamp(0ms))).WillOnce(DoAll(Notify(&called_0ms), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(10ms))).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(max_time))).WillOnce(DoAll(Notify(&called_20ms), Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        ASSERT_TRUE(called_0ms.waitForNotificationWithTimeout(1s));
        /// we are going directly to 20 ms (skipping 10 ms)
        clock_service.setCurrentTime(max_time);
        ASSERT_TRUE(called_20ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));
    }

    scheduler.stop();
}

/**
* @brief A continuous scheduler is executed for 15ms with a job cycle time of 10ms and offset of 5ms.
* Job has to be called 2 times (5ms, 15ms).
* @req_id FEPSDK-2286
*/
TEST_F(ClockBasedSchedulerContinuous, SchedulingDelay)
{
    const auto job_cycle_time = milliseconds(10);
    const auto delay_time = milliseconds(5);

    const helper::SimpleJobBuilder builder("my_job",
        duration_cast<fep3::Duration>(job_cycle_time),
        duration_cast<fep3::Duration>(delay_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ::test::helper::Notification called;

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(_)).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(5ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(15ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        // this will push the clock until 15ms in 5ms steps
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 0ms
        clock_service.incrementTime(5ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // 5ms
        clock_service.incrementTime(5ms);
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 10ms
        clock_service.incrementTime(5ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // 15ms
    }

    scheduler.stop();
}

/**
* @brief A discrete scheduler is executed for 15ms with a job cycle time of 10ms and offset of 5ms.
* Job has to be called 2 times (5ms, 15ms).
* @req_id FEPSDK-2286
*/
TEST_F(ClockBasedSchedulerDiscrete, SchedulingDelay)
{
    const auto job_cycle_time = milliseconds(10);
    const auto delay_time = milliseconds(5);

    const helper::SimpleJobBuilder builder("my_job",
        duration_cast<fep3::Duration>(job_cycle_time),
        duration_cast<fep3::Duration>(delay_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(_)).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(5ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(15ms))).Times(1);

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        scheduler_event_sink.lock()->timeUpdating(0ms);
        scheduler_event_sink.lock()->timeUpdating(5ms);
        scheduler_event_sink.lock()->timeUpdating(10ms);
        scheduler_event_sink.lock()->timeUpdating(15ms);
    }

    scheduler.stop();
}

/**
* @brief A continuous scheduler is executed for 100ms with a job cycle time of 20ms and offset of 20ms.
* Job has to be called 5 times (20ms, 40ms, 60ms, 80ms, 100ms).
* @req_id FEPSDK-2286
*/
TEST_F(ClockBasedSchedulerContinuous, SchedulingDelaySameAsCycleTime)
{
    const auto job_cycle_time = milliseconds(20);
    const auto delay_time = milliseconds(20);

    const helper::SimpleJobBuilder builder("my_job",
        duration_cast<fep3::Duration>(job_cycle_time),
        duration_cast<fep3::Duration>(delay_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ::test::helper::Notification called;

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(_)).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(20ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(40ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(60ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(80ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(100ms))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        // this will push the clock until 100ms in 10ms steps
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 0ms
        clock_service.incrementTime(10ms);
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 10ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // 20ms
        clock_service.incrementTime(10ms);
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 30ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // 40ms
        clock_service.incrementTime(10ms);
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 50ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // 60ms
        clock_service.incrementTime(10ms);
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 70ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // 80ms
        clock_service.incrementTime(10ms);
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 90ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // 100ms
    }

    scheduler.stop();
}

/**
* @brief A discrete scheduler is executed for 100ms with a job cycle time of 10ms and offset of 50ms.
* Job has to be called 6 times (50ms, 60ms, 70ms, 80ms, 90ms, 100ms).
* @req_id FEPSDK-2286
*/
TEST_F(ClockBasedSchedulerDiscrete, SchedulingDelayLargerThanCycleTime)
{
    const auto job_cycle_time = milliseconds(10);
    const auto delay_time = milliseconds(50);

    const helper::SimpleJobBuilder builder("my_job",
        duration_cast<fep3::Duration>(job_cycle_time),
        duration_cast<fep3::Duration>(delay_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(_)).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(50ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(60ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(70ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(80ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(90ms))).Times(1);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(100ms))).Times(1);


        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        for (int i = 0; i <= 100; ++i)
        {
            scheduler_event_sink.lock()->timeUpdating(milliseconds(i));
        }
    }

    scheduler.stop();
}

/**
* @brief A continuous scheduler is executed for 9223372036850 ms with a job cycle time of 10ms and offset of 9223372036800 ms.
* Job has to be called 6 times.
* @req_id FEPSDK-2285
*/
TEST_F(ClockBasedSchedulerContinuous, SchedulingLargeDelay)
{
    const auto job_cycle_time = milliseconds(10);
    const auto delay_time = milliseconds(9223372036800l);

    const helper::SimpleJobBuilder builder("my_job",
        duration_cast<fep3::Duration>(job_cycle_time),
        duration_cast<fep3::Duration>(delay_time));

    auto my_job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    my_job->setDefaultBehaviour();

    const fep3::Jobs jobs{ {builder.makeJobInfo().getName(), {my_job, builder.makeJobInfo()}} };

    fep3::native::LocalClockBasedScheduler scheduler(scheduler_test._logger);

    {
        ::test::helper::Notification called;

        ASSERT_FEP3_RESULT(scheduler.initialize(clock_service, jobs), fep3::ERR_NOERROR);
        ASSERT_FEP3_RESULT(scheduler.start(), fep3::ERR_NOERROR);

        EXPECT_CALL(*my_job, execute(_)).Times(0);
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + job_cycle_time))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + 2*job_cycle_time))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + 3*job_cycle_time))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + 4*job_cycle_time))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));
        EXPECT_CALL(*my_job, execute(fep3::Timestamp(delay_time + 5*job_cycle_time))).WillOnce(DoAll(Notify(&called), Return(fep3::ERR_NOERROR)));

        scheduler_event_sink.lock()->timeResetBegin(Timestamp(0), Timestamp(0));
        scheduler_event_sink.lock()->timeResetEnd(Timestamp(0));

        // this will push the clock until 9223372036850ms in 10ms steps
        ASSERT_FALSE(called.waitForNotificationWithTimeout(timeout)); // 0ms
        clock_service.incrementTime(delay_time);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // 9223372036800ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // ...10ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // ...20ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // ...30ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // ...40ms
        clock_service.incrementTime(10ms);
        ASSERT_TRUE(called.waitForNotificationWithTimeout(timeout)); // ...50ms
    }

    scheduler.stop();
}
