/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/rpc_services/component_registry/component_registry_rpc_intf_def.h>
#include <fep3/rpc_services/component_registry/component_registry_service_stub.h>

#include <functional>

namespace fep3 {
class ComponentVersionInfo;

namespace native {

class ComponentRegistryRpcService
    : public fep3::rpc::RPCService<fep3::rpc_stubs::RPCParticipantComponentRegistryServiceStub,
                                   fep3::rpc::IRPCComponentRegistryDef> {
public:
    explicit ComponentRegistryRpcService(std::shared_ptr<ComponentRegistry> component_registry);
    Json::Value getPluginVersion(const std::string& service_iid) override;
    Json::Value getFilePath(const std::string& service_iid) override;
    Json::Value getParticipantLibraryVersion(const std::string& service_iid) override;
    Json::Value getComponentIIDs() override;

private:
    Json::Value invokeCompVersionFunction(
        const std::string& service_iid,
        const std::string& json_key,
        std::function<std::string(const ComponentVersionInfo&)> invoke_function) const;

    std::shared_ptr<ComponentRegistry> _component_registry;
};

} // namespace native
} // namespace fep3
