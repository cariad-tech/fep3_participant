/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/job_registry/mock_job.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/native_components/scheduler/clock_based/data_triggered_receiver.h>
#include <fep3/native_components/scheduler/job_runner.h>

#include <boost/asio/thread_pool.hpp>
#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

struct FakeJobRunner {
    fep3::Result runJob(const fep3::Timestamp trigger_time, fep3::IJob& job)
    {
        return job.execute(trigger_time);
    }
};

template <typename ThreadPoolType>
struct TestDataTriggeredReceiver : public ::testing::Test {
    TestDataTriggeredReceiver(ThreadPoolType& thread_pool)
        : _mock_job(std::make_shared<::testing::StrictMock<fep3::mock::Job>>()),
          _thread_pool(thread_pool),
          _executor(_thread_pool),
          _data_triggered_receiver([this]() { return getTime(); },
                                   _mock_job,
                                   "signal_name",
                                   _job_runner,
                                   _executor,
                                   _logger)
    {
    }

    void SetUp() override
    {
        _thread_pool.start();
    }

    void TearDown() override
    {
        _thread_pool.stop();
    }

    fep3::Timestamp getTime()
    {
        using namespace std::chrono_literals;
        return 0ns;
    }

    std::shared_ptr<::testing::StrictMock<fep3::mock::Job>> _mock_job;
    FakeJobRunner _job_runner;

    ThreadPoolType& _thread_pool;
    fep3::native::DataTriggeredExecutor _executor;
    std::shared_ptr<NiceMock<fep3::mock::LoggerWithDefaultBehavior>> _logger =
        std::make_shared<NiceMock<fep3::mock::LoggerWithDefaultBehavior>>();
    fep3::native::DataTriggeredReceiver<FakeJobRunner> _data_triggered_receiver;
};

struct TestDataTriggeredReceiverWithMockThreadPool
    : public TestDataTriggeredReceiver<fep3::mock::MockThreadPoolExecutor> {
    TestDataTriggeredReceiverWithMockThreadPool()
        : TestDataTriggeredReceiver<fep3::mock::MockThreadPoolExecutor>(_thread_pool)
    {
    }

    fep3::mock::MockThreadPoolExecutor _thread_pool;
};

struct TestDataTriggeredReceiverWithRealThreadPool
    : public TestDataTriggeredReceiver<fep3::native::ThreadPoolExecutor> {
    TestDataTriggeredReceiverWithRealThreadPool()
        : TestDataTriggeredReceiver<fep3::native::ThreadPoolExecutor>(_thread_pool)
    {
    }

    fep3::native::ThreadPoolExecutor _thread_pool = fep3::native::ThreadPoolExecutor{5};
};

TEST_F(TestDataTriggeredReceiverWithMockThreadPool,
       firstOpNotPostedBeforeStart_secondOpCanBePostedAfterStart)
{
    EXPECT_CALL(*_mock_job, execute(_)).WillOnce(Return(fep3::Result{}));

    _data_triggered_receiver(std::shared_ptr<const fep3::arya::IDataSample>{});
    _executor.start();
    _data_triggered_receiver(std::shared_ptr<const fep3::arya::IDataSample>{});
}

TEST_F(TestDataTriggeredReceiverWithMockThreadPool, OpBeforeStart_WarningLogged)
{
    EXPECT_CALL(*_logger, logWarning(_)).WillOnce(Return(fep3::Result{}));
    _data_triggered_receiver(std::shared_ptr<const fep3::arya::IDataSample>{});
}

TEST_F(TestDataTriggeredReceiverWithMockThreadPool, busyJob_execute_warningLogged)
{
    boost::latch latch_job_in(1);
    boost::latch latch_job_out(1);

    ::Sequence{};
    EXPECT_CALL(*_mock_job, execute(_))
        .WillOnce(InvokeWithoutArgs([&latch_job_in, &latch_job_out]() {
            latch_job_in.count_down();
            latch_job_out.wait();
            return fep3::Result{};
        }));

    EXPECT_CALL(*_logger, logWarning(_)).WillOnce(Return(fep3::Result{}));

    _executor.start();

    auto thread_job = std::thread(
        [&]() { _data_triggered_receiver(std::shared_ptr<const fep3::arya::IDataSample>{}); });

    latch_job_in.wait();
    _data_triggered_receiver(std::shared_ptr<const fep3::arya::IDataSample>{});

    latch_job_out.count_down();
    thread_job.join();
}

TEST_F(TestDataTriggeredReceiverWithMockThreadPool, busyJob_furtherJobsNotTriggered)
{
    const uint32_t thread_count = 8;
    boost::latch latch_job_in(1);
    boost::latch latch_job_out(1);

    boost::asio::thread_pool pool(thread_count);

    EXPECT_CALL(*_mock_job, execute(_))
        .WillOnce(DoAll(InvokeWithoutArgs([&latch_job_in, &latch_job_out]() {
                            latch_job_in.count_down();
                            latch_job_out.wait();
                        }),
                        Return(fep3::Result{})));

    _executor.start();

    auto thread_job = std::thread(
        [&]() { _data_triggered_receiver(std::shared_ptr<const fep3::arya::IDataSample>{}); });

    latch_job_in.wait();
    // now the rest should not be triggered
    for (auto i = 0u; i < thread_count; ++i) {
        boost::asio::post(pool, [&]() {
            _data_triggered_receiver(std::shared_ptr<const fep3::arya::IDataSample>{});
        });
    }

    // here the jobs are triggered in the same thread that the operator()
    //  is executed. Means when the operator returns the job is either
    // executed or not
    pool.join();

    latch_job_out.count_down();
    thread_job.join();
}

TEST_F(TestDataTriggeredReceiverWithRealThreadPool, onlyOneJobRuns_OtherJobsDiscarded)
{
    const uint32_t thread_count = 8;
    boost::asio::thread_pool pool(thread_count);

    std::shared_ptr<std::mutex> unique_access = std::make_shared<std::mutex>();
    using namespace std::chrono_literals;
    EXPECT_CALL(*_mock_job, execute(_))
        .WillRepeatedly(DoAll(InvokeWithoutArgs([unique_access]() {
                                  bool res = unique_access->try_lock();
                                  std::this_thread::yield();
                                  ASSERT_TRUE(res) << "Jobs should be triggered sequentially";
                                  if (res) {
                                      unique_access->unlock();
                                  }
                              }),
                              Return(fep3::Result{})));

    _executor.start();

    for (auto i = 0u; i < 2 * thread_count; ++i) {
        boost::asio::post(pool, [&]() {
            _data_triggered_receiver(std::shared_ptr<const fep3::arya::IDataSample>{});
        });
    }

    pool.join();
}
