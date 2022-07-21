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
#include "component_registry_rpc.h"
#include "fep3/components/base/component_version_info.h"

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
    if (isOk(return_result)) {
        ret[json_key] = invoke_function(component_version_info);
        return ret;
    }
    else {
        ret["error"] = resultToJson(return_result);
    }

    return ret;
}

Json::Value ComponentRegistryRpcService::resultToJson(a_util::result::Result nResult) const
{
    Json::Value oResult;
    oResult["error_code"] = nResult.getErrorCode();
    oResult["description"] = nResult.getDescription();
    oResult["line"] = nResult.getLine();
    oResult["file"] = nResult.getFile();
    oResult["function"] = nResult.getFunction();

    return oResult;
}

} // namespace native
} // namespace fep3
