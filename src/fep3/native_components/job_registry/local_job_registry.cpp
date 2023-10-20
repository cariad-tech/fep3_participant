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

#include "local_job_registry.h"

#include "job_registry_impl.h"

namespace fep3 {
namespace native {

JobConfigurationToPropertyNode::JobConfigurationToPropertyNode(const std::string& job_name)
    : _job_name(job_name)
{
}

std::shared_ptr<base::NativePropertyNode> JobConfigurationToPropertyNode::createPropertyNode(
    const fep3::catelyn::JobConfiguration& job_configuration)
{
    _created_node = std::make_shared<base::NativePropertyNode>(_job_name);

    _created_node->setChild(
        std::make_shared<base::NativePropertyNode>(FEP3_JOB_TRIGGER_TYPE_PROPERTY,
                                                   FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY,
                                                   base::PropertyType<std::string>::getTypeName()));

    _created_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY,
        std::to_string(job_configuration._max_runtime_real_time.value_or(Duration{0}).count()),
        base::PropertyType<int32_t>::getTypeName()));

    _created_node->setChild(
        std::make_shared<base::NativePropertyNode>(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY,
                                                   job_configuration.toString(),
                                                   base::PropertyType<std::string>::getTypeName()));

    _created_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_CYCLE_SIM_TIME_PROPERTY, "", base::PropertyType<int32_t>::getTypeName()));

    _created_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_DELAY_SIM_TIME_PROPERTY, "", base::PropertyType<int32_t>::getTypeName()));

    _created_node->setChild(base::makeNativePropertyNode<std::vector<std::string>>(
        FEP3_JOB_TRIGGER_SIGNAL_PROPERTY, {""}));

    return job_configuration.acceptVisitor(*this) ? _created_node : nullptr;
}

fep3::Result JobConfigurationToPropertyNode::visitClockTriggeredConfiguration(
    const fep3::catelyn::ClockTriggeredJobConfiguration& configuration)
{
    _created_node->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)
        ->setValue(FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY);

    _created_node->getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)
        ->setValue(std::to_string(configuration._cycle_sim_time.count()));

    _created_node->getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY)
        ->setValue(std::to_string(configuration._delay_sim_time.count()));

    return {};
}

fep3::Result JobConfigurationToPropertyNode::visitDataTriggeredConfiguration(
    const fep3::catelyn::DataTriggeredJobConfiguration& configuration)
{
    _created_node->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)
        ->setValue(FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY);

    _created_node->getChild(FEP3_JOB_TRIGGER_SIGNAL_PROPERTY)
        ->setValue(fep3::base::DefaultPropertyTypeConversion<std::vector<std::string>>::toString(
            configuration._signal_names));

    return {};
}

std::string RPCJobRegistry::getJobNames()
{
    const auto job_infos = _job_registry.getJobInfosCatelyn();
    std::string job_names;
    auto first = true;
    for (const auto& job_info: job_infos) {
        if (first) {
            job_names = job_info.getName();
            first = false;
        }
        else {
            job_names += "," + job_info.getName();
        }
    }
    return job_names;
}

void RPCJobRegistryImpl::writeJsonValue(const fep3::catelyn::JobConfiguration& job_configuration,
                                        Json::Value& json_value)
{
    _current_parsed_value = &json_value;
    json_value["job_configuration"]["max_runtime_real_time"] =
        job_configuration._max_runtime_real_time.has_value() ?
            std::to_string(job_configuration._max_runtime_real_time.value().count()) :
            "";
    json_value["job_configuration"]["runtime_violation_strategy"] = job_configuration.toString();

    job_configuration.acceptVisitor(*this);
}

fep3::Result RPCJobRegistryImpl::visitClockTriggeredConfiguration(
    const fep3::catelyn::ClockTriggeredJobConfiguration& job_configuration)
{
    Json::Value& json_value = *_current_parsed_value;
    json_value["job_configuration"]["cycle_sim_time"] = job_configuration._cycle_sim_time.count();
    json_value["job_configuration"]["delay_sim_time"] = job_configuration._delay_sim_time.count();

    return {};
}

fep3::Result RPCJobRegistryImpl::visitDataTriggeredConfiguration(
    const fep3::catelyn::DataTriggeredJobConfiguration& job_configuration)
{
    Json::Value& json_value = *_current_parsed_value;
    Json::Value value = Json::arrayValue;
    for (const auto& signal_name: job_configuration._signal_names) {
        value.append(signal_name);
    }

    json_value["job_configuration"]["trigger_signals"] = value;

    return {};
}

Json::Value RPCJobRegistry::getJobInfo(const std::string& job_name)
{
    Json::Value json_value;

    try {
        json_value["job_name"] = job_name;

        const std::unique_ptr<catelyn::JobConfiguration> job_configuration =
            _job_registry.getJobsCatelyn().at(job_name).job_info.getConfigCopy();
        RPCJobRegistryImpl rpcJobRegistryImpl;

        rpcJobRegistryImpl.writeJsonValue(*job_configuration, json_value);

        return json_value;
    }
    catch (const std::exception& /*exception*/) {
        json_value["job_name"] = "";
        json_value["job_configuration"] = "";
        return json_value;
    }
}

JobRegistryConfiguration::JobRegistryConfiguration()
    : Configuration(FEP3_JOB_REGISTRY_CONFIG),
      _jobs_node(std::make_shared<base::NativePropertyNode>(FEP3_JOBS_PROPERTY))
{
}

fep3::Result JobRegistryConfiguration::addJobsNode()
{
    auto job_registry_root_node = getNode();
    if (!job_registry_root_node) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "Job registry property root node not available.");
    }
    job_registry_root_node->setChild(_jobs_node);

    return {};
}

fep3::Result JobRegistryConfiguration::addJobEntry(
    const std::string& name, const fep3::catelyn::JobConfiguration& job_configuration)
{
    JobConfigurationToPropertyNode configuration_to_property_node{name};
    auto job_property_entry = configuration_to_property_node.createPropertyNode(job_configuration);

    _jobs_node->setChild(job_property_entry);

    return {};
}

fep3::Result JobRegistryConfiguration::removeJobEntry(const std::string& name)
{
    _jobs_node->removeChild(name);

    return {};
}

JobRegistry::JobRegistry()
    : _job_registry_impl(std::make_unique<JobRegistryImpl>(_job_registry_configuration))
{
}

JobRegistry::~JobRegistry() = default;

fep3::Result JobRegistry::create()
{
    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            "No IComponents set, can not get logging and configuration interface");
    }

    FEP3_RETURN_IF_FAILED(setupLogger(*components));

    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration service is not registered");
    }

    FEP3_RETURN_IF_FAILED(_job_registry_configuration.initConfiguration(*configuration_service));
    FEP3_RETURN_IF_FAILED(_job_registry_configuration.addJobsNode());

    const auto service_bus = components->getComponent<IServiceBus>();
    if (!service_bus) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }
    const auto rpc_server = service_bus->getServer();
    if (!rpc_server) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }

    FEP3_RETURN_IF_FAILED(setupRPCJobRegistry(*rpc_server));

    return {};
}

fep3::Result JobRegistry::destroy()
{
    _logger.reset();
    return {};
}

fep3::Result JobRegistry::initialize()
{
    _initialized = true;

    return {};
}

fep3::Result JobRegistry::deinitialize()
{
    _initialized = false;

    return {};
}

fep3::Result JobRegistry::addJob(const std::string& name,
                                 const std::shared_ptr<fep3::IJob>& job,
                                 const arya::JobConfiguration& job_config)
{
    return addJobImpl(name, job, job_config);
}

fep3::Result JobRegistry::addJob(const std::string& name,
                                 const std::shared_ptr<arya::IJob>& job,
                                 const catelyn::JobConfiguration& job_config)
{
    return addJobImpl(name, job, job_config);
}

template <typename Configuration>
fep3::Result JobRegistry::addJobImpl(const std::string& name,
                                     const std::shared_ptr<fep3::arya::IJob>& job,
                                     const Configuration& job_config)
{
    if (_initialized) {
        fep3::Result result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_STATE, "Registering a job is possible before initialization only");

        result |= _logger->logError(result.getDescription());

        return result;
    }

    auto result = _job_registry_impl->addJob(name, job, job_config);
    if (ERR_RESOURCE_IN_USE == result) {
        result |= _logger->logError(result.getDescription());
    }
    else if (!result) {
        result |= _logger->logWarning(result.getDescription());
    }

    return result;
}

fep3::Result JobRegistry::removeJob(const std::string& name)
{
    if (_initialized) {
        fep3::Result result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_STATE, "Removing a job is possible before initialization only");

        result |= _logger->logError(result.getDescription());

        return result;
    }

    auto result = _job_registry_impl->removeJob(name);
    if (ERR_NOT_FOUND == result) {
        result |= _logger->logError(result.getDescription());
    }
    else if (!result) {
        result |= _logger->logWarning(result.getDescription());
    }

    return result;
}

std::list<fep3::arya::JobInfo> JobRegistry::getJobInfos() const
{
    return _job_registry_impl->getJobInfos();
}

std::list<fep3::catelyn::JobInfo> JobRegistry::getJobInfosCatelyn() const
{
    return _job_registry_impl->getJobInfosCatelyn();
}

fep3::arya::Jobs JobRegistry::getJobs() const
{
    return _job_registry_impl->getJobs();
}

fep3::catelyn::Jobs JobRegistry::getJobsCatelyn() const
{
    return _job_registry_impl->getJobsCatelyn();
}

fep3::Result JobRegistry::setupLogger(const IComponents& components)
{
    auto logging_service = components.getComponent<arya::ILoggingService>();
    if (!logging_service) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Logging service is not registered");
    }

    _logger = logging_service->createLogger("job_registry.component");

    return {};
}

fep3::Result JobRegistry::setupRPCJobRegistry(IServiceBus::IParticipantServer& rpc_server)
{
    if (!_rpc_job_registry) {
        _rpc_job_registry = std::make_shared<RPCJobRegistry>(*this);
    }

    FEP3_RETURN_IF_FAILED(rpc_server.registerService(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
                                                     _rpc_job_registry));

    return {};
}

} // namespace native
} // namespace fep3
