/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>
#include <testenvs/clock_sync_service_envs.h>

#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/native_components/clock/local_clock_service.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/base/properties/properties.h>
#include <fep3/base/properties/property_type.h>

using namespace ::testing;
using namespace fep3::native;
using namespace fep3::arya;
using namespace fep3::test::env;

/**
 * @detail Test whether the clock sync service default configuration is correct after creation.
 * This requires the following properties to be set:
 * * FEP3_TIMING_MASTER_PARTICIPANT = ""
 * * FEP3_CLOCK_SERVICE_SLAVE_SYNC_CYCLE_TIME = FEP3_CLOCK_SERVICE_SLAVE_SYNC_CYCLE_TIME_DEFAULT_VALUE
 * @req_id FEPSDK-2439
 */
TEST_F(NativeClockSyncService, testDefaultConfiguration)
{
    const std::string timing_master_name_expectation;
    const std::string slave_sync_cycle_time_default_value = std::to_string(FEP3_SLAVE_SYNC_CYCLE_TIME_DEFAULT_VALUE);

    // actual test case
    {
        ASSERT_EQ(fep3::base::getPropertyValue<std::string>(*_clock_sync_service_property_node->getChild(FEP3_TIMING_MASTER_PROPERTY)), timing_master_name_expectation);
        ASSERT_EQ(fep3::base::getPropertyValue<std::string>(*_clock_sync_service_property_node->getChild(FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY)), slave_sync_cycle_time_default_value);
    }
}

/**
 * @detail Test whether the clock sync service logs the configured timing master name on initialization success.
 */
TEST_F(NativeClockSyncService, testLogTimingMasterNameOnInit)
{
    EXPECT_CALL(*_logger, isDebugEnabled()).WillRepeatedly(Return(true));

    auto main_clock_property_node = _clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY);
    ASSERT_TRUE(main_clock_property_node);
    main_clock_property_node->setValue(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND);
    fep3::base::setPropertyValue(*_clock_sync_service_property_node->getChild(
        FEP3_TIMING_MASTER_PROPERTY),
        "timing_master");

    EXPECT_CALL(*_service_bus, getServer()).Times(1).WillOnce(Return(_rpc_server));
    EXPECT_CALL(*_rpc_server, registerService(fep3::rpc::IRPCClockSyncSlaveDef::getRPCDefaultName(),_))
        .Times(1).WillOnce(Return(fep3::Result()));
    EXPECT_CALL(*_service_bus, getRequester("timing_master")).Times(1).WillOnce(Return(_rpc_requester));

    // actual test case
    {
        EXPECT_CALL(*_logger, logDebug(_))
            .WillRepeatedly(Return(fep3::Result{}));
        EXPECT_CALL(*_logger, logDebug(fep3::mock::LogStringRegexMatcher
        ("Participant 'timing_master' is"))).WillOnce(Return(fep3::Result{}));

        const auto json_reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : 0})";

        EXPECT_CALL(*_rpc_requester, sendRequest(_, ContainsRegex("getMasterType"), _)).Times(1)
            .WillOnce(DoAll(
                WithArg<2>(Invoke([&json_reply](fep3::arya::IRPCRequester::IRPCResponse& response) {
            response.set(json_reply);
        })), Return(fep3::Result{})));

        EXPECT_CALL(*_rpc_requester, sendRequest(_, ContainsRegex("registerSyncSlave"), _)).Times(1)
            .WillOnce(DoAll(
                WithArg<2>(Invoke([&json_reply](fep3::arya::IRPCRequester::IRPCResponse& response) {
            response.set(json_reply);
        })), Return(fep3::Result{})));

        ASSERT_FEP3_NOERROR(_component_registry->initialize());
    }
}

/**
 * @detail Test whether the clock sync service returns an error on initialization if no timing master is configured.
 *
 */
TEST_F(NativeClockSyncService, testInitNoTimingMaster)
{
    // actual test case
    {
        EXPECT_CALL(*_logger, logError(ContainsRegex("timing master"))).Times(1).WillOnce(::testing::Return(::fep3::Result{}));


        // set properties to avoid getting an error which we do not test for in this test case
        fep3::base::setPropertyValue(*_clock_sync_service_property_node->getChild(
            FEP3_TIMING_MASTER_PROPERTY),
            "");

        auto main_clock_property_node = _clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY);
        ASSERT_TRUE(main_clock_property_node);
        main_clock_property_node->setValue(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND);

        ASSERT_FEP3_RESULT(_component_registry->initialize(), fep3::ERR_INVALID_ARG);
        ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    }

    // actual test case
    {
        EXPECT_CALL(*_logger, logError(ContainsRegex("timing master"))).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        // set properties to avoid getting an error which we do not test for in this test case
        fep3::base::setPropertyValue(*_clock_sync_service_property_node->getChild(
            FEP3_TIMING_MASTER_PROPERTY),
            "");

        auto main_clock_property_node = _clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY);
        ASSERT_TRUE(main_clock_property_node);
        main_clock_property_node->setValue(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE);

        ASSERT_FEP3_RESULT(_component_registry->initialize(), fep3::ERR_INVALID_ARG);
    }
}

/**
 * @detail Test whether the clock sync service successfully initializes if no main clock is configured.
 *
 */
TEST_F(NativeClockSyncService, testInitNoMainClock)
{
    // actual test case
    {
        // set properties to avoid getting an error which we do not test for in this test case
        fep3::base::setPropertyValue(*_clock_sync_service_property_node->getChild(
            FEP3_TIMING_MASTER_PROPERTY),
            "TimingMaster");

        auto main_clock_property_node = _clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY);
        ASSERT_TRUE(main_clock_property_node);
        main_clock_property_node->setValue("");

        ASSERT_FEP3_NOERROR(_component_registry->initialize());
    }
}

/**
 * @detail Test whether the clock sync service returns an error on initialization if an invalid sync cycle time is configured.
 *
 */
TEST_F(NativeClockSyncService, testInitInvalidSyncCycleTime)
{
    const int64_t invalid_sync_cycle_time = 0;

    // actual test case
    {
        EXPECT_CALL(*_logger, logError(ContainsRegex("sync cycle time"))).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        // set properties to avoid getting an error which we do not test for in this test case
        fep3::base::setPropertyValue(*_clock_sync_service_property_node->getChild(
            FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY),
            invalid_sync_cycle_time);
        fep3::base::setPropertyValue(*_clock_sync_service_property_node->getChild(
            FEP3_TIMING_MASTER_PROPERTY),
            "TimingMaster");

        auto main_clock_property_node = _clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY);
        ASSERT_TRUE(main_clock_property_node);
        main_clock_property_node->setValue(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND);

        ASSERT_FEP3_RESULT(_component_registry->initialize(), fep3::ERR_INVALID_ARG);
    }
}

/**
 * @detail Test whether the clock sync service successfully initializes if the clock service component does not provide a main clock property node.
 *
 */
TEST_F(NativeClockSyncService, testInitNoMainClockPropertyNode)
{
    // actual test case
    {
        auto clock_property_node = dynamic_cast<fep3::base::arya::IPropertyWithExtendedAccess*>(_clock_service_property_node.get());
        ASSERT_TRUE(clock_property_node);
        clock_property_node->removeChild(FEP3_MAIN_CLOCK_PROPERTY);

        ASSERT_FEP3_NOERROR(_component_registry->initialize());
    }
}