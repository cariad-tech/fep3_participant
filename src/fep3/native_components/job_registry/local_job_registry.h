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

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/rpc_services/job_registry/job_registry_rpc_intf_def.h>
#include <fep3/rpc_services/job_registry/job_registry_service_stub.h>

namespace fep3 {
namespace native {

struct JobConfigurationToPropertyNode : fep3::catelyn::IJobConfigurationVisitor {
    JobConfigurationToPropertyNode(const std::string& job_name);
    std::shared_ptr<base::NativePropertyNode> createPropertyNode(
        const fep3::catelyn::JobConfiguration& job_configuration);

private:
    fep3::Result visitClockTriggeredConfiguration(
        const fep3::catelyn::ClockTriggeredJobConfiguration& configuration) override;
    fep3::Result visitDataTriggeredConfiguration(
        const fep3::catelyn::DataTriggeredJobConfiguration& configuration) override;

    const std::string& _job_name;
    std::shared_ptr<base::NativePropertyNode> _created_node = nullptr;
};

/**
 * @brief Configuration for the Job Registry
 */
struct JobRegistryConfiguration : public base::Configuration {
public:
    JobRegistryConfiguration();
    ~JobRegistryConfiguration() = default;

public:
    fep3::Result addJobsNode();

    fep3::Result addJobEntry(const std::string& name,
                             const fep3::catelyn::JobConfiguration& job_configuration);

    fep3::Result removeJobEntry(const std::string& name);

public:
    std::shared_ptr<base::NativePropertyNode> _jobs_node{nullptr};
};

class RPCJobRegistry;
class JobRegistryImpl;

class JobRegistry
    : public fep3::base::Component<fep3::arya::IJobRegistry, fep3::catelyn::IJobRegistry> {
public:
    explicit JobRegistry();
    ~JobRegistry();

public:
    // Inherited via IJobRegistry
    fep3::Result addJob(const std::string& name,
                        const std::shared_ptr<fep3::IJob>& job,
                        const fep3::arya::JobConfiguration& job_config) override final;
    fep3::Result addJob(const std::string& name,
                        const std::shared_ptr<fep3::arya::IJob>& job,
                        const fep3::catelyn::JobConfiguration& job_config) override final;

    fep3::Result removeJob(const std::string& name) override final;
    std::list<fep3::arya::JobInfo> getJobInfos() const override final;
    std::list<fep3::catelyn::JobInfo> getJobInfosCatelyn() const override final;

    fep3::arya::Jobs getJobs() const override final;
    fep3::catelyn::Jobs getJobsCatelyn() const override;

    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;
    fep3::Result deinitialize() override;

private:
    fep3::Result setupLogger(const IComponents& components);
    fep3::Result setupRPCJobRegistry(IServiceBus::IParticipantServer& rpc_server);

private:
    template <typename Configuration>
    fep3::Result addJobImpl(const std::string& name,
                            const std::shared_ptr<fep3::arya::IJob>& job,
                            const Configuration& job_config);

    std::unique_ptr<JobRegistryImpl> _job_registry_impl;
    bool _initialized{false};
    std::shared_ptr<const ILogger> _logger;
    std::shared_ptr<RPCJobRegistry> _rpc_job_registry{};
    JobRegistryConfiguration _job_registry_configuration;
};

class RPCJobRegistry
    : public rpc::RPCService<rpc_stubs::RPCJobRegistryServiceStub, rpc::IRPCJobRegistryDef> {
public:
    explicit RPCJobRegistry(JobRegistry& job_registry) : _job_registry(job_registry)
    {
    }

protected:
    std::string getJobNames() override;
    Json::Value getJobInfo(const std::string& job_name) override;

private:
    JobRegistry& _job_registry;
};

class RPCJobRegistryImpl : public fep3::catelyn::IJobConfigurationVisitor {
public:
    void writeJsonValue(const fep3::catelyn::JobConfiguration& job_configuration,
                        Json::Value& json_value);

private:
    fep3::Result visitClockTriggeredConfiguration(
        const fep3::catelyn::ClockTriggeredJobConfiguration& job_configuration) override;
    fep3::Result visitDataTriggeredConfiguration(
        const fep3::catelyn::DataTriggeredJobConfiguration& job_configuration) override;
    Json::Value* _current_parsed_value;
};

} // namespace native
} // namespace fep3
