/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/clock/mock_clock.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/native_components/clock/clock_main_event_sink_intf.h>
#include <fep3/native_components/clock/clock_service.h>
#include <fep3/native_components/clock/rpc_clock_service.h>
#include <fep3/native_components/clock/rpc_clock_sync_service.h>

#include <gtest/gtest.h>

using namespace ::testing;
using namespace fep3;

namespace fep3::mock {
struct ClockMainEventSink : public fep3::rpc::IClockMainEventSink {
    MOCK_METHOD(fep3::Result,
                registerClient,
                (const std::string& client_name, int event_id_flag),
                (override));
    MOCK_METHOD(fep3::Result, unregisterClient, (const std::string& client_name), (override));
    MOCK_METHOD(fep3::Result,
                receiveClientSyncedEvent,
                (const std::string& client_name, Timestamp time),
                (override));
    MOCK_METHOD(fep3::Result, updateTimeout, (std::chrono::nanoseconds rpc_timeout), (override));

    MOCK_METHOD(void, timeUpdateBegin, (fep3::arya::Timestamp, fep3::arya::Timestamp), (override));
    MOCK_METHOD(void,
                timeUpdating,
                (fep3::arya::Timestamp, std::optional<fep3::arya::Timestamp>),
                (override));
    MOCK_METHOD(void, timeUpdateEnd, (fep3::arya::Timestamp), (override));
    MOCK_METHOD(void, timeResetBegin, (fep3::arya::Timestamp, fep3::arya::Timestamp), (override));
    MOCK_METHOD(void, timeResetEnd, (fep3::arya::Timestamp), (override));
};
} // namespace fep3::mock

using ClockServiceMock = NiceMock<fep3::mock::ClockService>;
using ClockMainEventSinkMock = NiceMock<fep3::mock::ClockMainEventSink>;
using LoggerMock = NiceMock<fep3::mock::LoggerWithDefaultBehavior>;

using ClockType = fep3::arya::IClock::ClockType;

struct RPCClockSyncServiceTest : public ::testing::Test {
    RPCClockSyncServiceTest()
        : _clock_service_mock(std::make_shared<ClockServiceMock>()),
          _event_sink_mock(std::make_shared<ClockMainEventSinkMock>()),
          _rpc_clock_sync_service(std::make_shared<native::RPCClockSyncService>(
              *_event_sink_mock, *_clock_service_mock))
    {
    }

    std::shared_ptr<ClockServiceMock> _clock_service_mock;
    std::shared_ptr<ClockMainEventSinkMock> _event_sink_mock;
    std::shared_ptr<native::RPCClockSyncService> _rpc_clock_sync_service;
};

TEST_F(RPCClockSyncServiceTest, registerSyncSlave__successful)
{
    EXPECT_CALL(*_event_sink_mock, registerClient(_, _))
        .Times(1)
        .WillRepeatedly(Return(fep3::Result{ERR_NOERROR}));

    ASSERT_EQ(_rpc_clock_sync_service->registerSyncSlave(1, "client_1"), 0);
}

TEST_F(RPCClockSyncServiceTest, registerSyncSlave__fail)
{
    EXPECT_CALL(*_event_sink_mock, registerClient(_, _))
        .Times(1)
        .WillOnce(Return(fep3::Result{fep3::ERR_FAILED}));
    ASSERT_EQ(_rpc_clock_sync_service->registerSyncSlave(1, "client_1"), -1);
}

TEST_F(RPCClockSyncServiceTest, unregisterSyncSlave__successful)
{
    EXPECT_CALL(*_event_sink_mock, unregisterClient("client_1")).WillOnce(Return(fep3::Result{}));

    ASSERT_EQ(_rpc_clock_sync_service->unregisterSyncSlave("client_1"), 0);
}

TEST_F(RPCClockSyncServiceTest, unregisterSyncSlave__fail)
{
    EXPECT_CALL(*_event_sink_mock, unregisterClient(_))
        .WillOnce(Return(fep3::Result{fep3::ERR_FAILED}));

    ASSERT_EQ(_rpc_clock_sync_service->unregisterSyncSlave("client_1"), -1);
}

TEST_F(RPCClockSyncServiceTest, slaveSyncedEvent__successful)
{
    EXPECT_CALL(*_event_sink_mock, receiveClientSyncedEvent(_, _)).WillOnce(Return(fep3::Result{}));

    ASSERT_EQ(_rpc_clock_sync_service->slaveSyncedEvent("timestamp", "client_1"), 0);
}

TEST_F(RPCClockSyncServiceTest, slaveSyncedEvent__fail)
{
    EXPECT_CALL(*_event_sink_mock, receiveClientSyncedEvent(_, _))
        .WillOnce(Return(fep3::Result{fep3::ERR_FAILED}));

    ASSERT_EQ(_rpc_clock_sync_service->slaveSyncedEvent("timestamp", "client_1"), -1);
}

TEST_F(RPCClockSyncServiceTest, getMasterTime__successful)
{
    using namespace std::chrono_literals;

    auto expected_timestamp = Timestamp{100ms};

    EXPECT_CALL(*_clock_service_mock, getTime()).WillOnce(Return(expected_timestamp));

    ASSERT_EQ(_rpc_clock_sync_service->getMasterTime(), std::to_string(expected_timestamp.count()));
}

TEST_F(RPCClockSyncServiceTest, getMasterType__successful)
{
    EXPECT_CALL(*_clock_service_mock, getType())
        .WillOnce(Return(fep3::arya::IClock::ClockType::continuous));

    ASSERT_EQ(_rpc_clock_sync_service->getMasterType(), 0);

    EXPECT_CALL(*_clock_service_mock, getType())
        .WillOnce(Return(fep3::arya::IClock::ClockType::discrete));

    ASSERT_EQ(_rpc_clock_sync_service->getMasterType(), 1);
}
