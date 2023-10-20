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

#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/core/element_base.h>
#include <fep3/native_components/clock/clock_main_event_sink.h>

#include <rpc/json_rpc.h>

#include <common/gtest_asserts.h>

using namespace ::testing;
using namespace fep3;
using namespace fep3::rpc;
using namespace fep3::arya;
using namespace fep3::core::arya;
using namespace std::chrono;

using EventID = IRPCClockSyncMasterDef::EventID;
using EventIDFlag = IRPCClockSyncMasterDef::EventIDFlag;

using Logger = NiceMock<fep3::mock::LoggerWithDefaultBehavior>;
using RPCRequester = NiceMock<fep3::mock::RPCRequester>;
using RPCResponse = NiceMock<fep3::mock::RPCRequester::RPCResponse>;

struct NativeClockSyncMasterTest : public Test {
    NativeClockSyncMasterTest()
        : _logger_mock(std::make_shared<Logger>()),
          _rpc_requester_mock(std::make_shared<RPCRequester>())
    {
        _get_rpc_requester_by_name = [this](const std::string& service_participant_name) {
            return _get_rpc_requester_by_name_mock.Call(service_participant_name);
        };
    }

    std::shared_ptr<Logger> _logger_mock;
    std::shared_ptr<RPCRequester> _rpc_requester_mock;
    std::function<const std::shared_ptr<IRPCRequester>(const std::string& service_participant_name)>
        _get_rpc_requester_by_name{};
    MockFunction<const std::shared_ptr<IRPCRequester>(const std::string& service_participant_name)>
        _get_rpc_requester_by_name_mock{};
    milliseconds _rpc_timeout{500};
};

class MyElement : public fep3::core::ElementBase {
public:
    MyElement() : fep3::core::ElementBase("test", "testversion")
    {
    }
};

std::string createRequestRegex(IRPCClockSyncMasterDef::EventID event_id)
{
    return a_util::strings::format(
        "event_id.*:%u",
        static_cast<std::underlying_type_t<IRPCClockSyncMasterDef::EventID>>(event_id));
}

struct slave {
    slave(std::string name, EventID event_id, EventIDFlag event_flag)
        : name(std::move(name)), event_id(event_id), event_flag(event_flag)
    {
    }
    std::string name;
    EventID event_id;
    EventIDFlag event_flag;
};

/**
 * @detail Test the clock sync master time synchronization.
 * Register a slave at the clock sync master and check whether the slave
 * receives the time update event.
 */
TEST_F(NativeClockSyncMasterTest, timeUpdating_successfulClientReceivesTimeUpdateEvent)
{
    const std::string slave_name{"slave_one"};
    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);

    const auto reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : "100"})";

    {
        EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_name))
            .WillOnce(Return(_rpc_requester_mock));
        EXPECT_CALL(*_rpc_requester_mock,
                    sendRequest(_,
                                ContainsRegex(createRequestRegex(
                                    IRPCClockSyncMasterDef::EventID::time_updating)),
                                _))
            .WillOnce(
                DoAll(WithArg<2>(testing::Invoke([reply](IRPCRequester::IRPCResponse& pResponse) {
                          pResponse.set(reply);
                      })),
                      Return(ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(clock_master.registerClient(
            slave_name,
            static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating)));
        clock_master.timeUpdating(Timestamp{1}, {});
    }
}

/**
 * @detail Test the clock sync master time update functionality.
 * Register a slave event for every time update event and check whether every slave
 * receives the corresponding time event.
 */
TEST_F(NativeClockSyncMasterTest, timeUpdating_successfulClientsReceiveOnlyRegisteredEvents)
{
    const Timestamp new_time{1}, old_time{0};
    const auto slaves = std::vector<slave>{
        {"slave_one_update_begin",
         EventID::time_update_before,
         EventIDFlag::register_for_time_update_before},
        {"slave_one_updating", EventID::time_updating, EventIDFlag::register_for_time_updating},
        {"slave_one_update_after",
         EventID::time_update_after,
         EventIDFlag::register_for_time_update_after},
        {"slave_one_reset_begin", EventID::time_reset, EventIDFlag::register_for_time_reset}};

    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);
    for (const auto& slave: slaves) {
        EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave.name))
            .WillOnce(Return(_rpc_requester_mock));
        ASSERT_FEP3_NOERROR(
            clock_master.registerClient(slave.name, static_cast<int>(slave.event_flag)));
    }

    const auto reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : "100"})";
    {
        for (const auto& slave: slaves) {
            EXPECT_CALL(*_rpc_requester_mock,
                        sendRequest(_, ContainsRegex(createRequestRegex(slave.event_id)), _))
                .WillOnce(DoAll(
                    WithArg<2>(testing::Invoke(
                        [reply](IRPCRequester::IRPCResponse& pResponse) { pResponse.set(reply); })),
                    Return(ERR_NOERROR)));
        }
    }

    clock_master.timeUpdateBegin(old_time, new_time);
    clock_master.timeUpdating(new_time, {});
    clock_master.timeUpdateEnd(new_time);
    clock_master.timeResetBegin(old_time, new_time);

    /// this one is not implemented yet
    clock_master.timeResetEnd(new_time);
}

/**
 * @detail Calling time updating with nullopt for next tick the request
 * should be sent with empty string in next tick
 */
TEST_F(NativeClockSyncMasterTest, timeUpdatingwithNullNextTime_successfulNextTickStringIsEmpty)
{
    const Timestamp new_time{1}, old_time{0};
    const auto slave_dummy = slave{
        "slave_one_updating", EventID::time_updating, EventIDFlag::register_for_time_updating};

    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);

    EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_dummy.name))
        .WillOnce(Return(_rpc_requester_mock));
    ASSERT_FEP3_NOERROR(
        clock_master.registerClient(slave_dummy.name, static_cast<int>(slave_dummy.event_flag)));

    const auto reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : "100"})";

    EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, HasSubstr(R"("next_tick":"")"), _))
        .WillOnce(DoAll(WithArg<2>(testing::Invoke([reply](IRPCRequester::IRPCResponse& pResponse) {
                            pResponse.set(reply);
                        })),
                        Return(ERR_NOERROR)));

    clock_master.timeUpdating(new_time, {});
}

/**
 * @detail Calling time updating with valid value for next_tick the request
 * should be sent with the correct value in next tick
 */
TEST_F(NativeClockSyncMasterTest, timeUpdatingwithValidNextTime_successfulNextTickStringIsNotEmpty)
{
    const Timestamp new_time{1}, old_time{0};
    const auto slave_dummy = slave{
        "slave_one_updating", EventID::time_updating, EventIDFlag::register_for_time_updating};

    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);

    EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_dummy.name))
        .WillOnce(Return(_rpc_requester_mock));
    ASSERT_FEP3_NOERROR(
        clock_master.registerClient(slave_dummy.name, static_cast<int>(slave_dummy.event_flag)));

    const auto reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : "100"})";

    EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, HasSubstr(R"("next_tick":"23")"), _))
        .WillOnce(DoAll(WithArg<2>(testing::Invoke([reply](IRPCRequester::IRPCResponse& pResponse) {
                            pResponse.set(reply);
                        })),
                        Return(ERR_NOERROR)));

    clock_master.timeUpdating(new_time, Timestamp{23});
}

/**
 * @detail Test the clock sync master synchronization with multiple slaves.
 * Register two slaves at the clock sync master and check whether both receive
 * a time update event
 */
TEST_F(NativeClockSyncMasterTest,
       timeUpdatingwithTwoClients_successfulBothClientsReceiveUdpateEvent)
{
    const std::string slave_one_name{"slave_one"}, slave_two_name{"slave_two"};
    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);

    const auto reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : "100"})";

    {
        EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_one_name))
            .WillOnce(Return(_rpc_requester_mock));
        EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_two_name))
            .WillOnce(Return(_rpc_requester_mock));
        EXPECT_CALL(*_rpc_requester_mock,
                    sendRequest(_,
                                ContainsRegex(createRequestRegex(
                                    IRPCClockSyncMasterDef::EventID::time_updating)),
                                _))
            .WillRepeatedly(
                DoAll(WithArg<2>(testing::Invoke([reply](IRPCRequester::IRPCResponse& pResponse) {
                          pResponse.set(reply);
                      })),
                      Return(ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(clock_master.registerClient(
            slave_one_name, static_cast<int>(EventIDFlag::register_for_time_updating)));
        ASSERT_FEP3_NOERROR(clock_master.registerClient(
            slave_two_name, static_cast<int>(EventIDFlag::register_for_time_updating)));

        clock_master.timeUpdating(Timestamp{1}, {});
    }
}

/**
 * @detail Test the clock sync master rpc timeout functionality.
 * Check whether an error is logged if the clock sync master does not
 * receive a rpc response before reaching the configured timeout due to
 * a jsonrpc exception being thrown.
 */
TEST_F(NativeClockSyncMasterTest, timeUpdatingwith_successfulErrorLoggedOnRpcResponseTimeout)
{
    const std::string slave_one_name{"slave_one"};
    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);

    {
        EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_one_name))
            .WillOnce(Return(_rpc_requester_mock));
        EXPECT_CALL(*_logger_mock, logError(HasSubstr("rpc timeout")))
            .WillOnce(Return(ERR_NOERROR));
        EXPECT_CALL(*_rpc_requester_mock,
                    sendRequest(_,
                                ContainsRegex(createRequestRegex(
                                    IRPCClockSyncMasterDef::EventID::time_updating)),
                                _))
            .WillOnce(
                DoAll(WithArg<2>(testing::Invoke([](IRPCRequester::IRPCResponse& /*pResponse*/) {
                          throw jsonrpc::JsonRpcException("rpc timeout");
                      })),
                      Return(ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(clock_master.registerClient(
            slave_one_name, static_cast<int>(EventIDFlag::register_for_time_updating)));
        clock_master.timeUpdating(Timestamp{1}, {});
    }
}

/**
 * @detail Test the clock sync master time update error.
 * Check whether an error is logged if the clock sync master does not
 * receive a rpc response before reaching the configured timeout due to
 * a runtime_error being thrown.
 */
TEST_F(NativeClockSyncMasterTest, timeUpdatingwith_successfulErrorLoggedOnClientRuntimeError)
{
    const std::string slave_one_name{"slave_one"};
    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);

    {
        EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_one_name))
            .WillOnce(Return(_rpc_requester_mock));
        EXPECT_CALL(*_logger_mock, logError(HasSubstr("some error"))).WillOnce(Return(ERR_NOERROR));
        EXPECT_CALL(*_rpc_requester_mock,
                    sendRequest(_,
                                ContainsRegex(createRequestRegex(
                                    IRPCClockSyncMasterDef::EventID::time_updating)),
                                _))
            .WillOnce(
                DoAll(WithArg<2>(testing::Invoke([](IRPCRequester::IRPCResponse& /*pResponse*/) {
                          throw std::runtime_error("some error");
                      })),
                      Return(ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(clock_master.registerClient(
            slave_one_name, static_cast<int>(EventIDFlag::register_for_time_updating)));
        clock_master.timeUpdating(Timestamp{1}, {});
    }
}

/**
 * @detail Test the clock sync master time register/unregister functionality.
 * Check whether a sync slave receives no more time events after being
 * unregistered from the clock sync master.
 */
TEST_F(NativeClockSyncMasterTest, unregisterClient_successfulClientDoesNotReceiveUpdateEvent)
{
    const std::string slave_one_name{"slave_one"};
    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);

    {
        EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_one_name))
            .WillOnce(Return(_rpc_requester_mock));

        ASSERT_FEP3_NOERROR(clock_master.registerClient(
            slave_one_name, static_cast<int>(EventIDFlag::register_for_time_updating)));
        ASSERT_FEP3_NOERROR(clock_master.unregisterClient(slave_one_name));
        /// no rpc calls are expected
        clock_master.timeUpdating(Timestamp{1}, {});
    }
}

/**
 * @detail Test the clock sync master slave registration.
 * Check whether a slave may be successfully registered if it has been registered already
 * which leads to activation of the already registered slave.
 */
TEST_F(NativeClockSyncMasterTest, registerClient_successfulOnRegisteringTwice)
{
    const std::string slave_one_name{"slave_one"};
    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);

    const auto reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : "100"})";

    {
        EXPECT_CALL(_get_rpc_requester_by_name_mock, Call(slave_one_name))
            .Times(2)
            .WillRepeatedly(Return(_rpc_requester_mock));
        EXPECT_CALL(*_rpc_requester_mock,
                    sendRequest(_,
                                ContainsRegex(createRequestRegex(
                                    IRPCClockSyncMasterDef::EventID::time_updating)),
                                _))
            .WillOnce(
                DoAll(WithArg<2>(testing::Invoke([reply](IRPCRequester::IRPCResponse& pResponse) {
                          pResponse.set(reply);
                      })),
                      Return(ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(clock_master.registerClient(
            slave_one_name, static_cast<int>(EventIDFlag::register_for_time_updating)));
        ASSERT_FEP3_NOERROR(clock_master.registerClient(
            slave_one_name, static_cast<int>(EventIDFlag::register_for_time_updating)));

        clock_master.timeUpdating(Timestamp{1}, {});
    }
}

/**
 * @detail Test the clock sync master time update timeout.
 * Check whether the rpc time update timeout may be reconfigured.
 */
TEST_F(NativeClockSyncMasterTest, updateTimeout_successfulWarningIfTimeoutIsLowerThanMinimumValue)
{
    ClockMainEventSink clock_master(_logger_mock, _rpc_timeout, _get_rpc_requester_by_name);
    /// we only test for no error
    ASSERT_FEP3_NOERROR(clock_master.updateTimeout(milliseconds(3000)));

    EXPECT_CALL(*_logger_mock, logWarning(HasSubstr("minimum value instead")))
        .WillOnce(Return(ERR_NOERROR));
    ASSERT_FEP3_NOERROR(clock_master.updateTimeout(milliseconds(1)));
}
