/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../test/private/utils/common/gtest_asserts.h"

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/base/mock_components.h>
#include <fep3/components/configuration/mock_configuration_service.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/components/simulation_bus/mock_simulation_bus.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/native_components/data_registry/data_registry.h>
#include <fep3/native_components/logging/logging_service.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>

using namespace ::testing;

using ServiceBusComponentMock = NiceMock<fep3::mock::ServiceBus>;
using RPCServerMock = NiceMock<fep3::mock::RPCServer>;
using RPCRequesterMock = NiceMock<fep3::mock::RPCRequester>;
using ConfigurationServiceComponentMock = NiceMock<fep3::mock::ConfigurationService>;
using SimulationBusComponentMock = NiceMock<fep3::mock::SimulationBus>;
using LoggingServiceComponentMock = NiceMock<fep3::mock::LoggingService>;
using Logger = NiceMock<fep3::mock::LoggerWithDefaultBehavior>;
using ComponentsMock = NiceMock<fep3::mock::Components>;

struct NativeDataRegistryWithMocksBase : public ::testing::Test {
    NativeDataRegistryWithMocksBase()
        : _component_registry(std::make_shared<ComponentsMock>()),
          _data_registry(std::make_shared<fep3::native::DataRegistry>()),
          _service_bus(std::make_shared<ServiceBusComponentMock>()),
          _rpc_server(std::make_shared<RPCServerMock>()),
          _rpc_requester(std::make_shared<RPCRequesterMock>()),
          _configuration_service(std::make_shared<ConfigurationServiceComponentMock>()),
          _logger_mock(std::make_shared<Logger>()),
          _logging_service(std::make_shared<LoggingServiceComponentMock>())
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_component_registry, findComponent(_service_bus->getComponentIID()))
            .Times(1)
            .WillOnce(::testing::Return(_service_bus.get()));
        EXPECT_CALL(*_service_bus, getServer()).Times(1).WillOnce(::testing::Return(_rpc_server));
        EXPECT_CALL(*_rpc_server,
                    registerService(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(), _))
            .WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*_configuration_service, registerNode(_))
            .Times(1)
            .WillOnce(
                DoAll(WithArg<0>(Invoke([&](const std::shared_ptr<fep3::IPropertyNode>& node) {
                          _data_registry_property_node = node;
                      })),
                      ::testing::Return(fep3::Result())));
        EXPECT_CALL(*_component_registry, findComponent(_configuration_service->getComponentIID()))
            .Times(1)
            .WillOnce(::testing::Return(_configuration_service.get()));
        EXPECT_CALL(*_logging_service, createLogger(_)).WillRepeatedly(Return(_logger_mock));
        EXPECT_CALL(*_component_registry, findComponent(_logging_service->getComponentIID()))
            .Times(1)
            .WillOnce(::testing::Return(_logging_service.get()));

        ASSERT_FEP3_NOERROR(_data_registry->createComponent(_component_registry));
    }

    std::shared_ptr<fep3::native::DataRegistry> _data_registry{};
    std::shared_ptr<ServiceBusComponentMock> _service_bus{};
    std::shared_ptr<RPCServerMock> _rpc_server{};
    std::shared_ptr<RPCRequesterMock> _rpc_requester{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service{};
    std::shared_ptr<fep3::IPropertyNode> _data_registry_property_node;
    std::shared_ptr<Logger> _logger_mock{};
    std::shared_ptr<LoggingServiceComponentMock> _logging_service{};
    std::shared_ptr<ComponentsMock> _component_registry{};
};

struct NativeDataRegistryWithMocks : public NativeDataRegistryWithMocksBase {
    NativeDataRegistryWithMocks()
        : NativeDataRegistryWithMocksBase(),
          _simulation_bus(std::make_shared<SimulationBusComponentMock>())
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_simulation_bus, startBlockingReception(_))
            .Times(1)
            .WillOnce(WithArg<0>(
                Invoke([&](const std::function<void()>& reception_preparation_done_callback) {
                    reception_preparation_done_callback();
                })));
        NativeDataRegistryWithMocksBase::SetUp();
    }

    std::shared_ptr<SimulationBusComponentMock> _simulation_bus{};
};

template <typename sim_bus = fep3::mock::SimulationBus>
struct ParticipantUnderTest {
    ParticipantUnderTest(const std::string& test_participant_name_default =
                             fep3::native::testing::participant_name_default)
        : _registry{std::make_shared<fep3::native::DataRegistry>()},
          _service_bus{std::make_shared<fep3::native::ServiceBus>()},
          _configuration_service{std::make_shared<fep3::native::ConfigurationService>()},
          _simulation_bus{std::make_shared<sim_bus>()},
          _component_registry{std::make_shared<fep3::ComponentRegistry>()},
          _test_participant_name_default(test_participant_name_default),
          _logging_service(std::make_shared<fep3::native::LoggingService>())
    {
    }

    void SetUp()
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(
            *_service_bus, _test_participant_name_default));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ISimulationBus>(
            _simulation_bus, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IDataRegistry>(
            _registry, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            _logging_service, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    void TearDown()
    {
        ASSERT_FEP3_NOERROR(_component_registry->destroy());
    }

    std::shared_ptr<fep3::native::DataRegistry> _registry;
    std::shared_ptr<fep3::native::ServiceBus> _service_bus;
    std::shared_ptr<fep3::native::ConfigurationService> _configuration_service;
    std::shared_ptr<sim_bus> _simulation_bus;
    std::shared_ptr<fep3::native::LoggingService> _logging_service;
    std::shared_ptr<fep3::ComponentRegistry> _component_registry;

    std::string _test_participant_name_default;
    const fep3::ComponentVersionInfo _dummy_component_version_info{
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};
};

struct NativeDataRegistry : public ::testing::Test, public ParticipantUnderTest<> {
    NativeDataRegistry() : ParticipantUnderTest<>()
    {
    }

    void SetUp() override
    {
        ParticipantUnderTest<>::SetUp();
    }

    void TearDown() override
    {
        ParticipantUnderTest<>::TearDown();
    }

    const fep3::ComponentVersionInfo _dummy_component_version_info{
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};
};
