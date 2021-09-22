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
#include "job_registry_with_rpc_c_access_wrapper.h"

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

fep3::Result JobRegistryWithRPC::create()
{
    const auto& components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get service bus interface");
    }

    const auto service_bus = components->getComponent<fep3::arya::IServiceBus>();
    if (!service_bus)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }
    const auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }
    FEP3_RETURN_IF_FAILED(rpc_server->registerService(rpc::arya::IRPCJobRegistryDef::getRPCDefaultName()
        , std::make_shared<RPCJobRegistry>(*this)));

    return JobRegistry::create();
}

std::string JobRegistryWithRPC::RPCJobRegistry::getJobNames()
{
    const auto job_infos = _job_registry.getJobInfos();
    std::string job_names;
    auto first = true;
    for (const auto& job_info : job_infos)
    {
        if (first)
        {
            job_names = job_info.getName();
            first = false;
        }
        else
        {
            job_names += "," + job_info.getName();
        }
    }
    return job_names;
}

Json::Value JobRegistryWithRPC::RPCJobRegistry::getJobInfo(const std::string& job_name)
{
    Json::Value json_value;

    try
    {
        const auto job_configuration = _job_registry.getJobs().at(job_name).job_info.getConfig();

        json_value["job_name"] = job_name;
        json_value["job_configuration"]["cycle_sim_time"] = job_configuration._cycle_sim_time.count();
        json_value["job_configuration"]["delay_sim_time"] = job_configuration._delay_sim_time.count();
        json_value["job_configuration"]["max_runtime_real_time"] = job_configuration._max_runtime_real_time.has_value() ?
            std::to_string(job_configuration._max_runtime_real_time.value().count()) : "";
        json_value["job_configuration"]["runtime_violation_strategy"] = job_configuration.toString();

        return json_value;
    }
    catch (const std::exception& /*exception*/)
    {
        json_value["job_name"] = "";
        json_value["job_configuration"] = "";
        return json_value;
    }
}

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
