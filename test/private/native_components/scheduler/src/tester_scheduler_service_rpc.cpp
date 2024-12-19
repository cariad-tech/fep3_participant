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
#include <fep3/components/scheduler/mock_scheduler.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/native_components/scheduler/local_scheduler_service.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>

#include <test_scheduler_service_client_stub.h>

namespace fep3 {
namespace test {
namespace env {

using namespace ::testing;

using LoggingServiceMock = NiceMock<mock::LoggingService>;
using LoggerMock = NiceMock<mock::LoggerWithDefaultBehavior>;
using ConfigurationServiceComponentMock = NiceMock<mock::ConfigurationService>;
using SchedulerMock = NiceMock<mock::Scheduler>;

class TestClient : public rpc::RPCServiceClient<::test::rpc_stubs::TestSchedulerServiceClientStub,
                                                rpc::IRPCSchedulerServiceDef> {
private:
    typedef RPCServiceClient<TestSchedulerServiceClientStub, rpc::IRPCSchedulerServiceDef>
        base_type;

public:
    using base_type::GetStub;

    TestClient(const std::string& server_object_name,
               const std::shared_ptr<IRPCRequester>& rpc_requester)
        : base_type(server_object_name, rpc_requester)
    {
    }
};

struct NativeSchedulerServiceRPC : public Test {
    NativeSchedulerServiceRPC()
        : _component_registry(std::make_shared<ComponentRegistry>()),
          _scheduler_service(std::make_shared<native::LocalSchedulerService>()),
          _logging_service_mock{std::make_shared<LoggingServiceMock>()},
          _configuration_service_mock{std::make_shared<ConfigurationServiceComponentMock>()},
          _service_bus{std::make_shared<fep3::native::ServiceBus>()}
    {
    }

    void SetUp() override
    {
        ON_CALL(*_configuration_service_mock, registerNode(_)).WillByDefault(Return(Result()));

        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ISchedulerService>(
            _scheduler_service, _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            _logging_service_mock, _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock, _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus, _dummy_component_version_info));

        _logger_mock = std::make_shared<LoggerMock>();
        EXPECT_CALL(*_logging_service_mock, createLogger(_)).WillRepeatedly(Return(_logger_mock));

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    std::shared_ptr<ComponentRegistry> _component_registry{};
    std::shared_ptr<native::LocalSchedulerService> _scheduler_service{};
    std::shared_ptr<LoggingServiceMock> _logging_service_mock{};
    std::shared_ptr<LoggerMock> _logger_mock{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
    std::shared_ptr<native::ServiceBus> _service_bus{};

private:
    const fep3::ComponentVersionInfo _dummy_component_version_info{
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};
};

TEST_F(NativeSchedulerServiceRPC, testGetSchedulerNames)
{
    TestClient client(rpc::IRPCSchedulerServiceDef::getRPCDefaultName(),
                      _service_bus->getRequester(native::testing::participant_name_default));

    // actual test
    {
        ASSERT_EQ(FEP3_SCHEDULER_CLOCK_BASED, client.getSchedulerNames());

        std::unique_ptr<SchedulerMock> scheduler_mock{std::make_unique<SchedulerMock>()};

        ON_CALL(*scheduler_mock, getName()).WillByDefault(Return("my_custom_scheduler"));
        _scheduler_service->registerScheduler(
            std::unique_ptr<fep3::catelyn::IScheduler>(scheduler_mock.release()));

        ASSERT_EQ(FEP3_SCHEDULER_CLOCK_BASED ",my_custom_scheduler", client.getSchedulerNames());

        _scheduler_service->unregisterScheduler("my_custom_scheduler");

        ASSERT_EQ(FEP3_SCHEDULER_CLOCK_BASED, client.getSchedulerNames());
    }
}

TEST_F(NativeSchedulerServiceRPC, testGetActiveSchedulerName)
{
    TestClient client(rpc::IRPCSchedulerServiceDef::getRPCDefaultName(),
                      _service_bus->getRequester(native::testing::participant_name_default));

    // actual test
    {
        ASSERT_EQ(FEP3_SCHEDULER_CLOCK_BASED, client.getActiveSchedulerName());
    }
}

} // namespace env
} // namespace test
} // namespace fep3
