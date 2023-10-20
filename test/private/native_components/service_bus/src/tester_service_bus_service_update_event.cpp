/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2022 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "tester_service_bus_native_and_base_mocks.h"

#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/native_components/service_bus/rpc/http/include/http_systemaccess.h>
#include <fep3/native_components/service_bus/rpc/http/include/service_update_sink_registry.h>

#include <boost/asio.hpp>
#include <boost/thread/barrier.hpp>

#include <gtest_asserts.h>

using namespace testing;
struct ServiceUpdateTest : public ::testing::Test {
protected:
    ServiceUpdateTest()
        : system_default_urls_access(
              std::make_shared<testing::NiceMock<SystemAccessDefaultUrlsMock>>()),
          _system_discovery_factory(
              std::make_shared<testing::NiceMock<ServiceDiscoveryFactoryMock>>())
    {
        auto service_finder_mock = std::make_unique<testing::StrictMock<ServiceFinderMock>>();
        service_finder = service_finder_mock.get();
        EXPECT_CALL(*service_finder, sendMSearch()).WillRepeatedly(Return(true));
        EXPECT_CALL(*service_finder, checkForServices(_, _)).WillRepeatedly(WithArg<0>([&](auto x) {
            return checkForServices(x);
        }));
        EXPECT_CALL(*_system_discovery_factory, getServiceFinder(_, _, _, _, _, _))
            .WillOnce(Return(ByMove(std::move(service_finder_mock))));
        _system_access = std::make_unique<fep3::native::HttpSystemAccess>(
            "system_name", "url", system_default_urls_access, nullptr, _system_discovery_factory);
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
        EXPECT_CALL(*service_finder, disableDiscovery()).Times(1);
    }

    bool checkForServices(
        const std::function<void(const fep3::native::ServiceUpdateEvent&)>& update_callback)
    {
        const std::lock_guard<std::mutex> lock(_mtx);
        if (!_update_events.empty()) {
            for (const auto& update_event: _update_events) {
                update_callback(update_event);
            }
            _update_events.clear();
            _promise.set_value();
        }
        return true;
    }

    std::future<void> setServiceUpdateEvents(
        std::vector<fep3::native::ServiceUpdateEvent> update_events)
    {
        const std::lock_guard<std::mutex> lock(_mtx);
        _update_events = update_events;
        _promise = std::promise<void>();
        return _promise.get_future();
    }

    std::shared_ptr<SystemAccessDefaultUrlsMock> system_default_urls_access;
    std::shared_ptr<ServiceDiscoveryFactoryMock> _system_discovery_factory;
    std::unique_ptr<fep3::native::HttpSystemAccess> _system_access;
    ServiceFinderMock* service_finder;
    std::vector<fep3::native::ServiceUpdateEvent> _update_events;
    std::promise<void> _promise;
    std::mutex _mtx;
    testing::StrictMock<fep3::mock::ServiceUpdateEventSink> _mock_update_sink;
    const std::string _system_name = "system_name";
    const std::string _service_name = "service_1";
};

TEST_F(ServiceUpdateTest, testServiceUpdate)
{
    EXPECT_CALL(
        _mock_update_sink,
        updateEvent(AllOf(
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::system_name, _system_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::service_name, _service_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::host_url, "url"),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::event_type,
                           fep3::IServiceBus::ServiceUpdateEventType::notify_byebye))))
        .Times(1);

    _system_access->registerUpdateEventSink(&_mock_update_sink);

    auto promise =
        setServiceUpdateEvents({{_service_name + "@" + _system_name,
                                 "url",
                                 fep3::IServiceBus::ServiceUpdateEventType::notify_byebye}});
    promise.get();

    _system_access->deregisterUpdateEventSink(&_mock_update_sink);
    promise = setServiceUpdateEvents({{_service_name + "@" + _system_name,
                                       "url2",
                                       fep3::IServiceBus::ServiceUpdateEventType::notify_alive}});
    promise.get();
}

struct ServiceUpdateSinkRegistryTest : public ::testing::Test {
protected:
    testing::StrictMock<fep3::mock::ServiceUpdateEventSink> _mock_update_sink;
    fep3::native::ServiceUpdateSinkRegistry _service_update_sink_registry;
    const std::string _system_name = "system_name";
    const std::string _service_name = "service_1";
    const std::string _host_url = "host_url";
    const fep3::IServiceBus::ServiceUpdateEvent _service_update_event{
        _service_name,
        _system_name,
        _host_url,
        fep3::IServiceBus::ServiceUpdateEventType::notify_byebye};
};

TEST_F(ServiceUpdateSinkRegistryTest, testRegistration)
{
    ASSERT_FEP3_NOERROR(_service_update_sink_registry.registerUpdateEventSink(&_mock_update_sink));
    ASSERT_FEP3_RESULT(_service_update_sink_registry.registerUpdateEventSink(&_mock_update_sink),
                       fep3::ERR_FAILED);
}

TEST_F(ServiceUpdateSinkRegistryTest, testDeregistration)
{
    _service_update_sink_registry.registerUpdateEventSink(&_mock_update_sink);
    ASSERT_FEP3_NOERROR(
        _service_update_sink_registry.deregisterUpdateEventSink(&_mock_update_sink));
    ASSERT_FEP3_RESULT(_service_update_sink_registry.deregisterUpdateEventSink(&_mock_update_sink),
                       fep3::ERR_FAILED);
}

TEST_F(ServiceUpdateSinkRegistryTest, testUpdateEvent)
{
    _service_update_sink_registry.registerUpdateEventSink(&_mock_update_sink);

    EXPECT_CALL(
        _mock_update_sink,
        updateEvent(AllOf(
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::system_name, _system_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::service_name, _service_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::host_url, _host_url),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::event_type,
                           fep3::IServiceBus::ServiceUpdateEventType::notify_byebye))))
        .Times(1);

    _service_update_sink_registry.updateEvent(_service_update_event);

    _service_update_sink_registry.deregisterUpdateEventSink(&_mock_update_sink);

    _service_update_sink_registry.updateEvent(_service_update_event);
}

TEST_F(ServiceUpdateSinkRegistryTest, testMultiThread)
{
    const uint8_t updates_before_all_registered = 10;
    const uint8_t updates_after_all_registered = 10;
    const uint8_t thread_number = 10;
    boost::asio::thread_pool pool(10);
    boost::barrier barrier(10);

    std::vector<testing::NiceMock<fep3::mock::ServiceUpdateEventSink>> sink_vector(10);

    for (uint8_t i = 0; i < thread_number; ++i) {
        EXPECT_CALL(
            sink_vector[i],
            updateEvent(AllOf(
                testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::system_name, _system_name),
                testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::service_name, _service_name),
                testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::host_url, _host_url),
                testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::event_type,
                               fep3::IServiceBus::ServiceUpdateEventType::notify_byebye))))
            .WillRepeatedly(Return());
    }

    for (uint8_t i = 0; i < thread_number; ++i) {
        boost::asio::post(pool, [&, i]() {
            for (uint8_t j = 0; j < updates_before_all_registered; ++j) {
                _service_update_sink_registry.updateEvent(_service_update_event);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            _service_update_sink_registry.registerUpdateEventSink(&sink_vector[i]);
            // wait that at least all are registered
            barrier.wait();

            for (uint8_t j = 0; j < updates_after_all_registered; ++j) {
                _service_update_sink_registry.updateEvent(_service_update_event);
            }

            // make sure all have sent something
            barrier.wait();

            _service_update_sink_registry.deregisterUpdateEventSink(&sink_vector[i]);
        });
    }

    pool.join();
}
