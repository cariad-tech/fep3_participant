/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/service_bus/mock_service_bus.h>

#include <boost/thread/barrier.hpp>
#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <service_update_event_sink_proxy.h>
#include <thread>

using namespace testing;

struct ServiceUpdateEventSinkProxyTest : public ::testing::Test {
    ServiceUpdateEventSinkProxyTest() : _event_sink_proxy(&_mock_update_sink)
    {
    }

    testing::NiceMock<fep3::mock::ServiceUpdateEventSink> _mock_update_sink;
    fep3::native::UpdateEventSinkProxy _event_sink_proxy;
    const std::string _system_name = "system_name";
    const std::string _service_name = "service_1";
    const std::string _host_url = "host_url";
    const fep3::IServiceBus::ServiceUpdateEvent _service_update_event{
        _service_name,
        _system_name,
        _host_url,
        fep3::IServiceBus::ServiceUpdateEventType::notify_byebye};
};

TEST_F(ServiceUpdateEventSinkProxyTest, run_callsUpdateEvent)
{
    EXPECT_CALL(
        _mock_update_sink,
        updateEvent(AllOf(
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::system_name, _system_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::service_name, _service_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::host_url, _host_url),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::event_type,
                           fep3::IServiceBus::ServiceUpdateEventType::notify_byebye))))
        .Times(1);

    _event_sink_proxy.run(_service_update_event);
}

TEST_F(ServiceUpdateEventSinkProxyTest, runAfterDectivate_doesNocallUpdateEvent)
{
    EXPECT_CALL(_mock_update_sink, updateEvent(_)).Times(0);

    _event_sink_proxy.deactivate();
    _event_sink_proxy.run(_service_update_event);
}

TEST_F(ServiceUpdateEventSinkProxyTest, deactivate_waitsRunningTask)
{
    boost::latch latch(1);
    boost::barrier all_threads_running(3);
    bool deactivate_returned = false;
    bool update_returned = false;

    auto update_call = [&]() {
        all_threads_running.wait();
        latch.wait();
        update_returned = true;
    };

    EXPECT_CALL(_mock_update_sink, updateEvent(_)).WillOnce(InvokeWithoutArgs(update_call));

    std::thread run([&]() { _event_sink_proxy.run(_service_update_event); });

    std::thread deactivate([&]() {
        all_threads_running.wait();
        _event_sink_proxy.deactivate();
        ASSERT_TRUE(update_returned);
        deactivate_returned = true;
    });

    all_threads_running.wait();
    ASSERT_FALSE(deactivate_returned);
    latch.count_down();
    deactivate.join();
    run.join();
}
