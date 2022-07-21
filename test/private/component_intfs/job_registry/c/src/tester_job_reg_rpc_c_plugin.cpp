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

#include <fep3/fep3_errors.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>
#include <fep3/components/job_registry/mock/mock_transferable_job_registry_with_access_to_jobs.h>
#include <fep3/components/job_registry/mock/mock_job.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/job_registry/c_access_wrapper/job_registry_with_rpc_c_access_wrapper.h>
#include <helper/component_c_plugin_helper.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>

#include "test_job_registry_client_stub.h"
#include <fep3/rpc_services/job_registry/job_registry_rpc_intf_def.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>

#include <common/gtest_asserts.h>

const std::string test_plugin_1_path = PLUGIN;

using namespace fep3::plugin::c::arya;
using namespace fep3::plugin::c::access::arya;

namespace fep3
{
namespace test
{

struct Plugin1PathGetter
{
    std::string operator()() const
    {
        return test_plugin_1_path;
    }
};
struct SetMockComponentFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockJobRegistry";
    }
};

/**
 * Test fixture loading a mocked job registry from within a C plugin
 */
using JobRegistryLoader = MockedComponentCPluginLoader
    <IJobRegistry
    , mock::TransferableJobRegistryWithAccessToJobs
    , plugin::c::access::arya::JobRegistryWithRPC
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
using JobRegistryLoaderFixture = MockedComponentCPluginLoaderFixture<JobRegistryLoader>;

namespace env
{

using namespace ::testing;

using LoggingServiceMock = mock::LoggingService;
using LoggerMock = StrictMock<mock::Logger>;
using ConfigurationServiceComponentMock = StrictMock<mock::TransferableConfigurationService>;

class TestClient : public rpc::RPCServiceClient<::test::rpc_stubs::TestJobRegistryClientStub, rpc::IRPCJobRegistryDef>
{
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

struct JobRegistryRPCCPlugin : public JobRegistryLoaderFixture
{
    JobRegistryRPCCPlugin()
        : _component_registry(std::make_shared<ComponentRegistry>())
        , _logger_mock(std::make_shared<LoggerMock>())
        , _service_bus(std::make_shared<native::ServiceBus>())
        , _configuration_service_mock(std::make_shared<ConfigurationServiceComponentMock>())
    {}

    void SetUp() override
    {
        ASSERT_TRUE(native::testing::prepareServiceBusForTestingDefault(*_service_bus));

        JobRegistryLoaderFixture::SetUp();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            JobRegistryLoaderFixture::extractComponent(), _dummy_component_version_info))
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            std::make_shared<LoggingServiceMock>(_logger_mock), _dummy_component_version_info))
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus, _dummy_component_version_info))
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock, _dummy_component_version_info))

        ASSERT_FEP3_NOERROR(_component_registry->create())
    }

    std::shared_ptr<ComponentRegistry> _component_registry{};
    std::shared_ptr<LoggerMock> _logger_mock{};
    std::shared_ptr<native::ServiceBus> _service_bus{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
private:
    const fep3::ComponentVersionInfo _dummy_component_version_info{ FEP3_PARTICIPANT_LIBRARY_VERSION_STR,"dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR };
};

/**
 * Test getting job names via RPC
 * @req_id TODO
 */
TEST_F(JobRegistryRPCCPlugin, testGetJobNames)
{
    const auto& test_job_1_name = std::string("test_job_1");
    const auto& test_job_2_name = std::string("test_job_2");

    auto& mock_job_registry = getMockComponent();
    // setting of expectations
    {
        ::testing::InSequence call_sequence;

        EXPECT_CALL(mock_job_registry, getJobInfos())
            .WillOnce(Return(std::list<arya::JobInfo>{}))
            .WillOnce(Return(std::list<arya::JobInfo>{{test_job_1_name, JobConfiguration{Duration{1}}}}))
            .WillOnce(Return(std::list<arya::JobInfo>
                {{test_job_1_name, JobConfiguration{Duration{1}}}
                , {test_job_2_name, JobConfiguration{Duration{2}}}
                }))
            ;
    }

    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(native::testing::participant_name_default));

    // actual test
    {
        EXPECT_EQ("", client.getJobNames());
        EXPECT_EQ(test_job_1_name, client.getJobNames());
        EXPECT_EQ(test_job_1_name + "," + test_job_2_name, client.getJobNames());
    }
}

TEST_F(JobRegistryRPCCPlugin, testGetJobInfoByJobName)
{
    const auto& test_job_1_name = std::string("test_job_1");
    const auto& test_job_1_configuration = JobConfiguration{Duration{1}};
    const auto& test_job_2_name = std::string("test_job_2");
    const auto& test_job_2_configuration = JobConfiguration
        {Duration{2}
        , Duration{3}
        , Duration{4}
        , JobConfiguration::TimeViolationStrategy::skip_output_publish
        };

    auto& mock_job_registry = getMockComponent();
    // setting of expectations
    {
        ::testing::InSequence call_sequence;

        EXPECT_CALL(mock_job_registry, getJobs()).WillRepeatedly(Return(Jobs
            {{test_job_1_name, {{}, {test_job_1_name, test_job_1_configuration}}}
            , {test_job_2_name, {{}, {test_job_2_name, test_job_2_configuration}}}
            }));
    }

    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(native::testing::participant_name_default));

    // actual test
    {
        const auto job_1_info = client.getJobInfo(test_job_1_name);
        EXPECT_EQ(test_job_1_name, job_1_info["job_name"].asString());
        EXPECT_EQ
            (test_job_1_configuration._cycle_sim_time.count()
            , std::stoi(job_1_info["job_configuration"]["cycle_sim_time"].asString())
            );
        EXPECT_EQ
            (test_job_1_configuration._delay_sim_time.count()
            , std::stoi(job_1_info["job_configuration"]["delay_sim_time"].asString())
            );
        EXPECT_FALSE(test_job_1_configuration._max_runtime_real_time.has_value());
        EXPECT_EQ
            (test_job_1_configuration.toString()
            , job_1_info["job_configuration"]["runtime_violation_strategy"].asString()
            );

        const auto job_2_info = client.getJobInfo(test_job_2_name);
        EXPECT_EQ(test_job_2_name, job_2_info["job_name"].asString());
        EXPECT_EQ
            (test_job_2_configuration._cycle_sim_time.count()
            , std::stoi(job_2_info["job_configuration"]["cycle_sim_time"].asString())
            );
        EXPECT_EQ
            (test_job_2_configuration._delay_sim_time.count()
            , std::stoi(job_2_info["job_configuration"]["delay_sim_time"].asString())
            );
        EXPECT_TRUE(test_job_2_configuration._max_runtime_real_time.has_value());
        EXPECT_EQ
            (test_job_2_configuration._max_runtime_real_time.value_or(0).count()
            , std::stoi(job_2_info["job_configuration"]["max_runtime_real_time"].asString())
            );
        EXPECT_EQ
            (test_job_2_configuration.toString()
            , job_2_info["job_configuration"]["runtime_violation_strategy"].asString()
            );
    }
}

TEST_F(JobRegistryRPCCPlugin, testGetNonExistentJobInfoByJobName)
{
    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(native::testing::participant_name_default));

    auto& mock_job_registry = getMockComponent();
    // setting of expectations
    {
        ::testing::InSequence call_sequence;

        EXPECT_CALL(mock_job_registry, getJobs()).WillOnce(Return(Jobs{}));
    }

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
