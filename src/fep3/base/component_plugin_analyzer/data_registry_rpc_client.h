/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fep3/base/component_plugin_analyzer/component_description_base.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/rpc_services/data_registry/data_registry_rpc_intf_def.h>

#include <a_util/strings.h>

#include <data_registry_client_stub.h>

namespace fep3::base {

class DataRegistryRpcClient
    : public fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCDataRegistryClientStub,
                                         fep3::rpc::IRPCDataRegistryDef> {
private:
    typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCDataRegistryClientStub,
                                        fep3::rpc::IRPCDataRegistryDef>
        base_type;

public:
    using base_type::GetStub;

    DataRegistryRpcClient(const char* server_object_name, std::shared_ptr<fep3::IRPCRequester> rpc);

    std::vector<SignalInfo> getRegisteredSignals();

private:
    void fillSignalInfo(const std::vector<std::string>& signal_names,
                        std::vector<SignalInfo>& info,
                        SignalFlow direction);

    std::vector<std::string> getSignalInNames() const;
    std::vector<std::string> getSignalOutNames() const;

    SignalInfo getSignalInfo(const std::string& signal_name, SignalFlow direction) const;
};
} // namespace fep3::base
