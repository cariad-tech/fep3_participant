/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/clock/mock_clock.h>
#include <fep3/components/configuration/mock_configuration_service.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/native_components/clock/clock_service.h>
#include <fep3/rpc_services/clock/clock_service_rpc_intf_def.h>

#include <common/gtest_asserts.h>

namespace fep3 {
namespace test {
namespace env {

using namespace ::testing;

using LoggingService = fep3::mock::LoggingService;
using WarningLogger = NiceMock<fep3::mock::WarningLogger>;

using ServiceBusComponent = NiceMock<fep3::mock::ServiceBus>;
using RPCServer = NiceMock<fep3::mock::RPCServer>;
using RPCRequester = NiceMock<fep3::mock::RPCRequester>;
using ConfigurationServiceComponentMock = StrictMock<fep3::mock::ConfigurationService>;
using ExperimentalClock = NiceMock<fep3::mock::experimental::Clock>;
using ExperimentalEventSinkMock = NiceMock<fep3::mock::experimental::Clock::EventSink>;

struct NativeClockService : public ::testing::Test {
    NativeClockService()
        : _component_registry(std::make_shared<fep3::ComponentRegistry>()),
          _service_bus(std::make_shared<ServiceBusComponent>()),
          _rpc_server(std::make_shared<RPCServer>()),
          _rpc_requester(std::make_shared<RPCRequester>()),
          _logger(std::make_shared<WarningLogger>()),
          _configuration_service_mock{std::make_shared<ConfigurationServiceComponentMock>()}
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_service_bus, getServer()).Times(1).WillOnce(::testing::Return(_rpc_server));
        EXPECT_CALL(*_rpc_server,
                    registerService(rpc::IRPCClockSyncMasterDef::getRPCDefaultName(), _))
            .WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*_rpc_server, registerService(rpc::IRPCClockServiceDef::getRPCDefaultName(), _))
            .WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*_configuration_service_mock, registerNode(_))
            .Times(1)
            .WillOnce(DoAll(WithArg<0>(Invoke([&](const std::shared_ptr<IPropertyNode>& node) {
                                _clock_service_property_node = node;
                            })),
                            ::testing::Return(Result())));

        EXPECT_CALL(*_configuration_service_mock, getNode(FEP3_CLOCK_SERVICE_MAIN_CLOCK))
            .WillRepeatedly(InvokeWithoutArgs([this]() {
                return _clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY);
            }));

        registerComponents();
        setComponents();

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    virtual void registerComponents()
    {
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus, _dummyComponentVersionInfo));
        auto logging_service = std::make_shared<LoggingService>();
        EXPECT_CALL(*logging_service, createLogger(_)).WillRepeatedly(Return(_logger));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            logging_service, _dummyComponentVersionInfo));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock, _dummyComponentVersionInfo));

        _clock_service_impl = std::make_shared<fep3::native::ClockService>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::arya::IClockService>(
            _clock_service_impl, _dummyComponentVersionInfo));
        ASSERT_FEP3_NOERROR(
            _component_registry->registerComponent<fep3::experimental::IClockService>(
                _clock_service_impl, _dummyComponentVersionInfo));
    }

    virtual void setComponents()
    {
        _clock_service_intf =
            _component_registry->getComponent<fep3::experimental::IClockService>();
        ASSERT_NE(_clock_service_intf, nullptr);
    }

    void TearDown() override
    {
        if (_service_bus) {
            testing::Mock::VerifyAndClearExpectations(_service_bus.get());
        }
        if (_rpc_server) {
            testing::Mock::VerifyAndClearExpectations(_rpc_server.get());
        }
    }

    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
    std::shared_ptr<ServiceBusComponent> _service_bus{nullptr};
    std::shared_ptr<RPCServer> _rpc_server{};
    std::shared_ptr<RPCRequester> _rpc_requester{};
    std::shared_ptr<WarningLogger> _logger{};
    fep3::experimental::IClockService* _clock_service_intf{nullptr};
    std::shared_ptr<fep3::native::ClockService> _clock_service_impl{nullptr};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
    std::shared_ptr<IPropertyNode> _clock_service_property_node;
    fep3::ComponentVersionInfo _dummyComponentVersionInfo{"3.0.1", "dummyPath", "3.1.0"};
};

struct NativeClockServiceWithClockMocks : NativeClockService {
    NativeClockServiceWithClockMocks()
        : _clock_mock(std::make_shared<ExperimentalClock>()),
          _clock_mock_2(std::make_shared<ExperimentalClock>())
    {
    }

    void SetUp() override
    {
        NativeClockService::SetUp();

        ON_CALL(*_clock_mock, getName()).WillByDefault(Return("my_clock"));
        ON_CALL(*_clock_mock_2, getName()).WillByDefault(Return("my_clock_2"));
    }

    std::shared_ptr<ExperimentalClock> _clock_mock{};
    std::shared_ptr<ExperimentalClock> _clock_mock_2{};
};

template <typename ClockMockType>
struct ClockServiceClockCompatibilityTests : public NativeClockService {
    void SetUp() override
    {
        NativeClockService::SetUp();

        _clock_service_catelyn_intf =
            _component_registry->getComponent<fep3::experimental::IClockService>();

        ASSERT_FEP3_NOERROR(_clock_service_catelyn_intf->registerClock(_clock_mock));

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
            *(_component_registry->getComponent<fep3::IConfigurationService>()),
            FEP3_CLOCK_SERVICE_MAIN_CLOCK,
            _clock_mock->getName()));
    }

    std::shared_ptr<ClockMockType> _clock_mock = std::make_shared<ClockMockType>();
    fep3::experimental::IClockService* _clock_service_catelyn_intf{nullptr};
};

template <typename T>
struct ClockServiceEventSinkCompatibilityTests
    : public ClockServiceClockCompatibilityTests<std::tuple_element_t<0, T>> {
    using ClockMockType = std::tuple_element_t<0, T>;
    using EventSinkMockType = std::tuple_element_t<1, T>;

    using EvenSinkType = std::conditional_t<std::is_same_v<ClockMockType, ExperimentalClock>,
                                            fep3::experimental::IClock::IEventSink,
                                            fep3::arya::IClock::IEventSink>;

    void SetUp() override
    {
        ClockServiceClockCompatibilityTests<ClockMockType>::SetUp();

        EXPECT_CALL(*(this->_clock_mock), start(_)).Times(1).WillOnce(Invoke([&](auto& event_sink) {
            _clock_event_sink = event_sink;
        }));

        ASSERT_FEP3_NOERROR(this->_clock_service_catelyn_intf->registerEventSink(_event_sink_mock));
        ASSERT_FEP3_NOERROR(this->_component_registry->initialize());
        ASSERT_FEP3_NOERROR(this->_component_registry->tense());
        ASSERT_FEP3_NOERROR(this->_component_registry->start());
    }

    void setUpdateExpectation()
    {
        if constexpr (std::is_same_v<EventSinkMockType, ExperimentalEventSinkMock>) {
            // A catelyn event sink with a catelyn clock will receive the next_tick
            if constexpr (std::is_same_v<EvenSinkType, fep3::experimental::IClock::IEventSink>)
                EXPECT_CALL(*_event_sink_mock,
                            timeUpdating(Timestamp{0}, ::testing::Optional(Timestamp{1})))
                    .Times(1);
            else
                // A catelyn event sink with an arya clock will not receive the next_tick
                EXPECT_CALL(*_event_sink_mock,
                            timeUpdating(Timestamp{0}, ::testing::Eq(std::nullopt)))
                    .Times(1);
        }
        else {
            EXPECT_CALL(*_event_sink_mock, timeUpdating(Timestamp{0})).Times(1);
        }
    }

    template <typename U>
    void doTimeUpdate(U ptr)
    {
        if constexpr (std::is_same_v<EvenSinkType, fep3::experimental::IClock::IEventSink>) {
            ptr->timeUpdating(Timestamp{0}, Timestamp{1});
        }
        else {
            ptr->timeUpdating(Timestamp{0});
        }
    }

    std::shared_ptr<EventSinkMockType> _event_sink_mock = std::make_shared<EventSinkMockType>();
    std::weak_ptr<EvenSinkType> _clock_event_sink;
};

} // namespace env
} // namespace test
} // namespace fep3
