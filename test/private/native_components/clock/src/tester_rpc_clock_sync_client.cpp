/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/native_components/clock/rpc_clock_sync_client.h>

#include <gtest/gtest.h>

using namespace ::testing;
using namespace fep3;

using RPCRequester = StrictMock<mock::RPCRequester>;
using RPCClockSyncClient = fep3::rpc::RPCClockSyncClient;
using IRPCClockSyncMasterDef = fep3::rpc::IRPCClockSyncMasterDef;

struct RPCClockSyncClientTest : public ::testing::Test {
    RPCClockSyncClientTest()
        : _name("client_1"),
          _event_id_flag(1),
          _requester(nullptr),
          _rpc_clock_sync_client(
              std::make_shared<RPCClockSyncClient>(_name, _requester, _event_id_flag))
    {
    }

    std::string _name;
    int _event_id_flag;
    std::shared_ptr<RPCRequester> _requester;
    std::shared_ptr<RPCClockSyncClient> _rpc_clock_sync_client;
};

TEST_F(RPCClockSyncClientTest, getName__successful)
{
    ASSERT_EQ(_rpc_clock_sync_client->getName(), _name);
}

TEST_F(RPCClockSyncClientTest, isActive__successful)
{
    ASSERT_EQ(_rpc_clock_sync_client->isActive(), false);
}

TEST_F(RPCClockSyncClientTest, activate__successful)
{
    _rpc_clock_sync_client->activate();
    ASSERT_TRUE(_rpc_clock_sync_client->isActive());
}

TEST_F(RPCClockSyncClientTest, isSet__successful)
{
    ASSERT_TRUE(_rpc_clock_sync_client->isSet(
        fep3::rpc::IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_before));
    ASSERT_FALSE(_rpc_clock_sync_client->isSet(
        IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating));
    ASSERT_FALSE(_rpc_clock_sync_client->isSet(
        IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_after));
    ASSERT_FALSE(_rpc_clock_sync_client->isSet(
        IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset));
}

TEST_F(RPCClockSyncClientTest, setEventIDFlag__successful)
{
    _rpc_clock_sync_client->setEventIDFlag(1);
    ASSERT_TRUE(_rpc_clock_sync_client->isSet(
        IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_before));
    _rpc_clock_sync_client->setEventIDFlag(2);
    ASSERT_TRUE(_rpc_clock_sync_client->isSet(
        IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating));
    _rpc_clock_sync_client->setEventIDFlag(4);
    ASSERT_TRUE(_rpc_clock_sync_client->isSet(
        IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_after));
    _rpc_clock_sync_client->setEventIDFlag(8);
    ASSERT_TRUE(_rpc_clock_sync_client->isSet(
        IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset));
}
