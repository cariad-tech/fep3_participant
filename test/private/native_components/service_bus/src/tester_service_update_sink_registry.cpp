/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tester_service_bus_native_and_base_mocks.h"

#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/native_components/service_bus/rpc/http/include/service_update_sink_registry.h>

#include <boost/asio.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <gtest_asserts.h>
#include <notification_waiting.h>

using namespace testing;

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
    boost::latch _callback_latch{1};
    _service_update_sink_registry.registerUpdateEventSink(&_mock_update_sink);

    EXPECT_CALL(
        _mock_update_sink,
        updateEvent(AllOf(
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::system_name, _system_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::service_name, _service_name),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::host_url, _host_url),
            testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::event_type,
                           fep3::IServiceBus::ServiceUpdateEventType::notify_byebye))))
        .WillOnce(InvokeWithoutArgs([&]() { _callback_latch.count_down(); }));

    _service_update_sink_registry.updateEvent(_service_update_event);

    _callback_latch.wait();
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
    std::array<fep3::native::NotificationWaiting, 10> wait_for_update;

    std::vector<testing::NiceMock<fep3::mock::ServiceUpdateEventSink>> sink_vector(10);

    for (uint8_t i = 0; i < thread_number; ++i) {
        ON_CALL(
            sink_vector[i],
            updateEvent(AllOf(
                testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::system_name, _system_name),
                testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::service_name, _service_name),
                testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::host_url, _host_url),
                testing::Field(&fep3::IServiceBus::ServiceUpdateEvent::event_type,
                               fep3::IServiceBus::ServiceUpdateEventType::notify_byebye))))
            .WillByDefault(
                DoAll(InvokeWithoutArgs([&, i]() { wait_for_update[i].notify(); }), Return()));
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
            // wait until a notfication comes before deregestering
            wait_for_update[i].waitForNotification();
            _service_update_sink_registry.deregisterUpdateEventSink(&sink_vector[i]);
        });
    }

    pool.join();
}

TEST_F(ServiceUpdateSinkRegistryTest, updateEvent_doesNotBlock)
{
    boost::latch _callback_latch_in{2};
    _service_update_sink_registry.registerUpdateEventSink(&_mock_update_sink);
    std::atomic<bool> _callback_returned{false};

    // it could be theoretically that the update is called multiple times
    EXPECT_CALL(_mock_update_sink, updateEvent(_)).Times(AnyNumber());

    EXPECT_CALL(_mock_update_sink, updateEvent(_)).WillOnce(InvokeWithoutArgs([&]() {
        _callback_latch_in.count_down_and_wait();
        _callback_returned = true;
    }));

    _service_update_sink_registry.updateEvent(_service_update_event);
    /// in case we do not reach here the sink does not do an async dispathing an we have an endless
    /// wait
    ASSERT_FALSE(_callback_returned);
    _callback_latch_in.count_down_and_wait();

    _service_update_sink_registry.deregisterUpdateEventSink(&_mock_update_sink);
}

TEST_F(ServiceUpdateSinkRegistryTest, destructor_blocksUntilEventsComplete)
{
    boost::latch _callback_latch_in{2}, _callback_latch_out{2};
    boost::latch _reset_latch{2};

    std::unique_ptr<fep3::native::ServiceUpdateSinkRegistry> service_update_sink_registry =
        std::make_unique<fep3::native::ServiceUpdateSinkRegistry>();
    std::atomic<bool> _reset_returned{false};

    service_update_sink_registry->registerUpdateEventSink(&_mock_update_sink);

    // it could be theoretically that the update is called multiple times
    EXPECT_CALL(_mock_update_sink, updateEvent(_)).Times(AnyNumber());

    EXPECT_CALL(_mock_update_sink, updateEvent(_)).WillOnce(InvokeWithoutArgs([&]() {
        _callback_latch_in.count_down_and_wait();
        _callback_latch_out.count_down_and_wait();
    }));

    service_update_sink_registry->updateEvent(_service_update_event);

    _callback_latch_in.count_down_and_wait();
    auto reset_thread = std::thread([&]() { // reset is called while callback in progress
        _reset_latch.count_down_and_wait();
        service_update_sink_registry.reset();
        _reset_returned = true;
    });

    // make sure the thread runs
    _reset_latch.count_down_and_wait();
    // destructor does not return until call in progress is done
    ASSERT_FALSE(_reset_returned);
    _callback_latch_out.count_down();
    reset_thread.join();
}

TEST_F(ServiceUpdateSinkRegistryTest, desregister_blocksUntilEventComplete)
{
    boost::latch _callback_latch_in{2}, _callback_latch_out{2};
    boost::latch _deregister_latch_in{2}, _deregister_latch_out{2};

    std::atomic<bool> _deregister_returned{false};
    std::atomic<bool> _callback_returned{false};

    _service_update_sink_registry.registerUpdateEventSink(&_mock_update_sink);

    // it could be theoretically that the update is called multiple times
    EXPECT_CALL(_mock_update_sink, updateEvent(_)).Times(AnyNumber());

    EXPECT_CALL(_mock_update_sink, updateEvent(_)).WillOnce(InvokeWithoutArgs([&]() {
        _callback_latch_in.count_down_and_wait();
        _callback_latch_out.count_down_and_wait();
        _callback_returned = true;
    }));

    _service_update_sink_registry.updateEvent(_service_update_event);

    _callback_latch_in.count_down_and_wait();
    auto deregister_thread = std::thread([&]() { // deregister is called while callback in progress
        _deregister_latch_in.count_down();
        _service_update_sink_registry.deregisterUpdateEventSink(&_mock_update_sink);
        ASSERT_TRUE(_callback_returned);
        _deregister_returned = true;
        _deregister_latch_out.count_down_and_wait();
    });

    // make sure the thread runs, should block in deregisterUpdateEventSink
    _deregister_latch_in.count_down_and_wait();
    // deregister does not return until call in progress is done
    ASSERT_FALSE(_deregister_returned);
    _callback_latch_out.count_down_and_wait();
    _deregister_latch_out.count_down_and_wait();

    deregister_thread.join();
}
