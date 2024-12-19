/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h>
#include <fep3/rpc_services/clock_sync/clock_sync_slave_client_stub.h>

#include <mutex>
#include <string>

namespace fep3 {
namespace rpc {
class RPCClockSyncClient
    : public RPCServiceClient<rpc_stubs::RPCClockSyncSlaveClientStub, IRPCClockSyncSlaveDef> {
public:
    RPCClockSyncClient(const std::string& name,
                       const std::shared_ptr<IRPCRequester>& rpc_requester,
                       int event_id_flag);

    void activate();
    void deactivate();
    bool isActive();

    bool isSet(IRPCClockSyncMasterDef::EventIDFlag flag);
    void setEventIDFlag(int event_id_flag);
    std::string getName();

private:
    bool _active;
    int _event_id_flag;
    std::string _name;
    std::mutex _mutex;
};

} // namespace rpc
} // namespace fep3
