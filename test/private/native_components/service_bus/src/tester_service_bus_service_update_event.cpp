/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tester_service_bus_native_and_base_mocks.h"

#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/native_components/service_bus/rpc/http/include/http_systemaccess.h>

#include <boost/thread/barrier.hpp>
#include <boost/thread/latch.hpp>

#include <future>
#include <gtest_asserts.h>
#include <notification_waiting.h>
#include <thread>

using namespace testing;
struct ServiceUpdateTest : public ::testing::Test {
protected:
    ServiceUpdateTest()
        : system_default_urls_access(
              std::make_shared<testing::NiceMock<SystemAccessDefaultUrlsMock>>()),
          _system_discovery_factory(
              std::make_shared<testing::NiceMock<ServiceDiscoveryFactoryMock>>())
    {
    }

    void SetUp() override
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
    boost::latch _latch{1};

    EXPECT_CALL(
        _mock_update_sink,
        updateEvent(AllOf(
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::system_name, _system_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::service_name, _service_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::host_url, "url"),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::event_type,
                           fep3::IServiceBus::ServiceUpdateEventType::notify_byebye))))
        .WillOnce(InvokeWithoutArgs([&]() { _latch.count_down(); }));

    _system_access->registerUpdateEventSink(&_mock_update_sink);

    auto promise =
        setServiceUpdateEvents({{_service_name + "@" + _system_name,
                                 "url",
                                 fep3::IServiceBus::ServiceUpdateEventType::notify_byebye}});
    promise.get();

    _latch.wait();

    _system_access->deregisterUpdateEventSink(&_mock_update_sink);
    promise = setServiceUpdateEvents({{_service_name + "@" + _system_name,
                                       "url2",
                                       fep3::IServiceBus::ServiceUpdateEventType::notify_alive}});
    promise.get();

    EXPECT_CALL(*service_finder, disableDiscovery()).Times(1);
    _system_access.reset();
}

struct SystemAccessBlockingUpdateEvent : public ServiceUpdateTest,
                                         fep3::IServiceBus::IServiceUpdateEventSink {
    void SetUp() override
    {
        _update_events = {};

        auto service_finder_mock = std::make_unique<testing::StrictMock<ServiceFinderMock>>();
        service_finder = service_finder_mock.get();
        EXPECT_CALL(*service_finder, sendMSearch()).WillRepeatedly(Return(true));
        EXPECT_CALL(*service_finder, checkForServices(_, _)).WillRepeatedly(WithArg<0>([&](auto x) {
            return checkForServices(x);
        }));
        EXPECT_CALL(*_system_discovery_factory, getServiceFinder(_, _, _, _, _, _))
            .WillOnce(Return(ByMove(std::move(service_finder_mock))));
        EXPECT_CALL(*service_finder, disableDiscovery);
        _system_access = std::make_unique<fep3::native::HttpSystemAccess>(
            "system_name", "url", system_default_urls_access, nullptr, _system_discovery_factory);
    }

    bool checkForServices(
        const std::function<void(const fep3::native::ServiceUpdateEvent&)>& update_callback)
    {
        const fep3::native::ServiceUpdateEvent update_event = {
            "some_participant@system_name",
            "url",
            fep3::IServiceBus::ServiceUpdateEventType::notify_alive};
        update_callback(update_event);
        --_count_check_for_service_calls;
        if (_count_check_for_service_calls >= 0) {
            _latch.count_down();
        }
        return true;
    }

    void updateEvent(const fep3::IServiceBus::ServiceUpdateEvent&)
    {
        // tests recursive removal
        _system_access->deregisterUpdateEventSink(this);
        _latch.count_down_and_wait();
    }
    int _count_check_for_service_calls = 3;
    // 2 = two threads
    boost::latch _latch{static_cast<size_t>(2) +
                        static_cast<size_t>(_count_check_for_service_calls)};
};

TEST_F(SystemAccessBlockingUpdateEvent, updateEvent_doesNotBlockDiscoveryThread)
{
    _system_access->registerUpdateEventSink(this);
    _latch.count_down_and_wait();
    _system_access.reset();
}
