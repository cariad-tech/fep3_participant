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

#pragma once

#include "job_health_registry.h"

#include <fep3/components/base/component.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/rpc_services/health/health_service_rpc_intf_def.h>
#include <fep3/rpc_services/health/health_service_stub.h>

namespace fep3 {
namespace native {

class IJobHealthRegistry;

class RPCHealthService : public rpc::RPCService<fep3::rpc_stubs::RPCHealthServiceStub,
                                                fep3::rpc::catelyn::IRPCHealthServiceDef> {
public:
    explicit RPCHealthService(IJobHealthRegistry& job_health_registry)
        : _job_health_registry(job_health_registry)
    {
    }

    // Inherited via RPCService
    Json::Value resetHealth() override;
    Json::Value getHealth() override;

private:
    Json::Value toJsonArray(const std::vector<std::string>& signal_names);
    Json::Value to_json(const IJobHealthRegistry::JobHealthiness::ExecuteError& execute_error);
    IJobHealthRegistry& _job_health_registry;
};

class HealthService : public fep3::base::Component<fep3::catelyn::IHealthService>,
                      public base::EasyLogging {
public:
    HealthService(std::unique_ptr<IJobHealthRegistry> jobs_health_registry =
                      std::make_unique<JobHealthRegistry>());
    HealthService(const HealthService&) = delete;
    HealthService(HealthService&&) = delete;
    HealthService operator=(const HealthService&) = delete;
    HealthService operator=(HealthService&&) = delete;
    ~HealthService() = default;

    // Inherited via IComponent
    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result tense() override;
    fep3::Result relax() override;

    Result resetHealth() override;

    fep3::Result updateJobStatus(const std::string& job_name,
                                 const JobExecuteResult& result) override;

private:
    fep3::Result setupLogger(const IComponents& components);
    fep3::Result setupRPCHealthService(const IComponents& components);
    fep3::Result unregisterRPCHealthService(const IComponents& components) const;

private:
    std::shared_ptr<IRPCServer::IRPCService> _rpc_service{nullptr};
    std::unique_ptr<IJobHealthRegistry> _jobs_health_registry;
    // we assume parallel calls only on updateJobStatus, and getHealth
};

} // namespace native
} // namespace fep3
