/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/rpc_services/clock_sync/clock_sync_master_service_stub.h>
#include <fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h>

#include <string>

namespace fep3::arya {
class IClockService;
} // namespace fep3::arya

namespace fep3 {
namespace rpc {

class IClockMainEventSink;

class RPCClockSyncService : public rpc::RPCService<rpc_stubs::RPCClockSyncMasterServiceStub,
                                                   rpc::arya::IRPCClockSyncMasterDef>,
                            public fep3::base::EasyLogging {
public:
    RPCClockSyncService(IClockMainEventSink& clock_main_event_sink,
                        fep3::arya::IClockService& service);

    int registerSyncSlave(int event_id_flag, const std::string& slave_name) override;
    int unregisterSyncSlave(const std::string& slave_name) override;
    int slaveSyncedEvent(const std::string& new_time, const std::string& slave_name) override;
    std::string getMasterTime() override;
    int getMasterType() override;

private:
    fep3::arya::IClockService& _service;
    IClockMainEventSink& _clock_main_event_sink;
};

} // namespace rpc
} // namespace fep3
