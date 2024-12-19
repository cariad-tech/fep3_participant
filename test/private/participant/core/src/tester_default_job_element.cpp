/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/base/mock_components.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/components/configuration/mock_configuration_service.h>
#include <fep3/components/configuration/mock_property_node.h>
#include <fep3/components/data_registry/mock_data_registry.h>
#include <fep3/components/job_registry/mock_job.h>
#include <fep3/components/job_registry/mock_job_registry.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/core.h>
#include <fep3/participant/mock/mock_custom_job_element.h>
#include <fep3/participant/mock/mock_data_io_container.h>
#include <fep3/participant/mock/mock_default_job.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <common/gtest_asserts.h>
#include <helper/gmock_async_helper.h>

using namespace ::testing;
using namespace std::chrono_literals;
using namespace fep3::core;

using MyJob = NiceMock<fep3::mock::DefaultJob>;
using MyElement = NiceMock<fep3::mock::CustomJobElement>;
using MyDataIOContainer = NiceMock<fep3::mock::DataIOContainer>;
using Components = NiceMock<fep3::mock::Components>;
using ConfigurationService = StrictMock<fep3::mock::ConfigurationService>;
using PropertyNode = StrictMock<fep3::mock::PropertyNode>;
using DataRegistry = StrictMock<fep3::mock::DataRegistry>;
using ClockService = StrictMock<fep3::mock::ClockService>;
using LoggingService = NiceMock<fep3::mock::LoggingService>;
using JobRegistry = NiceMock<fep3::mock::JobRegistry>;
using Logger = NiceMock<fep3::mock::LoggerWithDefaultBehavior>;

using MyDefaultElement = fep3::core::DefaultJobElement<MyElement>;

struct DefaultJobElementTest : public Test {
protected:
    void SetUp() override
    {
        ON_CALL(*components, findComponent(StrEq(ConfigurationService::getComponentIID())))
            .WillByDefault(Return(configuration_service.get()));
        ON_CALL(*components, findComponent(StrEq(JobRegistry::getComponentIID())))
            .WillByDefault(Return(job_registry.get()));
        ON_CALL(*components, findComponent(StrEq(LoggingService::getComponentIID())))
            .WillByDefault(Return(logging_service.get()));
        ON_CALL(*components, findComponent(StrEq(DataRegistry::getComponentIID())))
            .WillByDefault(Return(data_registry.get()));
        ON_CALL(*components, findComponent(StrEq(ClockService::getComponentIID())))
            .WillByDefault(Return(clock_service.get()));
        ON_CALL(*components, findComponent(StrEq(fep3::arya::IClockService::getComponentIID())))
            .WillByDefault(Return(clock_service.get()));
        ON_CALL(*logging_service, createLogger(_)).WillByDefault(Return(logger));
        ON_CALL(*configuration_service, registerNode(_)).WillByDefault(Return(fep3::Result()));
        ON_CALL(*configuration_service, unregisterNode(_)).WillByDefault(Return(fep3::Result()));
        ON_CALL(*job_registry, getJobInfosCatelyn())
            .WillByDefault(
                Return(std::list<fep3::JobInfo>{fep3::JobInfo(job_name, job_config->clone())}));
    }
    void TearDown() override
    {
    }

    void setUpLoadElement()
    {
        EXPECT_CALL(*configuration_service, registerNode(_)).WillRepeatedly(Return(fep3::Result{}));
        EXPECT_CALL(*my_job, registerPropertyVariables()).WillOnce(Return(fep3::Result{}));
        EXPECT_CALL(*my_element, load(_)).WillOnce(Return(fep3::Result{}));
        EXPECT_CALL(*my_element, createJob(_))
            .WillOnce(Return(ByMove(std::make_tuple(fep3::Result{}, my_job, job_config->clone()))));

        EXPECT_CALL(
            *job_registry,
            addJob(_,
                   _,
                   Matcher<const fep3::JobConfiguration&>{fep3::mock::JobConfigurationMatcher(
                       fep3::ClockTriggeredJobConfiguration(100ms))}))
            .WillOnce(Return(fep3::Result{}));
    }

    void setUpInitializeElement()
    {
        EXPECT_CALL(*configuration_service, getNode(_)).WillRepeatedly(Return(property_node));
        EXPECT_CALL(*property_node, getValue()).WillRepeatedly(Return(""));
    }

    std::string element_name = "my_element";
    std::string job_name = "my_job";
    std::unique_ptr<MyElement> my_element = std::make_unique<MyElement>(element_name);
    std::unique_ptr<fep3::ClockTriggeredJobConfiguration> job_config =
        std::make_unique<fep3::ClockTriggeredJobConfiguration>(100ms);
    std::unique_ptr<MyDataIOContainer> my_container = std::make_unique<MyDataIOContainer>();
    std::shared_ptr<MyJob> my_job = std::make_shared<MyJob>(job_name);
    std::shared_ptr<Components> components = std::make_shared<Components>();
    std::shared_ptr<ConfigurationService> configuration_service =
        std::make_shared<ConfigurationService>();
    std::shared_ptr<PropertyNode> property_node = std::make_shared<PropertyNode>();
    std::shared_ptr<JobRegistry> job_registry = std::make_shared<JobRegistry>();
    std::shared_ptr<DataRegistry> data_registry = std::make_shared<DataRegistry>();
    std::shared_ptr<ClockService> clock_service = std::make_shared<ClockService>();
    std::shared_ptr<LoggingService> logging_service = std::make_shared<LoggingService>();
    std::shared_ptr<Logger> logger = std::make_shared<Logger>();
};

TEST_F(DefaultJobElementTest, getTypename_successful)
{
    EXPECT_CALL(*my_element, getTypename()).WillOnce(Return(element_name));
    auto element = MyDefaultElement(std::move(my_element));
    ASSERT_EQ(element.getTypename(), element_name);
}

TEST_F(DefaultJobElementTest, getVersion_successful)
{
    EXPECT_CALL(*my_element, getVersion()).WillOnce(Return("my version"));
    auto element = MyDefaultElement(std::move(my_element));
    ASSERT_EQ(element.getVersion(), "my version");
}

TEST_F(DefaultJobElementTest, loadElement_successful)
{
    setUpLoadElement();
    auto element = MyDefaultElement(std::move(my_element));
    ASSERT_FEP3_NOERROR(element.loadElement(*components));
}

TEST_F(DefaultJobElementTest, initialize_successful)
{
    setUpLoadElement();
    setUpInitializeElement();

    EXPECT_CALL(*my_element, initialize(_)).WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*my_job, initialize(_)).WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*my_container, addToDataRegistry(_, _)).WillOnce(Return(fep3::Result{}));

    auto element = MyDefaultElement(std::move(my_element), std::move(my_container));
    ASSERT_FEP3_NOERROR(element.loadElement(*components));
    ASSERT_FEP3_NOERROR(element.initialize());
}

TEST_F(DefaultJobElementTest, initialize_reconfigureJobConfig_successful)
{
    setUpLoadElement();
    setUpInitializeElement();

    EXPECT_CALL(*my_element, initialize(_)).WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*my_job, initialize(_)).WillOnce(Return(fep3::Result{}));

    // Mock new config

    auto new_cycle = 200ms;

    std::list<fep3::JobInfo> job_infos = {
        fep3::JobInfo(job_name, std::make_unique<fep3::ClockTriggeredJobConfiguration>(new_cycle))};

    EXPECT_CALL(*job_registry, getJobInfosCatelyn()).WillOnce(Return(job_infos));
    EXPECT_CALL(
        *my_job,
        createDataIOs(_,
                      _,
                      Matcher<const fep3::JobConfiguration&>{fep3::mock::JobConfigurationMatcher(
                          fep3::ClockTriggeredJobConfiguration(new_cycle))}))
        .Times(1);
    EXPECT_CALL(*my_container, addToDataRegistry(_, _)).WillOnce(Return(fep3::Result{}));

    auto element = MyDefaultElement(std::move(my_element), std::move(my_container));
    ASSERT_FEP3_NOERROR(element.loadElement(*components));
    ASSERT_FEP3_NOERROR(element.initialize());
}

TEST_F(DefaultJobElementTest, unloadElement_successful)
{
    setUpLoadElement();

    EXPECT_CALL(*my_element, unload(_)).WillOnce(Return());
    EXPECT_CALL(*my_element, destroyJob()).WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*my_job, unregisterPropertyVariables()).WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*configuration_service, unregisterNode(_)).WillRepeatedly(Return(fep3::Result{}));
    EXPECT_CALL(*job_registry, removeJob(my_job->getName())).WillOnce(Return(fep3::Result{}));

    auto element = MyDefaultElement(std::move(my_element), std::move(my_container));
    element.loadElement(*components);
    element.unloadElement();
}

TEST_F(DefaultJobElementTest, deinitialize_successful)
{
    setUpLoadElement();

    EXPECT_CALL(*my_element, deinitialize(_)).WillOnce(Return());
    EXPECT_CALL(*configuration_service, unregisterNode(_)).WillRepeatedly(Return(fep3::Result{}));

    EXPECT_CALL(*my_job, deinitialize());

    EXPECT_CALL(*my_container, removeFromDataRegistry(_)).WillOnce(Return());

    auto element = MyDefaultElement(std::move(my_element), std::move(my_container));
    ASSERT_FEP3_NOERROR(element.loadElement(*components));
    element.deinitialize();
}

TEST_F(DefaultJobElementTest, deinitialize_jobDeinitializeFails)
{
    setUpLoadElement();

    EXPECT_CALL(*my_element, deinitialize(_)).WillOnce(Return());
    EXPECT_CALL(*configuration_service, unregisterNode(_)).WillRepeatedly(Return(fep3::Result{}));

    EXPECT_CALL(*my_job, deinitialize()).WillOnce(Return(fep3::Result(fep3::ERR_FAILED)));
    EXPECT_CALL(*logger, logError(_)).WillOnce(Return(fep3::Result{}));

    EXPECT_CALL(*my_container, removeFromDataRegistry(_)).WillOnce(Return());

    auto element = MyDefaultElement(std::move(my_element), std::move(my_container));
    ASSERT_FEP3_NOERROR(element.loadElement(*components));
    element.deinitialize();
}

TEST_F(DefaultJobElementTest, run_successful)
{
    setUpLoadElement();
    EXPECT_CALL(*my_job, start()).WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*my_element, run()).WillOnce(Return(fep3::Result{}));

    auto element = MyDefaultElement(std::move(my_element));
    ASSERT_FEP3_NOERROR(element.loadElement(*components));
    ASSERT_FEP3_NOERROR(element.run());
}

TEST_F(DefaultJobElementTest, stop_successful)
{
    setUpLoadElement();
    EXPECT_CALL(*my_job, stop());
    EXPECT_CALL(*my_element, stop());

    auto element = MyDefaultElement(std::move(my_element));
    ASSERT_FEP3_NOERROR(element.loadElement(*components));
    element.stop();
}

TEST_F(DefaultJobElementTest, stop_jobStopFails)
{
    setUpLoadElement();
    EXPECT_CALL(*my_job, stop()).WillOnce(Return(fep3::Result(fep3::ERR_FAILED)));
    EXPECT_CALL(*logger, logError(_)).WillOnce(Return(fep3::Result{}));

    EXPECT_CALL(*my_element, stop());

    auto element = MyDefaultElement(std::move(my_element));
    ASSERT_FEP3_NOERROR(element.loadElement(*components));
    element.stop();
}
