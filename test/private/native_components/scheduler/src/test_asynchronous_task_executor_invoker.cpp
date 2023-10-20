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
#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/native_components/scheduler/clock_based/system_clock/asynchronous_task_executor_invoker.h>

#include <boost/thread/barrier.hpp>
#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ::testing;
using namespace fep3::native;
using namespace fep3::mock;

struct TestAsyncTaskExecutorInvoker : public ::testing::Test {
    using NotificationMock = NiceMock<NotificationWaitingWithMockTimeout<true>>;
    using AsyncTaskExecutorDummy =
        AsyncTaskExecutorInvoker<NotificationMock, fep3::mock::AsyncTaskExecutorDummy>;

    void SetUp() override
    {
        _processor_mock = std::make_unique<NiceMock<AsyncTaskExecutorMock>>();

        _async_processor = std::make_unique<AsyncTaskExecutorDummy>(
            [&]() { return getTime(); },
            [&]() { return fep3::mock::AsyncTaskExecutorDummy(_processor_mock.get()); });
    }

    void TearDown() override
    {
        _async_processor.reset();
    }

    fep3::Timestamp getTime()
    {
        return _clock_time;
    };

    std::unique_ptr<AsyncTaskExecutorDummy> _async_processor;
    std::unique_ptr<NiceMock<AsyncTaskExecutorMock>> _processor_mock;
    NiceMock<MockClockFunction> _mock_clock_function;
    fep3::Timestamp _clock_time{0};
};

// NO RUN before start of task processor
TEST_F(TestAsyncTaskExecutorInvoker, start_NoProcessingUntilFirstTimeReset)
{
    EXPECT_CALL(*_processor_mock, run(_)).Times(0);
    _async_processor->start();
}

// NO further RUN after STOP
// simulates a stop received when a task is runnning
TEST_F(TestAsyncTaskExecutorInvoker, stopWhileTaskRunning_NoProcessingAfterStop)
{
    boost::latch wait_for_job_execution(1);

    std::atomic<bool> stoped(false);
    EXPECT_CALL(*_processor_mock, run(_)).WillRepeatedly(InvokeWithoutArgs([&]() {
        // wait until the processor stop is called
        EXPECT_FALSE(stoped) << "processor should not run again if stoped";
        return fep3::Timestamp{1s};
    }));

    EXPECT_CALL(*_processor_mock, run(0ns)).WillOnce(InvokeWithoutArgs([&]() {
        // wait until the processor stop is called
        wait_for_job_execution.count_down();

        // the stop flag is set, so we are 100% sure that this is the only call
        // to the run()
        EXPECT_FALSE(stoped) << "processor should not run again if stoped";
        return fep3::Timestamp{1s};
    }));

    _async_processor->timeReset(0ns, 0ns);
    _async_processor->start();
    wait_for_job_execution.wait();
    _async_processor->stop();
    stoped = true;
}

TEST_F(TestAsyncTaskExecutorInvoker, processInMainLoop_ProcessorCallWaitsReset)
{
    boost::barrier barrier(2);

    Sequence s;
    bool run_called = false;

    EXPECT_CALL(*_processor_mock, run(0ns)).WillRepeatedly(InvokeWithoutArgs([&]() {
        if (!run_called) {
            run_called = true;
            barrier.count_down_and_wait();
        }

        return fep3::Timestamp{0};
    }));

    _async_processor->start();
    ASSERT_FALSE(run_called);
    _async_processor->timeReset(0ns, 0ns);
    barrier.count_down_and_wait();
    _async_processor->stop();
}

TEST_F(TestAsyncTaskExecutorInvoker, run_ResetAndQueueProcessingDoNotRunConcurently)
{
    boost::barrier barrier(2);

    std::atomic<bool> processing_running{true};
    std::atomic<bool> reset_running{false};

    _async_processor->timeReset(0ns, 0ns);
    {
        EXPECT_CALL(*_processor_mock, run(10ns)).WillRepeatedly(Return(fep3::Timestamp{0}));
        EXPECT_CALL(*_processor_mock, run(0ns)).WillOnce(InvokeWithoutArgs([&]() {
            barrier.count_down_and_wait();
            processing_running = false;
            _clock_time = 10ns;
            EXPECT_FALSE(reset_running);
            return fep3::Timestamp{0};
        }));

        EXPECT_CALL(*_processor_mock, timeReset(_, _)).WillOnce(InvokeWithoutArgs([&]() {
            reset_running = true;
            EXPECT_FALSE(processing_running);
        }));

        _async_processor->start();
        barrier.count_down_and_wait();
        _async_processor->timeReset(0ns, 10ns);
    }
}
