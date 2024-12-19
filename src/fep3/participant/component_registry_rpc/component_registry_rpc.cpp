/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "component_registry_rpc.h"

#include <fep3/rpc_services/base/fep_rpc_result_to_json.h>

#include <a_util/strings/strings_functions.h>

namespace fep3 {
namespace native {

ComponentRegistryRpcService::ComponentRegistryRpcService(
    std::shared_ptr<ComponentRegistry> component_registry)
    : _component_registry(std::move(component_registry))
{
}

Json::Value ComponentRegistryRpcService::getPluginVersion(const std::string& service_iid)
{
    return invokeCompVersionFunction(
        service_iid, "version", [](const ComponentVersionInfo& c) { return c.getVersion(); });
}

Json::Value ComponentRegistryRpcService::getFilePath(const std::string& service_iid)
{
    return invokeCompVersionFunction(
        service_iid, "file_path", [](const ComponentVersionInfo& c) { return c.getFilePath(); });
}

Json::Value ComponentRegistryRpcService::getParticipantLibraryVersion(
    const std::string& service_iid)
{
    return invokeCompVersionFunction(
        service_iid, "participant_version", [](const ComponentVersionInfo& c) {
            return c.getParticipantLibraryVersion();
        });
}

Json::Value ComponentRegistryRpcService::getComponentIIDs()
{
    const std::vector<std::string> component_iids = _component_registry->getComponentIIDs();
    Json::Value ret;
    ret["component_iids"] = a_util::strings::join(component_iids, ",");

    return ret;
}

Json::Value ComponentRegistryRpcService::invokeCompVersionFunction(
    const std::string& service_iid,
    const std::string& json_key,
    std::function<std::string(const ComponentVersionInfo&)> invoke_function) const
{
    fep3::Result return_result;
    ComponentVersionInfo component_version_info;
    // with c++17 we could use structured bindings, make members of ComponentVersionInfo const and
    // delete the default constructor
    std::tie(return_result, component_version_info) =
        _component_registry->getComponentVersion(service_iid);

    Json::Value ret;
    if (return_result) {
        ret[json_key] = invoke_function(component_version_info);
        return ret;
    }
    else {
        ret["error"] = fep3::rpc::arya::resultToJson(return_result);
    }

    return ret;
}

} // namespace native
} // namespace fep3
