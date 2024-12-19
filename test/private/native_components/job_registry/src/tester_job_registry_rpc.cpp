/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../../utils/common/gtest_asserts.h"

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/configuration/mock_configuration_service.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/core/job.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/native_components/job_registry/local_job_registry.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>

#include <test_job_registry_client_stub.h>

namespace fep3 {
namespace test {
namespace env {

using namespace ::testing;

using LoggingServiceMock = mock::LoggingService;
using LoggerMock = NiceMock<mock::LoggerWithDefaultBehavior>;
using ConfigurationServiceComponentMock = NiceMock<fep3::mock::ConfigurationService>;

class TestClient : public rpc::RPCServiceClient<::test::rpc_stubs::TestJobRegistryClientStub,
                                                rpc::IRPCJobRegistryDef> {
private:
    typedef RPCServiceClient<TestJobRegistryClientStub, rpc::IRPCJobRegistryDef> base_type;

public:
    using base_type::GetStub;

    TestClient(const std::string& server_object_name,
               const std::shared_ptr<IRPCRequester>& rpc_requester)
        : base_type(server_object_name, rpc_requester)
    {
    }
};

struct NativeJobRegistryRPC : public Test {
    NativeJobRegistryRPC()
        : _component_registry(std::make_shared<ComponentRegistry>()),
          _job_registry(std::make_shared<native::JobRegistry>()),
          _logger_mock(std::make_shared<LoggerMock>()),
          _service_bus{std::make_shared<fep3::native::ServiceBus>()},
          _configuration_service_mock(std::make_shared<ConfigurationServiceComponentMock>())
    {
    }

    void SetUp() override
    {
        ON_CALL(*_configuration_service_mock, registerNode(_))
            .WillByDefault(Return(fep3::Result()));

        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            _job_registry, _dummy_component_version_info));
        auto logging_service_mock = std::make_shared<LoggingServiceMock>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            logging_service_mock, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock, _dummy_component_version_info));

        EXPECT_CALL(*logging_service_mock, createLogger(_)).WillRepeatedly(Return(_logger_mock));

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    void TearDown() override
    {
        ASSERT_FEP3_NOERROR(_component_registry->destroy());
    }

    std::shared_ptr<ComponentRegistry> _component_registry{};
    std::shared_ptr<native::JobRegistry> _job_registry{};
    std::shared_ptr<LoggerMock> _logger_mock{};
    std::shared_ptr<native::ServiceBus> _service_bus{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};

private:
    const fep3::ComponentVersionInfo _dummy_component_version_info{
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};
};

TEST_F(NativeJobRegistryRPC, testGetJobNames)
{
    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(native::testing::participant_name_default));

    // actual test
    {
        EXPECT_EQ("", client.getJobNames());

        ASSERT_FEP3_NOERROR(
            _job_registry->addJob("test_job_1",
                                  std::make_shared<core::Job>("test_job_1", Duration{1}),
                                  fep3::arya::JobConfiguration{Duration{1}}));
        EXPECT_EQ("test_job_1", client.getJobNames());

        ASSERT_FEP3_NOERROR(
            _job_registry->addJob("test_job_2",
                                  std::make_shared<core::Job>("test_job_2", Duration{2}),
                                  fep3::arya::JobConfiguration{Duration{2}}));
        EXPECT_EQ("test_job_1,test_job_2", client.getJobNames());

        ASSERT_FEP3_NOERROR(_job_registry->removeJob("test_job_1"));
        EXPECT_EQ("test_job_2", client.getJobNames());

        ASSERT_FEP3_NOERROR(_job_registry->removeJob("test_job_2"));
        EXPECT_EQ("", client.getJobNames());
    }
}

TEST_F(NativeJobRegistryRPC, testGetJobInfoByJobName)
{
    _job_registry->addJob("test_job_1",
                          std::make_shared<core::Job>("test_job_1", Duration{1}),
                          fep3::arya::JobConfiguration{Duration{1}});
    _job_registry->addJob(
        "test_job_2",
        std::make_shared<core::Job>("test_job_2", Duration{2}),
        fep3::arya::JobConfiguration{
            Duration{2},
            Duration{3},
            Duration{4},
            fep3::arya::JobConfiguration::TimeViolationStrategy::skip_output_publish});

    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(native::testing::participant_name_default));

    // actual test
    {
        const auto job_info = client.getJobInfo("test_job_1");

        EXPECT_EQ("test_job_1", job_info["job_name"].asString());
        EXPECT_EQ("1", job_info["job_configuration"]["cycle_sim_time"].asString());
        EXPECT_EQ("0", job_info["job_configuration"]["delay_sim_time"].asString());
        EXPECT_EQ("", job_info["job_configuration"]["max_runtime_real_time"].asString());
        EXPECT_EQ("ignore_runtime_violation",
                  job_info["job_configuration"]["runtime_violation_strategy"].asString());

        const auto job_info_2 = client.getJobInfo("test_job_2");

        EXPECT_EQ("test_job_2", job_info_2["job_name"].asString());
        EXPECT_EQ("2", job_info_2["job_configuration"]["cycle_sim_time"].asString());
        EXPECT_EQ("3", job_info_2["job_configuration"]["delay_sim_time"].asString());
        EXPECT_EQ("4", job_info_2["job_configuration"]["max_runtime_real_time"].asString());
        EXPECT_EQ("skip_output_publish",
                  job_info_2["job_configuration"]["runtime_violation_strategy"].asString());
    }
}

TEST_F(NativeJobRegistryRPC, testGetNonExistentJobInfoByJobName)
{
    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(native::testing::participant_name_default));

    // actual test
    {
        const auto job_info = client.getJobInfo("");

        EXPECT_EQ("", job_info["job_name"].asString());
        EXPECT_EQ("", job_info["job_configuration"].asString());
    }
}

} // namespace env
} // namespace test
} // namespace fep3
