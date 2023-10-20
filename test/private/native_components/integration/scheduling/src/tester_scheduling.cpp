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
#include "fep3/native_components/scheduler/clock_based/scheduler_factory.h"
#include "fep3/native_components/scheduler/clock_based/system_clock/asynchronous_task_executor.h"
#include "fep3/native_components/scheduler/clock_based/system_clock/asynchronous_task_executor_invoker.h"
#include "notification_waiting.h"
#include "threaded_executor.h"

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/clock/mock_clock.h>
#include <fep3/components/data_registry/mock_data_registry.h>
#include <fep3/components/health_service/mock_health_service.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/core/mock/mock_core.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/native_components/clock/clock_service.h>
#include <fep3/native_components/clock_sync/clock_sync_service.h>
#include <fep3/native_components/clock_sync/master_on_demand_clock_client.h>
#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/native_components/scheduler/local_scheduler_service.h>
#include <fep3/rpc_services/clock/clock_service_rpc_intf_def.h>

#include <common/gtest_asserts.h>
#include <helper/gmock_async_helper.h>
#include <helper/job_registry_helper.h>

using namespace ::testing;
using namespace fep3::test;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace fep3::native;

using WarningLoggerMock = NiceMock<fep3::mock::WarningLogger>;

using LoggingService = NiceMock<fep3::mock::LoggingService>;
using ServiceBusComponent = NiceMock<fep3::mock::ServiceBus>;
using RPCServer = NiceMock<fep3::mock::RPCServer>;
using RPCRequester = NiceMock<fep3::mock::RPCRequester>;
using HealthServiceMock = NiceMock<fep3::mock::HealthService>;
using DataRegistryMock = NiceMock<fep3::mock::DataRegistry>;

// this factory uses just a dummy thread pool so by
// executing tasks in the same thread are done before the next run
struct TestSchedulerFactory : public ISchedulerFactory {
    TestSchedulerFactory(IThreadPoolExecutor& mock_pool) : _mock_pool(mock_pool)
    {
    }
    using AsyncTaskExecutorWithSerialTaskExecution = fep3::native::AsyncTaskExecutor;

    // this is the type used in production with the exception that the thread pool executes the
    // tasks on post() and waiting on timeout just continues
    using TestAsyncTaskExecutorInvoker =
        AsyncTaskExecutorInvoker<NotificationWaiting, AsyncTaskExecutorWithSerialTaskExecution>;

    std::unique_ptr<fep3::native::ITaskExecutorInvoker> createSchedulerProcessor(
        IThreadPoolExecutor& threaded_executor,
        fep3::arya::IClock::ClockType clock_type,
        std::function<fep3::Timestamp()> time_getter,
        std::shared_ptr<const fep3::ILogger> logger) const override
    {
        switch (clock_type) {
        case (fep3::arya::IClock::ClockType::discrete):
            return _default_factory.createSchedulerProcessor(
                threaded_executor, clock_type, time_getter, logger);
        case (fep3::arya::IClock::ClockType::continuous):
            return std::make_unique<TestAsyncTaskExecutorInvoker>(std::move(time_getter), [&]() {
                return AsyncTaskExecutorWithSerialTaskExecution(_mock_pool);
            });
        }
        return nullptr;
    }

private:
    SchedulerFactory _default_factory;
    IThreadPoolExecutor& _mock_pool;
};

class ExternalClockSimulationMock : public IExternalClock {
public:
    ExternalClockSimulationMock(std::chrono::milliseconds step = 100ms) : _step(step)
    {
    }
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    void setTimeStep(std::chrono::milliseconds step)
    {
        _step = step;
    }

    TimePoint now()
    {
        return _current_time;
    }
    void waitUntil(std::function<void()>)
    {
    }
    void notify(std::function<void()>)
    {
    }

    void increment()
    {
        _current_time += _step;
    }

private:
    TimePoint _current_time = std::chrono::steady_clock::now();
    std::chrono::milliseconds _step;
};

struct SchedulingWithNativeClock : public ::testing::TestWithParam<std::string> {
    SchedulingWithNativeClock()
        : _component_registry(std::make_shared<fep3::ComponentRegistry>()),
          _logging_service_mock(std::make_shared<LoggingService>()),
          _service_bus(std::make_shared<ServiceBusComponent>()),
          _rpc_server(std::make_shared<RPCServer>()),
          _health_service(std::make_shared<HealthServiceMock>()),
          _data_registry(std::make_shared<DataRegistryMock>())
    {
    }

    void SetUp() override
    {
        using namespace fep3;
        using namespace native;
        using namespace fep3::rpc;

        EXPECT_CALL(*_service_bus, getServer()).WillRepeatedly(Return(_rpc_server));
        EXPECT_CALL(*_rpc_server, registerService(IRPCClockSyncMasterDef::getRPCDefaultName(), _))
            .WillOnce(Return(Result()));
        EXPECT_CALL(*_rpc_server, registerService(IRPCClockServiceDef::getRPCDefaultName(), _))
            .WillOnce(Return(Result()));
        EXPECT_CALL(*_rpc_server, registerService(IRPCConfigurationDef::getRPCDefaultName(), _))
            .WillOnce(Return(Result()));
        EXPECT_CALL(*_rpc_server, registerService(IRPCSchedulerServiceDef::getRPCDefaultName(), _))
            .WillOnce(Return(Result()));
        EXPECT_CALL(*_rpc_server, registerService(IRPCJobRegistryDef::getRPCDefaultName(), _))
            .WillOnce(Return(Result()));

        EXPECT_CALL(*_rpc_server, unregisterService(_)).WillRepeatedly(Return(fep3::Result()));

        registerComponents();
        setComponents();

        _logger = std::make_shared<WarningLoggerMock>();
        EXPECT_CALL(*_logging_service_mock, createLogger(_)).WillRepeatedly(Return(_logger));

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    void registerComponents()
    {
        using namespace fep3::native;

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            std::make_unique<JobRegistry>(), _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ISchedulerService>(
            std::make_shared<LocalSchedulerService>(), _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            _logging_service_mock, _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus, _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(
            _component_registry->registerComponent<fep3::experimental::IClockService>(
                _clock_service_catelyn, _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::arya::IClockService>(
            _clock_service_catelyn, _dummy_component_version_info));

        if (hasClockSyncService()) {
            ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IClockSyncService>(
                std::make_shared<fep3::native::ClockSynchronizationService>(),
                _dummy_component_version_info));
        }

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            std::make_shared<ConfigurationService>(), _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IHealthService>(
            _health_service, _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IDataRegistry>(
            _data_registry, _dummy_component_version_info));
    }

    void setComponents()
    {
        _clock_service = _component_registry->getComponent<fep3::experimental::IClockService>();
        ASSERT_NE(_clock_service, nullptr);

        if (hasClockSyncService()) {
            _clock_sync_service = _component_registry->getComponent<fep3::IClockSyncService>();
            ASSERT_NE(_clock_sync_service, nullptr);
        }

        _scheduler_service = _component_registry->getComponent<fep3::ISchedulerService>();
        ASSERT_NE(_scheduler_service, nullptr);

        _job_registry_impl = _component_registry->getComponent<fep3::IJobRegistry>();
        ASSERT_NE(_job_registry_impl, nullptr);
    }

    fep3::ISchedulerService* _scheduler_service = nullptr;
    fep3::IJobRegistry* _job_registry_impl = nullptr;
    fep3::experimental::IClockService* _clock_service = nullptr;
    std::shared_ptr<ClockService> _clock_service_catelyn = std::make_shared<ClockService>();
    fep3::IClockSyncService* _clock_sync_service = nullptr;
    fep3::native::MasterOnDemandClockDiscrete* _clock = nullptr;

    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
    std::shared_ptr<WarningLoggerMock> _logger{};
    std::shared_ptr<LoggingService> _logging_service_mock{};
    std::shared_ptr<ServiceBusComponent> _service_bus{};
    std::shared_ptr<RPCServer> _rpc_server{};
    std::shared_ptr<HealthServiceMock> _health_service{};
    std::shared_ptr<DataRegistryMock> _data_registry{};

    virtual bool hasClockSyncService() const
    {
        return false;
    }

private:
    const fep3::ComponentVersionInfo _dummy_component_version_info{
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};
};

INSTANTIATE_TEST_SUITE_P(TestExecuteOneJobWithReinitialization,
                         SchedulingWithNativeClock,
                         ::testing::Values(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME,
                                           FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

struct SchedulingWithDiscreteClock : public SchedulingWithNativeClock {
    void SetUp() override
    {
        SchedulingWithNativeClock::SetUp();

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
            *(_component_registry->getComponent<fep3::IConfigurationService>()),
            FEP3_CLOCK_SERVICE_MAIN_CLOCK,
            FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
            *(_component_registry->getComponent<fep3::IConfigurationService>()),
            FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_STEP_SIZE,
            static_cast<int64_t>(fep3::Duration{10ms}.count())));
    }
};

struct SchedulingWithContinuousClock : public SchedulingWithNativeClock {
    void SetUp() override
    {
        SchedulingWithNativeClock::SetUp();

        // put a system clock with time that is incremented by the mock and not std::chrono
        auto clock_p = std::make_unique<ExternalClockSimulationMock>();
        _mock_continuous_clock = clock_p.get();
        _clock_service->registerClock(std::make_unique<fep3::native::SystemClock>(
            "sim_clock_with_mock_chrono", std::move(clock_p)));

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
            *(_component_registry->getComponent<fep3::IConfigurationService>()),
            FEP3_CLOCK_SERVICE_MAIN_CLOCK,
            "sim_clock_with_mock_chrono"));

        auto scheduler_name = _scheduler_service->getActiveSchedulerName();
        _scheduler_service->unregisterScheduler(scheduler_name);
        _mock_pool = std::make_unique<fep3::mock::MockThreadPoolExecutor>();

        std::unique_ptr<fep3::catelyn::IScheduler> local_clock_based_scheduler =
            std::make_unique<ClockBasedScheduler>(
                _logger, std::make_unique<TestSchedulerFactory>(*_mock_pool));

        _scheduler_service->registerScheduler(std::move(local_clock_based_scheduler));
    }

    ExternalClockSimulationMock* _mock_continuous_clock;
    std::unique_ptr<fep3::mock::MockThreadPoolExecutor> _mock_pool;
};

struct SchedulingWithDiscreteClockSlave : public SchedulingWithNativeClock {
    void SetUp() override
    {
        using namespace fep3;

        SchedulingWithNativeClock::SetUp();

        ASSERT_FEP3_NOERROR(
            base::setPropertyValue(*(_component_registry->getComponent<IConfigurationService>()),
                                   FEP3_CLOCK_SERVICE_MAIN_CLOCK,
                                   FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE));

        ASSERT_FEP3_NOERROR(
            base::setPropertyValue(*(_component_registry->getComponent<IConfigurationService>()),
                                   FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER,
                                   "timing_master"));

        EXPECT_CALL(*_rpc_server, getName()).Times(1).WillOnce(Return("slave_participant"));

        _rpc_requester = std::make_shared<RPCRequester>();
        EXPECT_CALL(*_service_bus, getRequester("timing_master"))
            .Times(1)
            .WillOnce(Return(_rpc_requester));

        const std::string reply_begin(R"({"id" : 1,"jsonrpc" : "2.0","result" : )");

        const std::string master_type_reply_discrete =
            reply_begin +
            std::to_string(static_cast<int>(fep3::arya::IClock::ClockType::discrete)) + "}";
        EXPECT_CALL(*_rpc_requester, sendRequest(_, ContainsRegex("getMasterType"), _))
            .WillOnce(DoAll(WithArg<2>(Invoke([master_type_reply_discrete](
                                                  fep3::IRPCRequester::IRPCResponse& response) {
                                response.set(master_type_reply_discrete);
                            })),
                            Return(ERR_NOERROR)));

        const std::string result_ok = reply_begin + "0}";
        EXPECT_CALL(*_rpc_requester, sendRequest(_, ContainsRegex("registerSyncSlave"), _))
            .WillRepeatedly(
                DoAll(WithArg<2>(Invoke([result_ok](fep3::IRPCRequester::IRPCResponse& response) {
                          response.set(result_ok);
                      })),
                      Return(ERR_NOERROR)));
    }

    virtual bool hasClockSyncService() const override
    {
        return true;
    }
    std::shared_ptr<RPCRequester> _rpc_requester{};
    fep3::rpc::arya::FarClockUpdater* _clock_updater;
};

/**
 * @brief Tests empty jobs
 *
 */
TEST_F(SchedulingWithContinuousClock, EmptyJobs)
{
    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Tests job execution by the continuous clock
 *
 * @detail The default scheduler (clock_based_scheduler) is driven by the continous clock
 * One job with cycle time 100 ms will be executed until job is executed with >= 400ms
 * After that the the components are restarted
 * And another 400ms will be simulated.
 * @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2468, FEPSDK-2467, FEPSDK-2472
 */
TEST_F(SchedulingWithContinuousClock, ExecuteOneJobWithReset)
{
    const auto job_cycle_time = 20ms;
    _mock_continuous_clock->setTimeStep(job_cycle_time);

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration>(job_cycle_time));
    auto job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    job->setDefaultBehaviour();

    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(builder._job_name, job, builder.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start - stop
    {
        ::test::helper::Notification done;

        // we expect 0ms, 20ms, 40ms
        EXPECT_CALL(*job, execute(Le(40ms)))
            .Times(3)
            .WillRepeatedly(DoAll(InvokeWithoutArgs([&]() { _mock_continuous_clock->increment(); }),
                                  Return(fep3::Result{})));
        // stop at 60ms, do not increment clock further
        EXPECT_CALL(*job, execute(Eq(60ms))).WillOnce(DoAll(Notify(&done), Return(fep3::Result{})));

        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(done.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());

        Mock::VerifyAndClearExpectations(job.get());
    }

    // restart
    {
        ::test::helper::Notification called_400ms;

        // we expect 0ms, 20ms, 40ms
        EXPECT_CALL(*job, execute(Le(40ms)))
            .Times(3)
            .WillRepeatedly(DoAll(InvokeWithoutArgs([&]() { _mock_continuous_clock->increment(); }),
                                  Return(fep3::Result{})));
        // stop at 60ms, do not increment clock further
        EXPECT_CALL(*job, execute(Eq(60ms)))
            .WillOnce(DoAll(Notify(&called_400ms), Return(fep3::Result{})));

        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(called_400ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Tests job execution by the discrete clock
 *
 * @detail The default scheduler (clock_based_scheduler) is driven by the discrete clock.
 * One job with cycle time 10 ms will be executed for a period of 50ms.
 * After that the the components are restarted
 * And another 50ms will be simulated.
 * @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2469, FEPSDK-2467, FEPSDK-2472
 */
TEST_F(SchedulingWithDiscreteClock, ExecuteOneJobWithReset)
{
    const auto job_cycle_time = 10ms;
    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration>(job_cycle_time));
    auto job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    job->setDefaultBehaviour();

    auto configuration_service = _component_registry->getComponent<fep3::IConfigurationService>();
    ASSERT_NE(configuration_service, nullptr);
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
        *configuration_service, FEP3_CLOCK_SERVICE_MAIN_CLOCK, FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(builder._job_name, job, builder.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start - stop
    {
        ::test::helper::Notification called_50ms;

        EXPECT_CALL(*job, execute(_)).Times(AnyNumber());
        EXPECT_CALL(*job, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(30ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(40ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(50ms)))
            .WillOnce(DoAll(Notify(&called_50ms), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(called_50ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());

        Mock::VerifyAndClearExpectations(job.get());
    }

    // restart
    {
        ::test::helper::Notification done;

        EXPECT_CALL(*job, execute(_)).Times(AnyNumber());
        // FYI in contrast to start the 0ms is skipped here => EXPECT_CALL(*job,
        // execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(30ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(40ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(50ms)))
            .WillOnce(DoAll(Notify(&done), Return(fep3::ERR_NOERROR)));
        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(done.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }

    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Tests that for a job the runtime checks are performed
 *
 * @detail Only the integration is tested here. The detailed tests are executed as unit tests.
 * A sleeping job is used. This job runs longer than the configured max runtime.
 * Therefore a logging is executed. Test passes if the log is detected.
 * @req_id FEPSDK-2089
 */
TEST_F(SchedulingWithNativeClock, VerifyJobRuntimeCheckIsExecuted)
{
    using namespace std::chrono_literals;
    using Strategy = fep3::JobConfiguration::TimeViolationStrategy;

    const auto job_cycle_time = 10ms;
    const auto job_sleeping_time = 2us;
    const auto job_max_runtime = 1us;
    const auto job_name = "my_job";

    ASSERT_GT(job_sleeping_time, job_max_runtime);

    const auto job =
        std::make_shared<helper::SleepingJob>(job_name,
                                              duration_cast<fep3::Duration>(job_cycle_time),
                                              duration_cast<fep3::Duration>(job_sleeping_time),
                                              duration_cast<fep3::Duration>(20ms));

    const auto job_configuration =
        fep3::arya::JobConfiguration(duration_cast<fep3::Duration>(job_cycle_time),
                                     duration_cast<fep3::Duration>(0ms),
                                     duration_cast<fep3::Duration>(job_max_runtime),
                                     Strategy::warn_about_runtime_violation);

    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, job, job_configuration));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start (expect runtime violation warning)
    {
        // log is emitted for every job call
        EXPECT_CALL((*_logger), logWarning(_))
            .Times(AtLeast(1))
            .WillRepeatedly(::testing::Return(::fep3::Result{}));

        ASSERT_FEP3_NOERROR(_component_registry->start());

        job->waitForExpectedCallTime(1s);

        ASSERT_FEP3_NOERROR(_component_registry->stop());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
        ASSERT_FEP3_NOERROR(_component_registry->destroy());
    }
}

/**
 * @brief Two jobs will be scheduled discrete
 * @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2469
 */
TEST_F(SchedulingWithDiscreteClock, ExecuteTwoJobs)
{
    const helper::SimpleJobBuilder builder_10ms("my_job_10ms", duration_cast<fep3::Duration>(10ms));
    auto job_10ms = builder_10ms.makeJob<NiceMock<fep3::mock::core::Job>>();
    job_10ms->setDefaultBehaviour();

    const helper::SimpleJobBuilder builder_20ms("my_job_20ms", duration_cast<fep3::Duration>(20ms));
    auto job_20ms = builder_20ms.makeJob<NiceMock<fep3::mock::core::Job>>();
    job_20ms->setDefaultBehaviour();

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
        *(_component_registry->getComponent<fep3::IConfigurationService>()),
        FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_STEP_SIZE,
        static_cast<int64_t>(fep3::Duration{10ms}.count())));

    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(builder_10ms._job_name, job_10ms, builder_10ms.makeJobConfig()));
    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(builder_20ms._job_name, job_20ms, builder_20ms.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start - stop
    {
        ::test::helper::Notification called_10ms;
        ::test::helper::Notification called_20ms;

        EXPECT_CALL(*job_10ms, execute(_)).Times(AnyNumber());
        EXPECT_CALL(*job_10ms, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*job_10ms, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*job_10ms, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*job_10ms, execute(fep3::Timestamp(30ms)))
            .WillOnce(DoAll(Notify(&called_10ms), Return(fep3::ERR_NOERROR)));

        EXPECT_CALL(*job_20ms, execute(_)).Times(AnyNumber());
        EXPECT_CALL(*job_20ms, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*job_20ms, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*job_20ms, execute(fep3::Timestamp(40ms))).Times(1);
        EXPECT_CALL(*job_20ms, execute(fep3::Timestamp(60ms)))
            .WillOnce(DoAll(Notify(&called_20ms), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(_component_registry->start());

        ASSERT_TRUE(called_10ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_TRUE(called_20ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));

        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }

    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Two jobs will be scheduled continuous
 * @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2468
 */
TEST_F(SchedulingWithContinuousClock, ExecuteTwoJobs)
{
    const auto job1_period = 10ms;
    const auto job2_period = 20ms;
    // since will be incremented twice, once for each job
    // we set the increment to half the clock period
    _mock_continuous_clock->setTimeStep(job1_period);

    const helper::SimpleJobBuilder builder_10ms("my_job_10ms", job1_period);
    auto job_10ms = builder_10ms.makeJob<NiceMock<fep3::mock::core::Job>>();
    job_10ms->setDefaultBehaviour();

    const helper::SimpleJobBuilder builder_20ms("my_job_20ms", job2_period);
    auto job_20ms = builder_20ms.makeJob<NiceMock<fep3::mock::core::Job>>();
    job_20ms->setDefaultBehaviour();

    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(builder_10ms._job_name, job_10ms, builder_10ms.makeJobConfig()));
    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(builder_20ms._job_name, job_20ms, builder_20ms.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start - stop
    {
        ::test::helper::Notification called_60ms_job1;
        ::test::helper::Notification called_60ms_job2;
        // 0ms -> 50ms is 6 times
        EXPECT_CALL(*job_10ms, execute(Le(50ms)))
            .Times(6)
            .WillRepeatedly(DoAll(InvokeWithoutArgs([&]() { _mock_continuous_clock->increment(); }),
                                  Return(fep3::Result{})));
        // stop at 60ms, do not increment clock further
        EXPECT_CALL(*job_10ms, execute(Eq(60ms)))
            .WillOnce(DoAll(Notify(&called_60ms_job1), Return(fep3::Result{})));

        // 0ms -> 40ms is 3 times
        EXPECT_CALL(*job_20ms, execute(Le(40ms))).Times(3).WillRepeatedly(Return(fep3::Result{}));
        // stop at , 60ms
        EXPECT_CALL(*job_20ms, execute(Eq(60ms)))
            .WillOnce(DoAll(Notify(&called_60ms_job2), Return(fep3::Result{})));

        ASSERT_FEP3_NOERROR(_component_registry->start());

        ASSERT_TRUE(called_60ms_job1.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_TRUE(called_60ms_job2.waitForNotificationWithTimeout(std::chrono::seconds(1)));

        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }

    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Tests whether it causes negative timestamp if timing master is started earlier than slave,
 *
 * @detail If clock synchronization events are processed before the participant is started, but jobs
 * are added, it can result in anomaly in timer values (i.e. they are put to the past), like in
 * FEPSDK-2860 Two events (one time reset to 0s and one time update to 2s) are sent to the slave
 * before it is started, the expectation is that it does not cause any error/assert/exception.
 */
TEST_F(SchedulingWithDiscreteClockSlave, EventsBeforeStart)
{
    const helper::SimpleJobBuilder builder_100ms("my_job_100ms",
                                                 duration_cast<fep3::Duration>(100ms));
    auto job = builder_100ms.makeJob<NiceMock<fep3::mock::core::Job>>();
    job->setDefaultBehaviour();

    EXPECT_CALL((*_logger), logWarning(_))
        .Times(AtLeast(1))
        .WillRepeatedly(::testing::Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(builder_100ms._job_name, job, builder_100ms.makeJobConfig()));

    EXPECT_CALL(*_rpc_server,
                registerService(fep3::rpc::IRPCClockSyncSlaveDef::getRPCDefaultName(), _))
        .WillOnce(DoAll(
            WithArg<1>(Invoke([&](const std::shared_ptr<fep3::IRPCServer::IRPCService>& service) {
                _clock_updater = dynamic_cast<fep3::rpc::arya::FarClockUpdater*>(service.get());
                ASSERT_TRUE(_clock_updater);
            })),
            Return(fep3::Result())));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    EXPECT_EQ(_clock_updater->syncTimeEvent(
                  static_cast<int>(fep3::rpc::IRPCClockSyncMasterDef::EventID::time_reset),
                  std::to_string(fep3::Timestamp(0).count()),
                  std::to_string(fep3::Timestamp(0).count()),
                  std::to_string(fep3::Timestamp(0).count())),
              "0");

    EXPECT_EQ(_clock_updater->syncTimeEvent(
                  static_cast<int>(fep3::rpc::IRPCClockSyncMasterDef::EventID::time_updating),
                  std::to_string(fep3::Timestamp(2s).count()),
                  std::to_string(fep3::Timestamp(0).count()),
                  ""),
              "0");

    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Tests job execution by the continuous and discrete clock with reinitialization
 *
 * @detail the job execution by the continuous and discrete clock should always be triggered with
 * positive timestamp after reinitialization.
 * testflow: start -> stop -> deinitialize -> initialize-> start -> stop
 * This test does not test exact scheduling behavior, so it can be used without mocking
 * the system clock or the thread pool
 * @req_id
 */
TEST_P(SchedulingWithNativeClock, ExecuteOneJobWithReinitialization)
{
    const auto job_cycle_time = 100ms;

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration>(job_cycle_time));
    auto job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
    job->setDefaultBehaviour();
    auto configuration_service = _component_registry->getComponent<fep3::IConfigurationService>();
    ASSERT_NE(configuration_service, nullptr);
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
        *configuration_service, FEP3_CLOCK_SERVICE_MAIN_CLOCK, GetParam()));

    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(builder._job_name, job, builder.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    ::test::helper::Notification called_100ms_job;

    // Start - stop
    {
        EXPECT_CALL(*job, execute(_)).Times(AnyNumber());
        // Job should always be triggered with positive timestamp
        EXPECT_CALL(*job, execute(Lt(fep3::Timestamp(0)))).Times(0);
        EXPECT_CALL(*job, execute(Ge(fep3::Timestamp(400ms))))
            .WillRepeatedly(DoAll(Notify(&called_100ms_job), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(called_100ms_job.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());

        Mock::VerifyAndClearExpectations(job.get());
    }

    // Deinitialize - initialize - start - stop
    {
        EXPECT_CALL(*job, execute(_)).Times(AnyNumber());
        // Job should always be triggered with positive timestamp
        EXPECT_CALL(*job, execute(Lt(fep3::Timestamp(0)))).Times(0);
        EXPECT_CALL(*job, execute(Ge(fep3::Timestamp(400ms))))
            .WillRepeatedly(DoAll(Notify(&called_100ms_job), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(called_100ms_job.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }

    // Deinitialize - initialize - start - stop
    {
        EXPECT_CALL(*job, execute(_)).Times(AnyNumber());
        // Job should always be triggered with positive timestamp
        EXPECT_CALL(*job, execute(Lt(fep3::Timestamp(0)))).Times(0);
        EXPECT_CALL(*job, execute(Ge(fep3::Timestamp(400ms))))
            .WillRepeatedly(DoAll(Notify(&called_100ms_job), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(called_100ms_job.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }

    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}
