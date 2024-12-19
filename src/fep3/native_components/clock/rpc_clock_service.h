/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/rpc_services/clock/clock_service_rpc_intf_def.h>
#include <fep3/rpc_services/clock/clock_service_stub.h>

namespace fep3 {
namespace arya {
class IClockService;
} // namespace arya
namespace rpc {

class RPCClockService
    : public rpc::RPCService<rpc_stubs::RPCClockServiceStub, rpc::IRPCClockServiceDef> {
public:
    explicit RPCClockService(fep3::experimental::IClockService& service);

    std::string getClockNames() override;
    std::string getMainClockName() override;
    std::string getTime(const std::string& clock_name) override;
    int getType(const std::string& clock_name) override;

private:
    fep3::arya::IClockService& _service;
};

} // namespace rpc
} // namespace fep3
