/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/job_registry/mock_job.h>
#include <fep3/components/job_registry/mock_job_registry.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/native_components/health/health_service.h>

#include <common/gtest_asserts.h>

using namespace ::testing;
using namespace fep3;

enum class JobType
{
    ClockTriggered,
    DataTriggered
};

struct JobHealthRegistryMock : public fep3::native::IJobHealthRegistry

{
    MOCK_METHOD(void,
                initialize,
                (const fep3::Jobs&, std::shared_ptr<fep3::arya::ILogger>),
                (override));
    MOCK_METHOD(void, deinitialize, (), (override));
    MOCK_METHOD(Result, resetHealth, (), (override));
    MOCK_METHOD(Result,
                updateJobStatus,
                (const std::string&, const fep3::IHealthService::JobExecuteResult&),
                (override));
    MOCK_METHOD(std::vector<fep3::native::IJobHealthRegistry::JobHealthiness>,
                getHealth,
                (),
                (const, override));
};

struct HealthServiceTest : public ::testing::Test {
    using JobRegistryComponent = NiceMock<fep3::mock::JobRegistry>;
    using LoggingService = NiceMock<fep3::mock::LoggingService>;

protected:
    HealthServiceTest()
        : _component_registry{std::make_shared<fep3::ComponentRegistry>()},
          _job_registry_mock{std::make_unique<JobRegistryComponent>()},
          _logger_mock{std::make_shared<LoggingService>()}
    {
        auto job_health_registry_mock = std::make_unique<StrictMock<JobHealthRegistryMock>>();
        _job_health_registry_mock = job_health_registry_mock.get();
        _health_service =
            std::make_shared<fep3::native::HealthService>(std::move(job_health_registry_mock));
    }

    void SetUp() override
    {
        createJobs();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            _job_registry_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IHealthService>(
            _health_service, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            _logger_mock, _dummy_component_version_info));

        _component_registry->create();
    }

    void createJobs()
    {
        auto job_mock{std::make_shared<NiceMock<fep3::mock::Job>>()};
        auto job_mock2{std::make_shared<NiceMock<fep3::mock::Job>>()};
        auto job_mock3{std::make_shared<NiceMock<fep3::mock::Job>>()};
        _jobs.emplace(
            "job1",
            JobEntry{std::shared_ptr<IJob>{job_mock}, JobInfo{"job1", fep3::Duration{100}}});
        _jobs.emplace(
            "job2", JobEntry{std::shared_ptr<IJob>{job_mock}, JobInfo{"job2", fep3::Duration{10}}});
        _jobs.emplace("job3",
                      JobEntry{std::shared_ptr<IJob>{job_mock3},
                               fep3::JobInfo{"job3",
                                             std::make_unique<fep3::DataTriggeredJobConfiguration>(
                                                 std::vector<std::string>{"signal_name"})}});
    }

    void clearJobs()
    {
        _jobs.clear();
    }

    void startComponents()
    {
        // We do not have service bus so these will return an error.
        _component_registry->create();
        _component_registry->initialize();

        EXPECT_CALL(*_job_registry_mock, getJobsCatelyn()).WillOnce(::testing::Return(_jobs));
        if (_job_health_registry_mock) {
            EXPECT_CALL(*_job_health_registry_mock, initialize(_, _));
        }
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->start());
    }

    void stopComponents()
    {
        ASSERT_FEP3_NOERROR(_component_registry->stop());
        if (_job_health_registry_mock) {
            EXPECT_CALL(*_job_health_registry_mock, deinitialize());
        }
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
        ASSERT_FEP3_NOERROR(_component_registry->destroy());
    }

    fep3::IHealthService* getHealthService()
    {
        const auto health_service_intf = _component_registry->getComponent<fep3::IHealthService>();
        return health_service_intf;
    }

    const fep3::ComponentVersionInfo _dummy_component_version_info{"3.1.0", "dummyPath", "3.1.1"};
    fep3::Jobs _jobs;
    JobHealthRegistryMock* _job_health_registry_mock{};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry;
    std::shared_ptr<native::HealthService> _health_service;
    std::shared_ptr<JobRegistryComponent> _job_registry_mock;
    std::shared_ptr<LoggingService> _logger_mock{};
};
