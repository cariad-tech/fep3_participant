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

#include "rpc_clock_sync_client.h"

namespace fep3 {
namespace rpc {

RPCClockSyncClient::RPCClockSyncClient(const std::string& name,
                                       const std::shared_ptr<IRPCRequester>& rpc_requester,
                                       const int event_id_flag)
    : RPCServiceClient<RPCClockSyncSlaveClientStub, IRPCClockSyncSlaveDef>(
          IRPCClockSyncSlaveDef::getRPCDefaultName(), rpc_requester),
      _active(false),
      _event_id_flag(event_id_flag),
      _name(name)
{
}

void RPCClockSyncClient::activate()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _active = true;
}

void RPCClockSyncClient::deactivate()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _active = false;
}

bool RPCClockSyncClient::isActive()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _active;
}

bool RPCClockSyncClient::isSet(IRPCClockSyncMasterDef::EventIDFlag flag)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return ((_event_id_flag & static_cast<int>(flag)) == static_cast<int>(flag));
}

void RPCClockSyncClient::setEventIDFlag(const int event_id_flag)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _event_id_flag = event_id_flag;
}

std::string RPCClockSyncClient::getName()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _name;
}

} // namespace rpc
} // namespace fep3
