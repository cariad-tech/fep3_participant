/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "health_service.h"

#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/rpc_services/base/fep_rpc_result_to_json.h>

namespace fep3 {
namespace native {

Json::Value RPCHealthService::resetHealth()
{
    Json::Value ret;
    ret["error"] = fep3::rpc::arya::resultToJson(_job_health_registry.resetHealth());
    return ret;
}

Json::Value RPCHealthService::getHealth()
{
    Json::Value ret = Json::objectValue;
    const auto& jobs_healthiness = _job_health_registry.getHealth();
    for (const auto& job_healthiness: jobs_healthiness) {
        Json::Value job_health_json;

        job_health_json["job_name"] = job_healthiness.job_name;

        std::visit(
            [&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<
                                  T,
                                  IJobHealthRegistry::JobHealthiness::ClockTriggeredJobInfo>)
                    job_health_json["cycle_time"] = arg.cycle_time.count();
                else if constexpr (std::is_same_v<
                                       T,
                                       IJobHealthRegistry::JobHealthiness::DataTriggeredJobInfo>)
                    job_health_json["trigger_signals"] = toJsonArray(arg.trigger_signals);
            },
            job_healthiness.job_info);

        job_health_json["simulation_timestamp"] = job_healthiness.simulation_time.count();
        job_health_json["last_execute_data_in_error"] =
            to_json(job_healthiness.execute_data_in_error);
        job_health_json["last_execute_error"] = to_json(job_healthiness.execute_error);
        job_health_json["last_execute_data_out_error"] =
            to_json(job_healthiness.execute_data_out_error);

        ret["jobs_healthiness"].append(job_health_json);
    }

    return ret;
}

Json::Value RPCHealthService::to_json(
    const IJobHealthRegistry::JobHealthiness::ExecuteError& execute_error)
{
    Json::Value execute_error_json;
    execute_error_json["error_count"] = execute_error.error_count;
    execute_error_json["simulation_timestamp"] = execute_error.simulation_time.count();
    execute_error_json["last_error"] = fep3::rpc::arya::resultToJson(execute_error.last_error);

    return execute_error_json;
}

Json::Value RPCHealthService::toJsonArray(const std::vector<std::string>& signal_names)
{
    Json::Value value = Json::arrayValue;
    for (const auto& signal_name: signal_names) {
        value.append(signal_name);
    }

    return value;
}

HealthService::HealthService(std::unique_ptr<IJobHealthRegistry> jobs_health_registry)
    : _jobs_health_registry(std::move(jobs_health_registry))
{
}

fep3::Result HealthService::create()
{
    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Component pointer is invalid");
    }

    FEP3_RETURN_IF_FAILED(setupRPCHealthService(*components));
    FEP3_RETURN_IF_FAILED(setupLogger(*components));

    return {};
}

fep3::Result HealthService::destroy()
{
    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Component pointer is invalid");
    }

    FEP3_RETURN_IF_FAILED(unregisterRPCHealthService(*components));

    return {};
}

fep3::Result HealthService::tense()
{
    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "access to components was not possible");
    }

    const auto job_registry = components->getComponent<fep3::IJobRegistry>();
    if (!job_registry) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "access to component IJobRegistry was not possible");
    }

    const auto jobs = job_registry->getJobsCatelyn();

    _jobs_health_registry->initialize(jobs, getLogger());
    return {};
}

fep3::Result HealthService::relax()
{
    _jobs_health_registry->deinitialize();
    return {};
}

Result HealthService::resetHealth()
{
    _jobs_health_registry->resetHealth();
    return {};
}

fep3::Result HealthService::setupLogger(const IComponents& components)
{
    FEP3_RETURN_IF_FAILED(initLogger(components, "health_service.component"));

    return {};
}

fep3::Result HealthService::setupRPCHealthService(const IComponents& components)
{
    const auto service_bus = components.getComponent<fep3::IServiceBus>();
    if (!service_bus) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }

    auto rpc_server = service_bus->getServer();
    if (!rpc_server) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }

    if (!_rpc_service) {
        _rpc_service = std::make_shared<RPCHealthService>(*_jobs_health_registry);
        FEP3_RETURN_IF_FAILED(rpc_server->registerService(
            rpc::catelyn::IRPCHealthServiceDef::getRPCDefaultName(), _rpc_service));
    }

    return {};
}

fep3::Result HealthService::unregisterRPCHealthService(const IComponents& components) const
{
    const auto* service_bus = components.getComponent<fep3::IServiceBus>();
    if (service_bus) {
        auto rpc_server = service_bus->getServer();
        if (rpc_server) {
            rpc_server->unregisterService(rpc::catelyn::IRPCHealthServiceDef::getRPCDefaultName());
        }
    }

    return {};
}

fep3::Result HealthService::updateJobStatus(const std::string& job_name,
                                            const JobExecuteResult& result)
{
    auto update_result = _jobs_health_registry->updateJobStatus(job_name, result);
    FEP3_LOG_RESULT(update_result);
    return update_result;
}

} // namespace native
} // namespace fep3
