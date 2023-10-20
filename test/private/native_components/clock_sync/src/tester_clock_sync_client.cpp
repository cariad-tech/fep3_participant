/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include <fep3/components/base/mock_components.h>
#include <fep3/components/clock/mock_clock.h>
#include <fep3/components/clock_sync/mock/mock_clock_sync_service_addons.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/core/element_base.h>
#include <fep3/native_components/clock_sync/master_on_demand_clock_client.h>

#include <boost/thread/latch.hpp>

using namespace ::testing;
using namespace fep3;
using namespace fep3::arya;
using namespace fep3::rpc::arya;
using namespace fep3::core::arya;
using namespace fep3::native;
using namespace std::chrono_literals;

using Logger = NiceMock<mock::LoggerWithDefaultBehavior>;
using RPCRequester = StrictMock<mock::RPCRequester>;
using EventSink = NiceMock<mock::experimental::Clock::EventSink>;
using InterpolationTimeMock = NiceMock<mock::InterpolationTime>;
using LoggingService = fep3::mock::LoggingService;
using ComponentsMock = NiceMock<mock::Components>;
using Logger = NiceMock<mock::LoggerWithDefaultBehavior>;

struct NativeClockSyncClientTest : public Test {
    NativeClockSyncClientTest()
        : _event_sink_mock(std::make_shared<EventSink>()),
          _rpc_requester_mock(std::make_shared<RPCRequester>())
    {
        _master_time_event = [&](fep3::rpc::arya::IRPCClockSyncMasterDef::EventID id,
                                 Timestamp new_time,
                                 Timestamp old_time,
                                 std::optional<Timestamp> next_tick) {
            return masterTimeEvent(id, new_time, old_time, next_tick);
        };

        ON_CALL(*_components_mock, findComponent(_))
            .WillByDefault(Return(_logging_service_mock.get()));
        EXPECT_CALL(*_logging_service_mock, createLogger(_)).WillRepeatedly(Return(_logger_mock));
    }

    Timestamp masterTimeEvent(const fep3::rpc::IRPCClockSyncMasterDef::EventID,
                              const Timestamp,
                              const Timestamp,
                              std::optional<Timestamp>)
    {
        return {};
    }
    InterpolationTimeMock* _interpolation_time_mock;
    std::unique_ptr<InterpolationTimeMock> _interpolation_time_mock_unique_ptr;
    std::shared_ptr<EventSink> _event_sink_mock;
    std::shared_ptr<RPCRequester> _rpc_requester_mock;
    const std::string _participant_name_default = "test_participant_name";
    std::function<fep3::Timestamp(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID id,
                                  Timestamp new_time,
                                  Timestamp old_time,
                                  std::optional<Timestamp> next_tick)>
        _master_time_event;
    std::shared_ptr<LoggingService> _logging_service_mock = std::make_shared<LoggingService>();
    std::shared_ptr<ComponentsMock> _components_mock = std::make_shared<ComponentsMock>();
    std::shared_ptr<Logger> _logger_mock = std::make_shared<Logger>();
};

struct MasterOnDemandClockInterpolatingTest : NativeClockSyncClientTest {
    MasterOnDemandClockInterpolatingTest()
        : _interpolation_time_mock{new InterpolationTimeMock},
          _interpolation_time_mock_unique_ptr{_interpolation_time_mock}
    {
    }
    InterpolationTimeMock* _interpolation_time_mock;
    std::unique_ptr<InterpolationTimeMock> _interpolation_time_mock_unique_ptr;
};

/**
 * @detail Test the clock sync client interpolating clock getNewTime functionality.
 * A time retrieved from the clock sync client clock MasterOnDemandClockInterpolating
 * shall be interpolated and therefore be retrieved from IInterpolationClock, in this
 * test a mock.
 * @req_id FEPSDK-2442
 */
TEST_F(MasterOnDemandClockInterpolatingTest, start_setTimeCalledWithTimeFromTimeUpdateFunction)
{
    boost::latch latch{2};

    const Timestamp new_time{0};
    std::function<std::optional<fep3::Timestamp>()> time_update = [&]() {
        latch.count_down();
        return 101ns;
    };
    const Duration sync_cycle_time{std::chrono::milliseconds{100}};

    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        std::move(_interpolation_time_mock_unique_ptr), time_update, sync_cycle_time);

    {
        EXPECT_CALL(*_interpolation_time_mock, setTime(Duration{101}, _)).Times(AtLeast(1));
        master_on_demand_clock_interpolating->start(_event_sink_mock);
        latch.wait();

        master_on_demand_clock_interpolating->stop();
    }
}

/**
 * @detail Test the clock sync client interpolating clock resetTime functionality.
 * A reset shall call the corresponding IInterpolation time functionality, in this
 * test a mock.
 */
TEST_F(MasterOnDemandClockInterpolatingTest,
       startThenReset__resetCalledWithTimeFromTimeUpdateFunction)
{
    const Timestamp new_time{0};
    std::function<std::optional<fep3::Timestamp>()> time_update = []() { return 101ns; };

    const Duration sync_cycle_time{std::chrono::milliseconds{100}};

    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        std::move(_interpolation_time_mock_unique_ptr), time_update, sync_cycle_time);

    {
        EXPECT_CALL(*_interpolation_time_mock, resetTime(new_time)).Times(1);
        master_on_demand_clock_interpolating->start(_event_sink_mock);
        ASSERT_EQ(master_on_demand_clock_interpolating->masterTimeEvent(
                      fep3::rpc::IRPCClockSyncMasterDef::EventID::time_reset,
                      new_time,
                      new_time,
                      std::nullopt),
                  new_time);
        master_on_demand_clock_interpolating->stop();
    }
}

/**
 * @detail When the reset event is received before the clock is started then
 * the Event sink should be reset on clock start and a warning should be emitted.
 * @req_id FEPSDK-3655
 */
TEST_F(MasterOnDemandClockInterpolatingTest, resetThenStart__eventSinkResetCalled)
{
    auto event_sink_mock = std::make_shared<StrictMock<mock::experimental::Clock::EventSink>>();

    const Timestamp initial_time{0};
    const Duration sync_cycle_time{std::chrono::milliseconds{100}};
    const Timestamp reset_time{std::chrono::milliseconds{200}};
    std::function<std::optional<fep3::Timestamp>()> time_update = [&]() { return 150ms; };

    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        std::move(_interpolation_time_mock_unique_ptr), time_update, sync_cycle_time);
    master_on_demand_clock_interpolating->initLogger(*_components_mock, "mock_logger");

    master_on_demand_clock_interpolating->masterTimeEvent(
        IRPCClockSyncMasterDef::EventID::time_reset, reset_time, {}, {});

    EXPECT_CALL(*_logger_mock, logWarning(_)).Times(AtLeast(1));
    EXPECT_CALL(*event_sink_mock, timeResetBegin(initial_time, reset_time)).Times(1);
    EXPECT_CALL(*event_sink_mock, timeResetEnd(reset_time)).Times(1);

    master_on_demand_clock_interpolating->start(event_sink_mock);

    master_on_demand_clock_interpolating->stop();
}

/**
 * @detail When the reset event is received after clock is started then
 * the Event sink should be reset (happy path).
 * @req_id FEPSDK-3655
 */
TEST_F(MasterOnDemandClockInterpolatingTest, startThenReset_EventSinkResetCalled)
{
    auto event_sink_mock = std::make_shared<StrictMock<mock::experimental::Clock::EventSink>>();

    const Timestamp initial_time{0};
    const Duration sync_cycle_time{std::chrono::milliseconds{100}};
    const Timestamp reset_time{std::chrono::milliseconds{200}};
    std::function<std::optional<fep3::Timestamp>()> time_update = [&]() { return 150ms; };

    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        std::move(_interpolation_time_mock_unique_ptr), time_update, sync_cycle_time);

    EXPECT_CALL(*event_sink_mock, timeResetBegin(initial_time, reset_time)).Times(1);
    EXPECT_CALL(*event_sink_mock, timeResetEnd(reset_time)).Times(1);

    master_on_demand_clock_interpolating->start(event_sink_mock);
    master_on_demand_clock_interpolating->masterTimeEvent(
        IRPCClockSyncMasterDef::EventID::time_reset, reset_time, {}, {});

    master_on_demand_clock_interpolating->stop();
}

/**
 * @detail When the clock is stopped, no events should be forwarded to the sink.
 * @req_id FEPSDK-3655
 */
TEST_F(MasterOnDemandClockInterpolatingTest, startThenStopThenReset__eventSinkResetNotCalled)
{
    auto event_sink_mock = std::make_shared<StrictMock<mock::experimental::Clock::EventSink>>();

    const Duration sync_cycle_time{std::chrono::milliseconds{100}};
    const Timestamp reset_time{std::chrono::milliseconds{200}};
    std::function<std::optional<fep3::Timestamp>()> time_update = [&]() { return 150ms; };

    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        std::move(_interpolation_time_mock_unique_ptr), time_update, sync_cycle_time);

    EXPECT_CALL(*event_sink_mock, timeResetBegin(_, _)).Times(0);
    EXPECT_CALL(*event_sink_mock, timeResetEnd(_)).Times(0);

    master_on_demand_clock_interpolating->start(event_sink_mock);
    master_on_demand_clock_interpolating->stop();
    master_on_demand_clock_interpolating->masterTimeEvent(
        IRPCClockSyncMasterDef::EventID::time_reset, reset_time, {}, {});
}

/**
 * @detail When the clock is stopped and then started again,
 * the Event sink should be reset.
 * @req_id FEPSDK-3655
 */
TEST_F(MasterOnDemandClockInterpolatingTest, startThenStopThenStartThenReset_EventSinkCalled)
{
    auto event_sink_mock = std::make_shared<StrictMock<mock::experimental::Clock::EventSink>>();

    const Timestamp initial_time{0};
    const Duration sync_cycle_time{std::chrono::milliseconds{100}};
    const Timestamp reset_time{std::chrono::milliseconds{200}};
    std::function<std::optional<fep3::Timestamp>()> time_update = [&]() { return 150ms; };

    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        std::move(_interpolation_time_mock_unique_ptr), time_update, sync_cycle_time);

    EXPECT_CALL(*event_sink_mock, timeResetBegin(initial_time, reset_time)).Times(1);
    EXPECT_CALL(*event_sink_mock, timeResetEnd(reset_time)).Times(1);

    master_on_demand_clock_interpolating->start(event_sink_mock);
    master_on_demand_clock_interpolating->stop();
    master_on_demand_clock_interpolating->start(event_sink_mock);
    master_on_demand_clock_interpolating->masterTimeEvent(
        IRPCClockSyncMasterDef::EventID::time_reset, reset_time, {}, {});
}

struct FarClockUpdaterTest : NativeClockSyncClientTest {
};

/**
 * @detail Test the clock sync interpolating clock exception handling during registration/
 * deregistration to/from master.
 * Following request exceptions are handled:
 * * getMasterType
 * * registerSyncSlave
 * * unregisterSyncSlave
 */
TEST_F(FarClockUpdaterTest, registerRpcClient_warningLoggedOnExceptionDuringRpcCall)
{
    const Duration sync_cycle_time{std::chrono::milliseconds{100}};
    auto master_on_demand_clock_interpolating = std::make_shared<FarClockUpdater>(
        _rpc_requester_mock, _participant_name_default, _master_time_event);
    master_on_demand_clock_interpolating->initLogger(*_components_mock, "mock_logger");

    EXPECT_CALL(*_logger_mock, isDebugEnabled()).WillRepeatedly(Return(false));

    {
        EXPECT_CALL(*_logger_mock, logWarning(HasSubstr("get master type exception")))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(ERR_NOERROR));
        EXPECT_CALL(*_logger_mock, logWarning(HasSubstr("register sync slave exception")))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(ERR_NOERROR));
        EXPECT_CALL(*_logger_mock, logWarning(HasSubstr("unregister sync slave exception")))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(ERR_NOERROR));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("getMasterType"), _))
            .Times(AtLeast(1))
            .WillRepeatedly(DoAll(
                WithArg<2>(testing::Invoke([](fep3::IRPCRequester::IRPCResponse& /*pResponse*/) {
                    throw std::runtime_error("get master type exception");
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("registerSyncSlave"), _))
            .Times(AtLeast(1))
            .WillRepeatedly(DoAll(
                WithArg<2>(testing::Invoke([](fep3::IRPCRequester::IRPCResponse& /*pResponse*/) {
                    throw std::runtime_error("register sync slave exception");
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("unregisterSyncSlave"), _))
            .Times(AtLeast(1))
            .WillRepeatedly(DoAll(
                WithArg<2>(testing::Invoke([](fep3::IRPCRequester::IRPCResponse& /*pResponse*/) {
                    throw std::runtime_error("unregister sync slave exception");
                })),
                Return(ERR_NOERROR)));

        master_on_demand_clock_interpolating->startRPC();
        master_on_demand_clock_interpolating->stopRPC();
    }
}

struct MasterOnDemandClockDiscreteTest : NativeClockSyncClientTest {
    MasterOnDemandClockDiscreteTest()
        : _master_on_demand_clock_discrete(std::make_shared<MasterOnDemandClockDiscrete>())
    {
    }

    void SetUp() override
    {
        _master_on_demand_clock_discrete->initLogger(*_components_mock, "mock_logger");
        _master_on_demand_clock_discrete->start(_event_sink_mock);
    }

    void TearDown()
    {
        _master_on_demand_clock_discrete->stop();
    }
    std::shared_ptr<MasterOnDemandClockDiscrete> _master_on_demand_clock_discrete;
};
/**
 * @detail Test the clock sync client discrete clock.
 */
TEST_F(MasterOnDemandClockDiscreteTest, reset_eventSinkResetBeginEndCalled)
{
    const Timestamp reset_time{0}, default_clock_start_time{0}, not_used_time{-1};

    {
        EXPECT_CALL(*_event_sink_mock, timeResetBegin(default_clock_start_time, reset_time))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeResetEnd(reset_time)).Times(1);

        _master_on_demand_clock_discrete->masterTimeEvent(
            IRPCClockSyncMasterDef::EventID::time_reset, reset_time, not_used_time, std::nullopt);
    }
}

TEST_F(MasterOnDemandClockDiscreteTest, timeUpdating_eventSinkBeginTimeUpdatingEndCalled)
{
    const Timestamp new_time{0}, reset_time{0}, next_tick{100}, not_used_time{-1};

    _master_on_demand_clock_discrete->masterTimeEvent(
        IRPCClockSyncMasterDef::EventID::time_reset, reset_time, not_used_time, std::nullopt);
    {
        EXPECT_CALL(*_event_sink_mock, timeUpdateBegin(reset_time, new_time)).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(new_time, ::testing::Optional(next_tick)))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdateEnd(new_time)).Times(1);

        _master_on_demand_clock_discrete->masterTimeEvent(
            IRPCClockSyncMasterDef::EventID::time_updating, new_time, reset_time, next_tick);
    }
}

TEST_F(MasterOnDemandClockDiscreteTest, timeUpdatingNullNextTick_eventSinkTimeUpdatingNextTickNull)
{
    const Timestamp new_time{0}, reset_time{0}, old_time{0}, not_used_time{-1};

    _master_on_demand_clock_discrete->masterTimeEvent(
        IRPCClockSyncMasterDef::EventID::time_reset, reset_time, not_used_time, std::nullopt);
    {
        EXPECT_CALL(*_event_sink_mock, timeUpdateBegin(old_time, new_time)).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(new_time, ::testing::Eq(std::nullopt)))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdateEnd(new_time)).Times(1);

        _master_on_demand_clock_discrete->masterTimeEvent(
            IRPCClockSyncMasterDef::EventID::time_updating, new_time, old_time, std::nullopt);
    }
}

TEST_F(MasterOnDemandClockDiscreteTest, timeUpdateEvent_noRpcResetEventClockResetsWithWarning)
{
    const Timestamp new_time{0}, reset_time{0}, default_clock_start_time{0}, next_tick{100},
        not_used_time{-1};

    {
        EXPECT_CALL(*_logger_mock, logWarning(_)).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeResetBegin(default_clock_start_time, new_time)).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeResetEnd(new_time)).Times(1);

        EXPECT_CALL(*_event_sink_mock, timeUpdateBegin(default_clock_start_time, new_time))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(new_time, ::testing::Optional(next_tick)))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdateEnd(new_time)).Times(1);

        _master_on_demand_clock_discrete->masterTimeEvent(
            IRPCClockSyncMasterDef::EventID::time_updating, new_time, not_used_time, next_tick);
    }
}
