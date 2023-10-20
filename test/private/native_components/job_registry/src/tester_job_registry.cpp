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

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/configuration/mock_configuration_service.h>
#include <fep3/components/job_registry/mock_job.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/native_components/job_registry/local_job_registry.h>

#include <common/gtest_asserts.h>
#include <helper/job_registry_helper.h>

using namespace ::testing;
using namespace fep3;
using namespace fep3::test;
using namespace std::chrono;
using namespace std::chrono_literals;

using LoggerMock = NiceMock<fep3::mock::LoggerWithDefaultBehavior>;
using LoggingService = fep3::mock::LoggingService;
using ConfigurationServiceComponentMock = StrictMock<fep3::mock::ConfigurationService>;
using ServiceBusComponent = StrictMock<fep3::mock::ServiceBus>;
using ServiceBusComponentMock = StrictMock<fep3::mock::ServiceBus>;
using RPCServerMock = StrictMock<fep3::mock::RPCServer>;

struct JobRegistryWithComponentRegistry
    : TestWithParam<std::tuple<std::shared_ptr<fep3::core::Job>,
                               std::string,
                               std::shared_ptr<fep3::JobConfiguration>>> {
    JobRegistryWithComponentRegistry()
        : _component_registry(std::make_shared<fep3::ComponentRegistry>()),
          _logging_service_mock(std::make_shared<LoggingService>()),
          _configuration_service_mock(std::make_shared<ConfigurationServiceComponentMock>()),
          _service_bus_mock(std::make_shared<ServiceBusComponentMock>()),
          _rpc_server_mock(std::make_shared<RPCServerMock>())
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_service_bus_mock, getServer())
            .Times(1)
            .WillOnce(::testing::Return(_rpc_server_mock));
        EXPECT_CALL(*_rpc_server_mock,
                    registerService(fep3::rpc::IRPCJobRegistryDef::getRPCDefaultName(), _))
            .WillOnce(Return(fep3::Result()));

        createComponents();
        setComponents();

        EXPECT_CALL(*_configuration_service_mock, registerNode(_))
            .Times(1)
            .WillOnce(DoAll(
                WithArg<0>(Invoke([&](const std::shared_ptr<fep3::arya::IPropertyNode>& node) {
                    if (FEP3_JOB_REGISTRY_CONFIG == node->getName()) {
                        _job_registry_property_node = node;
                    }
                })),
                Return(fep3::Result())));

        _logger = std::make_shared<LoggerMock>();
        EXPECT_CALL(*_logging_service_mock, createLogger(_)).WillRepeatedly(Return(_logger));

        ASSERT_FEP3_RESULT(_component_registry->create(), fep3::ERR_NOERROR);
    }

    void createComponents()
    {
        using namespace fep3::native;

        auto job_registry = std::make_unique<JobRegistry>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            std::move(job_registry), _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            _logging_service_mock, _dummy_component_version_info));
    }

    void setComponents()
    {
        _job_registry_impl = _component_registry->getComponent<fep3::IJobRegistry>();
        ASSERT_NE(_job_registry_impl, nullptr);
    }

    void validateJobConfiguration(
        fep3::Duration cycle_time,
        fep3::Duration delay_time,
        fep3::Optional<fep3::Duration> max_runtime_real_time,
        fep3::JobConfiguration::TimeViolationStrategy runtime_violation_strategy,
        std::shared_ptr<IPropertyNode> job_property_entry)
    {
        EXPECT_EQ(FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY,
                  base::getPropertyValue<std::string>(
                      *job_property_entry->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)));
        EXPECT_EQ(cycle_time.count(),
                  base::getPropertyValue<int32_t>(
                      *job_property_entry->getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)));
        EXPECT_EQ(delay_time.count(),
                  base::getPropertyValue<int32_t>(
                      *job_property_entry->getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY)));
        if (max_runtime_real_time.has_value()) {
            EXPECT_EQ(max_runtime_real_time.value().count(),
                      base::getPropertyValue<int32_t>(
                          *job_property_entry->getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)));
        }
        else {
            ASSERT_FALSE(base::getPropertyValue<int32_t>(
                *job_property_entry->getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)));
        }
        EXPECT_EQ(
            runtime_violation_strategy,
            fep3::JobConfiguration::fromString(base::getPropertyValue<std::string>(
                *job_property_entry->getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY))));
        EXPECT_EQ(std::vector<std::string>{},
                  base::getPropertyValue<std::vector<std::string>>(
                      *job_property_entry->getChild(FEP3_JOB_TRIGGER_SIGNAL_PROPERTY)));
    }

    void validateClockJobConfiguration(
        fep3::Duration cycle_time,
        fep3::Duration delay_time,
        fep3::Optional<fep3::Duration> max_runtime_real_time,
        fep3::JobConfiguration::TimeViolationStrategy runtime_violation_strategy,
        std::shared_ptr<IPropertyNode> job_property_entry)
    {
        validateJobConfiguration(cycle_time,
                                 delay_time,
                                 max_runtime_real_time,
                                 runtime_violation_strategy,
                                 job_property_entry);
    }

    void validateDataJobConfiguration(
        const std::vector<std::string>& signal_names,
        fep3::Optional<fep3::Duration> max_runtime_real_time,
        fep3::JobConfiguration::TimeViolationStrategy runtime_violation_strategy,
        std::shared_ptr<IPropertyNode> job_property_entry)
    {
        EXPECT_EQ(FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY,
                  base::getPropertyValue<std::string>(
                      *job_property_entry->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)));
        EXPECT_EQ(signal_names,
                  base::getPropertyValue<std::vector<std::string>>(
                      *job_property_entry->getChild(FEP3_JOB_TRIGGER_SIGNAL_PROPERTY)));
        if (max_runtime_real_time.has_value()) {
            EXPECT_EQ(max_runtime_real_time.value().count(),
                      base::getPropertyValue<int32_t>(
                          *job_property_entry->getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)));
        }
        else {
            ASSERT_FALSE(base::getPropertyValue<int32_t>(
                *job_property_entry->getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)));
        }
        EXPECT_EQ(
            runtime_violation_strategy,
            fep3::JobConfiguration::fromString(base::getPropertyValue<std::string>(
                *job_property_entry->getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY))));
        EXPECT_EQ(0,
                  base::getPropertyValue<int32_t>(
                      *job_property_entry->getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)));
        EXPECT_EQ(0,
                  base::getPropertyValue<int32_t>(
                      *job_property_entry->getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY)));
    }

    void validaReconfiguration(const std::string& job_name,
                               const std::string& clock_job_name,
                               const std::string& data_job_name,
                               const fep3::arya::JobConfiguration& expected_job_config,
                               const ClockTriggeredJobConfiguration& expected_clock_job_config,
                               const DataTriggeredJobConfiguration& expected_data_job_config)
    {
        const auto configured_job_info =
            _job_registry_impl->getJobs().at(job_name).job_info.getConfig();
        const auto clock_job_info_clone =
            _job_registry_impl->getJobsCatelyn().at(clock_job_name).job_info.getConfigCopy();
        const auto data_job_info_clone =
            _job_registry_impl->getJobsCatelyn().at(data_job_name).job_info.getConfigCopy();

        const auto configured_clock_job_info =
            dynamic_cast<const catelyn::ClockTriggeredJobConfiguration&>(*clock_job_info_clone);
        const auto configured_data_job_info =
            dynamic_cast<const catelyn::DataTriggeredJobConfiguration&>(*data_job_info_clone);

        EXPECT_THAT(expected_job_config,
                    fep3::mock::arya::JobConfigurationMatcher(configured_job_info));
        EXPECT_THAT(expected_clock_job_config,
                    fep3::mock::JobConfigurationMatcher(configured_clock_job_info));
        EXPECT_THAT(expected_data_job_config,
                    fep3::mock::JobConfigurationMatcher(configured_data_job_info));
    }

    helper::SimpleJobBuilder _job;
    fep3::IJobRegistry* _job_registry_impl{nullptr};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry;
    std::shared_ptr<LoggerMock> _logger;
    std::shared_ptr<LoggingService> _logging_service_mock{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
    std::shared_ptr<ServiceBusComponent> _service_bus_mock{nullptr};
    std::shared_ptr<RPCServerMock> _rpc_server_mock{};
    std::shared_ptr<fep3::IPropertyNode> _job_registry_property_node;

protected:
    const fep3::ComponentVersionInfo _dummy_component_version_info{
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};
};

/**
 * @brief All states of the JobRegistry are iterated thru
 */
TEST_F(JobRegistryWithComponentRegistry, IterateAllStates)
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
 * @detail Test whether the job registry default configuration is correct after creation.
 * This requires the following nodes to be available:
 * * FEP3_JOB_REGISTRY_CONFIG
 * * FEP3_JOB_REGISTRY_JOBS
 */
TEST_F(JobRegistryWithComponentRegistry, DefaultConfiguration)
{
    EXPECT_TRUE(_job_registry_property_node);
    EXPECT_TRUE(_job_registry_property_node->getChild(FEP3_JOBS_PROPERTY));
}

/**
 * @brief Functional smoke test of the JobRegistry
 * @req_id FEPSDK-2085, FEPSDK-2086, FEPSDK-2087
 */
TEST_F(JobRegistryWithComponentRegistry, FunctionalSmokeTest)
{
    // actual test
    {
        auto job = _job.makeJob();
        auto job_config = _job.makeJobConfig();
        auto job_name = _job._job_name;

        ASSERT_EQ(_job_registry_impl->getJobInfos().size(), 0);
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));
        ASSERT_EQ(_job_registry_impl->getJobInfos().size(), 1);
        ASSERT_EQ(_job_registry_impl->getJobInfosCatelyn().size(), 1);
        ASSERT_FEP3_NOERROR(_job_registry_impl->removeJob(job_name));
        ASSERT_EQ(_job_registry_impl->getJobInfos().size(), 0);
        ASSERT_EQ(_job_registry_impl->getJobInfosCatelyn().size(), 0);
    }

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Test to validate the configuration of job to be registered
 * @req_id FEPSDK-2098, FEPSDK-2166, FEPSDK-2165, FEPSDK-2167, FEPSDK-2284
 */
TEST_F(JobRegistryWithComponentRegistry, JobConfiguration)
{
    auto job = _job.makeJob();
    auto clock_job = _job.makeClockJob();
    auto data_job = _job.makeDataJob();

    {
        const auto job_name = "my_job";
        const auto clock_job_name = "my_clock_job";
        const auto data_job_name = "my_data_job";
        const auto signal_names = std::vector<std::string>{"signal_name"};

        const auto cycle_time = 10ms;
        const auto first_delay_sim_time = 1ms;
        const auto max_runtime_real_time = fep3::Optional<fep3::Duration>(100ms);
        const auto runtime_violation_strategy =
            fep3::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation;
        auto job_config = fep3::arya::JobConfiguration(
            cycle_time, first_delay_sim_time, max_runtime_real_time, runtime_violation_strategy);
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));
        ASSERT_FEP3_NOERROR(
            _job_registry_impl->addJob(clock_job_name,
                                       std::move(clock_job),
                                       fep3::ClockTriggeredJobConfiguration(job_config)));
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(
            data_job_name,
            std::move(data_job),
            fep3::DataTriggeredJobConfiguration(
                signal_names, max_runtime_real_time, runtime_violation_strategy)));
    }

    // use seconds for durations
    {
        const auto job_name = "my_second_job";
        const auto clock_job_name = "my_second_clock_job";
        const auto data_job_name = "my_second_data_job";
        const auto signal_names = std::vector<std::string>{"signal_name"};

        const auto cycle_time = 1s;
        const auto first_delay_sim_time = 1s;
        const auto max_runtime_real_time = fep3::Optional<fep3::Duration>(1s);
        const auto runtime_violation_strategy =
            fep3::JobConfiguration::TimeViolationStrategy::skip_output_publish;
        auto job_config = fep3::arya::JobConfiguration(
            cycle_time, first_delay_sim_time, max_runtime_real_time, runtime_violation_strategy);

        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(
            clock_job_name, std::move(job), fep3::ClockTriggeredJobConfiguration(job_config)));
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(
            data_job_name,
            std::move(job),
            fep3::DataTriggeredJobConfiguration(
                signal_names, max_runtime_real_time, runtime_violation_strategy)));
    }
}

/**
 * @brief Test whether a job property entry is created/removed if a job is added/removed to/from the
 * job registry
 */
TEST_F(JobRegistryWithComponentRegistry, AddRemoveJobPropertyEntry)
{
    const auto job_name = "my_job";
    const auto clock_job_name = "my_clock_job";
    const auto data_job_name = "my_data_job";

    EXPECT_FALSE(_job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(job_name));
    EXPECT_FALSE(
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(clock_job_name));
    EXPECT_FALSE(
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(data_job_name));

    auto job = _job.makeJob();
    auto clock_job = _job.makeClockJob();
    auto data_job = _job.makeDataJob();

    const auto cycle_time = 10ms;
    const auto first_delay_sim_time = 1ms;
    const auto max_runtime_real_time = fep3::arya::Optional<Duration>(100ms);
    const auto runtime_violation_strategy =
        fep3::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation;
    const auto signal_names = std::vector<std::string>{"my_signal"};

    auto job_config = fep3::arya::JobConfiguration(
        cycle_time, first_delay_sim_time, max_runtime_real_time, runtime_violation_strategy);

    // add job
    {
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));
        const auto job_property_entry =
            _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(job_name);
        ASSERT_TRUE(job_property_entry);
        validateJobConfiguration(cycle_time,
                                 first_delay_sim_time,
                                 max_runtime_real_time,
                                 runtime_violation_strategy,
                                 job_property_entry);

        ASSERT_FEP3_NOERROR(
            _job_registry_impl->addJob(clock_job_name,
                                       std::move(clock_job),
                                       fep3::ClockTriggeredJobConfiguration(job_config)));
        const auto clock_job_property_entry =
            _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(clock_job_name);
        ASSERT_TRUE(clock_job_property_entry);
        validateClockJobConfiguration(cycle_time,
                                      first_delay_sim_time,
                                      max_runtime_real_time,
                                      runtime_violation_strategy,
                                      clock_job_property_entry);

        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(
            data_job_name,
            std::move(data_job),
            fep3::DataTriggeredJobConfiguration(
                signal_names, max_runtime_real_time, runtime_violation_strategy)));
        const auto data_job_property_entry =
            _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(data_job_name);
        ASSERT_TRUE(data_job_property_entry);
        validateDataJobConfiguration(signal_names,
                                     max_runtime_real_time,
                                     runtime_violation_strategy,
                                     data_job_property_entry);
    }

    // remove job
    {
        ASSERT_FEP3_NOERROR(_job_registry_impl->removeJob(job_name));
        EXPECT_FALSE(_job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(job_name));

        ASSERT_FEP3_NOERROR(_job_registry_impl->removeJob(clock_job_name));
        EXPECT_FALSE(
            _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(clock_job_name));

        ASSERT_FEP3_NOERROR(_job_registry_impl->removeJob(data_job_name));
        EXPECT_FALSE(
            _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(data_job_name));
    }

    // add job
    {
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));
        const auto job_property_entry =
            _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(job_name);
        ASSERT_TRUE(job_property_entry);
        validateJobConfiguration(cycle_time,
                                 first_delay_sim_time,
                                 max_runtime_real_time,
                                 runtime_violation_strategy,
                                 job_property_entry);

        ASSERT_FEP3_NOERROR(
            _job_registry_impl->addJob(clock_job_name,
                                       std::move(clock_job),
                                       fep3::ClockTriggeredJobConfiguration(job_config)));
        const auto clock_job_property_entry =
            _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(clock_job_name);
        ASSERT_TRUE(clock_job_property_entry);
        validateClockJobConfiguration(cycle_time,
                                      first_delay_sim_time,
                                      max_runtime_real_time,
                                      runtime_violation_strategy,
                                      clock_job_property_entry);

        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(
            data_job_name,
            std::move(data_job),
            fep3::DataTriggeredJobConfiguration(
                signal_names, max_runtime_real_time, runtime_violation_strategy)));
        const auto data_job_property_entry =
            _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(data_job_name);
        ASSERT_TRUE(data_job_property_entry);
        validateDataJobConfiguration(signal_names,
                                     max_runtime_real_time,
                                     runtime_violation_strategy,
                                     data_job_property_entry);
    }
}

/**
 * @brief Tests that adding a job while in RUNNING is not possible
 * @req_id FEPSDK-2100
 */
TEST_P(JobRegistryWithComponentRegistry, AddFailsInRunning)
{
    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    // actual test
    {
        auto [job, job_name, job_config] = GetParam();

        EXPECT_CALL((*_logger), logError(_)).WillOnce(::testing::Return(::fep3::ERR_FAILED));
        ASSERT_FEP3_RESULT(_job_registry_impl->addJob(job_name, std::move(job), *job_config),
                           fep3::ERR_INVALID_STATE);
    }

    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Tests that removing a job after initialization is not possible
 * @req_id FEPSDK-2101
 */
TEST_F(JobRegistryWithComponentRegistry, RemoveFailsInRunning)
{
    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    // actual test
    {
        EXPECT_CALL((*_logger), logError(_)).WillOnce(::testing::Return(::fep3::ERR_FAILED));

        ASSERT_FEP3_RESULT(_job_registry_impl->removeJob("not_existing"), fep3::ERR_INVALID_STATE);
    }

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Tests that a log is issued if an error ocurrs on add or remove
 * @req_id FEPSDK-2085, FEPSDK-2086
 */
TEST_P(JobRegistryWithComponentRegistry, LogOnAddOrRemoveError)
{
    // add
    {
        auto [job, job_name, job_config] = GetParam();

        EXPECT_CALL((*_logger), logError(_)).WillOnce(::testing::Return(::fep3::ERR_FAILED));

        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, job, *job_config));
        ASSERT_FEP3_RESULT(_job_registry_impl->addJob(job_name, job, *job_config),
                           fep3::ERR_RESOURCE_IN_USE);
        ASSERT_FEP3_NOERROR(_job_registry_impl->removeJob(job_name));
    }

    // remove
    {
        EXPECT_CALL((*_logger), logError(_)).WillOnce(::testing::Return(::fep3::ERR_FAILED));

        ASSERT_FEP3_RESULT(_job_registry_impl->removeJob("not_existing"), fep3::ERR_NOT_FOUND);
    }

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

namespace {

fep3::Result reconfigureJobPropertyNodeByConfig(
    fep3::IPropertyNode& job_node, const fep3::arya::JobConfiguration& job_configuration)
{
    FEP3_RETURN_IF_FAILED(
        job_node.getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)
            ->setValue(std::to_string(job_configuration._cycle_sim_time.count())));
    FEP3_RETURN_IF_FAILED(
        job_node.getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY)
            ->setValue(std::to_string(job_configuration._delay_sim_time.count())));
    FEP3_RETURN_IF_FAILED(
        job_node.getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)
            ->setValue(std::to_string(job_configuration._max_runtime_real_time.value().count())));
    FEP3_RETURN_IF_FAILED(job_node.getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY)
                              ->setValue(fep3::JobConfiguration::toString(
                                  job_configuration._runtime_violation_strategy)));

    return {};
}

fep3::Result reconfigureJobPropertyNodeByConfig(
    fep3::IPropertyNode& job_node, const ClockTriggeredJobConfiguration& job_configuration)
{
    FEP3_RETURN_IF_FAILED(
        job_node.getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)
            ->setValue(std::to_string(job_configuration._cycle_sim_time.count())));
    FEP3_RETURN_IF_FAILED(
        job_node.getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY)
            ->setValue(std::to_string(job_configuration._delay_sim_time.count())));

    FEP3_RETURN_IF_FAILED(
        job_node.getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)
            ->setValue(std::to_string(job_configuration._max_runtime_real_time.value().count())));
    FEP3_RETURN_IF_FAILED(job_node.getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY)
                              ->setValue(fep3::JobConfiguration::toString(
                                  job_configuration._runtime_violation_strategy)));

    return {};
}

fep3::Result reconfigureJobPropertyNodeByConfig(
    fep3::IPropertyNode& job_node, const DataTriggeredJobConfiguration& job_configuration)
{
    FEP3_RETURN_IF_FAILED(
        job_node.getChild(FEP3_JOB_TRIGGER_SIGNAL_PROPERTY)
            ->setValue(base::DefaultPropertyTypeConversion<std::vector<std::string>>::toString(
                job_configuration._signal_names)));
    FEP3_RETURN_IF_FAILED(
        job_node.getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)
            ->setValue(std::to_string(job_configuration._max_runtime_real_time.value().count())));
    FEP3_RETURN_IF_FAILED(job_node.getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY)
                              ->setValue(fep3::JobConfiguration::toString(
                                  job_configuration._runtime_violation_strategy)));

    return {};
}

/**
 * @brief Test whether the trigger type of a registered job may be reconfigured successfully using
 * the job property configuration.
 *
 * @req_id TODO
 */
TEST_F(JobRegistryWithComponentRegistry, ReconfigureJobsTriggerType)
{
    const auto clock_job_name = "my_clock_job";
    const auto data_job_name = "my_data_job";
    const auto signal_names = std::vector<std::string>{"my_signal"};

    helper::SimpleJobBuilder jb2{clock_job_name};
    helper::SimpleJobBuilder jb3{data_job_name, signal_names};

    auto clock_job = jb2.makeClockJob();
    auto data_job = jb3.makeDataJob();

    auto clock_job_config = jb2.makeClockJobConfig();
    auto data_job_config = jb3.makeDataJobConfig();

    const ClockTriggeredJobConfiguration clock_job_config_reconfiguration =
        ClockTriggeredJobConfiguration{
            100ms,
            20ms,
            fep3::Optional<Duration>(50ms),
            fep3::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation};

    const DataTriggeredJobConfiguration data_job_config_reconfiguration =
        DataTriggeredJobConfiguration{
            signal_names,
            fep3::Optional<Duration>(80ms),
            fep3::JobConfiguration::TimeViolationStrategy::skip_output_publish};

    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(clock_job_name, std::move(clock_job), clock_job_config));
    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(data_job_name, std::move(data_job), data_job_config));

    const auto clock_job_property_entry =
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(clock_job_name);
    const auto data_job_property_entry =
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(data_job_name);
    ASSERT_TRUE(clock_job_property_entry);
    ASSERT_TRUE(data_job_property_entry);

    // reconfigure a clock triggered job to data triggered job
    ASSERT_FEP3_NOERROR(clock_job_property_entry->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)
                            ->setValue(FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY));
    ASSERT_FEP3_NOERROR(reconfigureJobPropertyNodeByConfig(*clock_job_property_entry,
                                                           data_job_config_reconfiguration));

    // reconfigure data triggered job to clock triggered job
    ASSERT_FEP3_NOERROR(data_job_property_entry->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)
                            ->setValue(FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY));
    ASSERT_FEP3_NOERROR(reconfigureJobPropertyNodeByConfig(*data_job_property_entry,
                                                           clock_job_config_reconfiguration));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    const auto clock_job_info_clone =
        _job_registry_impl->getJobsCatelyn().at(clock_job_name).job_info.getConfigCopy();
    const auto data_job_info_clone =
        _job_registry_impl->getJobsCatelyn().at(data_job_name).job_info.getConfigCopy();

    // Note: cast them to the other ones, they are now reconfigured
    const auto configured_clock_job_info =
        dynamic_cast<const catelyn::DataTriggeredJobConfiguration&>(*clock_job_info_clone);
    const auto configured_data_job_info =
        dynamic_cast<const catelyn::ClockTriggeredJobConfiguration&>(*data_job_info_clone);

    EXPECT_THAT(data_job_config_reconfiguration,
                fep3::mock::JobConfigurationMatcher(configured_clock_job_info));
    EXPECT_THAT(clock_job_config_reconfiguration,
                fep3::mock::JobConfigurationMatcher(configured_data_job_info));

    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}
} // namespace

/**
 * @brief Test whether a registered job is reconfigured successfully on initialization
 * if its configuration properties are adapted and no max runtime is configured.
 *
 * @req_id TODO
 */
TEST_F(JobRegistryWithComponentRegistry, ReconfigureJobsByTimingConfigNoMaxRuntime)
{
    const auto job_name = "my_job";
    const auto clock_job_name = "my_clock_job";
    const auto data_job_name = "my_data_job";
    const auto signal_names = std::vector<std::string>{"my_signal"};

    helper::SimpleJobBuilder jb1{job_name};
    helper::SimpleJobBuilder jb2{clock_job_name};
    helper::SimpleJobBuilder jb3{data_job_name, signal_names};

    auto job = jb1.makeJob();
    auto clock_job = jb2.makeClockJob();
    auto data_job = jb3.makeDataJob();

    auto job_config = jb1.makeJobConfig();
    auto clock_job_config = jb2.makeClockJobConfig();
    auto data_job_config = jb3.makeDataJobConfig();

    const fep3::arya::JobConfiguration job_config_reconfiguration{
        1ms,
        0ms,
        fep3::Optional<Duration>(0ms), // 0 value will not overwrite the config
        fep3::JobConfiguration::TimeViolationStrategy::warn_about_runtime_violation};

    const ClockTriggeredJobConfiguration clock_job_config_reconfiguration =
        ClockTriggeredJobConfiguration{job_config_reconfiguration};

    const DataTriggeredJobConfiguration data_job_config_reconfiguration =
        DataTriggeredJobConfiguration{
            signal_names,
            fep3::Optional<Duration>(0ms), // 0 value will not overwrite the config
            fep3::JobConfiguration::TimeViolationStrategy::skip_output_publish};

    const fep3::arya::JobConfiguration expected_job_config_reconfigured{
        1ms, 0ms, {}, fep3::JobConfiguration::TimeViolationStrategy::warn_about_runtime_violation};

    const ClockTriggeredJobConfiguration expected_clock_job_config_reconfigured =
        ClockTriggeredJobConfiguration{expected_job_config_reconfigured};

    const DataTriggeredJobConfiguration expected_data_job_config_reconfigured =
        DataTriggeredJobConfiguration{
            signal_names, {}, fep3::JobConfiguration::TimeViolationStrategy::skip_output_publish};

    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));
    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(clock_job_name, std::move(clock_job), clock_job_config));
    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(data_job_name, std::move(data_job), data_job_config));

    const auto job_property_entry =
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(job_name);
    const auto clock_job_property_entry =
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(clock_job_name);
    const auto data_job_property_entry =
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(data_job_name);
    ASSERT_TRUE(job_property_entry);
    ASSERT_TRUE(clock_job_property_entry);
    ASSERT_TRUE(data_job_property_entry);

    ASSERT_FEP3_NOERROR(
        reconfigureJobPropertyNodeByConfig(*job_property_entry, job_config_reconfiguration));
    EXPECT_EQ(clock_job_property_entry->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)->getValue(),
              FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY);
    ASSERT_FEP3_NOERROR(reconfigureJobPropertyNodeByConfig(*clock_job_property_entry,
                                                           clock_job_config_reconfiguration));
    EXPECT_EQ(data_job_property_entry->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)->getValue(),
              FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY);
    ASSERT_FEP3_NOERROR(reconfigureJobPropertyNodeByConfig(*data_job_property_entry,
                                                           data_job_config_reconfiguration));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    validaReconfiguration(job_name,
                          clock_job_name,
                          data_job_name,
                          expected_job_config_reconfigured,
                          expected_clock_job_config_reconfigured,
                          expected_data_job_config_reconfigured);

    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Test whether registered jobs are reconfigured successfully on initialization
 * if its configuration properties are adapted and a max runtime is configured.
 *
 * @req_id TODO
 */
TEST_F(JobRegistryWithComponentRegistry, ReconfigureJobsByTimingConfigMaxRuntime)
{
    const auto job_name = "my_job";
    const auto clock_job_name = "my_clock_job";
    const auto data_job_name = "my_data_job";
    const auto signal_names = std::vector<std::string>{"my_signal"};

    helper::SimpleJobBuilder jb1{job_name};
    helper::SimpleJobBuilder jb2{clock_job_name};
    helper::SimpleJobBuilder jb3{data_job_name, signal_names};

    auto job = jb1.makeJob();
    auto clock_job = jb2.makeClockJob();
    auto data_job = jb3.makeDataJob();

    auto job_config = jb1.makeJobConfig();
    auto clock_job_config = jb2.makeClockJobConfig();
    auto data_job_config = jb3.makeDataJobConfig();

    fep3::arya::JobConfiguration job_config_reconfiguration{
        50ms,
        50ms,
        fep3::Optional<Duration>(50ms),
        fep3::arya::JobConfiguration::TimeViolationStrategy::skip_output_publish};

    const ClockTriggeredJobConfiguration clock_job_config_reconfiguration =
        ClockTriggeredJobConfiguration{job_config_reconfiguration};

    const DataTriggeredJobConfiguration data_job_config_reconfiguration =
        DataTriggeredJobConfiguration{
            signal_names,
            fep3::Optional<Duration>(50ms),
            fep3::JobConfiguration::TimeViolationStrategy::skip_output_publish};

    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));
    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(clock_job_name, std::move(clock_job), clock_job_config));
    ASSERT_FEP3_NOERROR(
        _job_registry_impl->addJob(data_job_name, std::move(data_job), data_job_config));

    const auto job_property_entry =
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(job_name);
    const auto clock_job_property_entry =
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(clock_job_name);
    const auto data_job_property_entry =
        _job_registry_property_node->getChild(FEP3_JOBS_PROPERTY)->getChild(data_job_name);
    ASSERT_TRUE(job_property_entry);
    ASSERT_TRUE(clock_job_property_entry);
    ASSERT_TRUE(data_job_property_entry);

    ASSERT_FEP3_NOERROR(
        reconfigureJobPropertyNodeByConfig(*job_property_entry, job_config_reconfiguration));
    EXPECT_EQ(clock_job_property_entry->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)->getValue(),
              FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY);
    ASSERT_FEP3_NOERROR(reconfigureJobPropertyNodeByConfig(*clock_job_property_entry,
                                                           clock_job_config_reconfiguration));
    EXPECT_EQ(data_job_property_entry->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)->getValue(),
              FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY);
    ASSERT_FEP3_NOERROR(reconfigureJobPropertyNodeByConfig(*data_job_property_entry,
                                                           data_job_config_reconfiguration));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    validaReconfiguration(job_name,
                          clock_job_name,
                          data_job_name,
                          job_config_reconfiguration,
                          clock_job_config_reconfiguration,
                          data_job_config_reconfiguration);

    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

TEST_F(JobRegistryWithComponentRegistry, JobRegistry_hasCatelynAndAryaInterfaces)
{
    _component_registry->clear();
    auto job_registry = std::make_shared<fep3::native::JobRegistry>();

    ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::arya::IJobRegistry>(
        (job_registry), _dummy_component_version_info));
    ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::catelyn::IJobRegistry>(
        (job_registry), _dummy_component_version_info));

    auto catelyn_job_registry = _component_registry->getComponent<fep3::catelyn::IJobRegistry>();
    ASSERT_NE(catelyn_job_registry, nullptr);

    auto arya_job_registry = _component_registry->getComponent<fep3::arya::IJobRegistry>();
    ASSERT_NE(arya_job_registry, nullptr);
}

helper::SimpleJobBuilder job;

INSTANTIATE_TEST_SUITE_P(
    JobRegistryWithIJobConfigurationTests,
    JobRegistryWithComponentRegistry,
    testing::Values(std::make_tuple(job.makeJob(),
                                    job._job_name,
                                    std::make_shared<fep3::ClockTriggeredJobConfiguration>(
                                        job.makeClockJobConfig())),
                    std::make_tuple(job.makeJob(),
                                    job._job_name,
                                    std::make_shared<fep3::DataTriggeredJobConfiguration>(
                                        job.makeDataJobConfig()))));
