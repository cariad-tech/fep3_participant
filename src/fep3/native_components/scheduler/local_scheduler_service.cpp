/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "local_scheduler_service.h"

#include "clock_based/clock_based_scheduler.h"

namespace fep3 {
namespace native {

SchedulerServiceConfiguration::SchedulerServiceConfiguration()
    : Configuration(FEP3_SCHEDULER_SERVICE_CONFIG)
{
}

fep3::Result SchedulerServiceConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(
        registerPropertyVariable(_active_scheduler_name, FEP3_SCHEDULER_PROPERTY));

    return {};
}

fep3::Result SchedulerServiceConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(
        unregisterPropertyVariable(_active_scheduler_name, FEP3_SCHEDULER_PROPERTY));

    return {};
}

std::string RPCSchedulerService::getSchedulerNames()
{
    const auto scheduler_names_list = _scheduler_service.getSchedulerNames();
    auto first = true;
    std::string return_string;
    for (auto& scheduler_name: scheduler_names_list) {
        if (first) {
            return_string = scheduler_name;
            first = false;
        }
        else {
            return_string += "," + scheduler_name;
        }
    }
    return return_string;
}

std::string RPCSchedulerService::getActiveSchedulerName()
{
    return _scheduler_service.getActiveSchedulerName();
}

LocalSchedulerService::LocalSchedulerService()
    : _logger_wrapper_forward(std::make_shared<LoggerForward>())
{
    createSchedulerRegistry();
}
LocalSchedulerService::~LocalSchedulerService()
{
}

void LocalSchedulerService::createSchedulerRegistry()
{
    std::unique_ptr<fep3::catelyn::IScheduler> local_clock_based_scheduler =
        std::make_unique<ClockBasedScheduler>(_logger_wrapper_forward);
    _scheduler_registry = std::make_unique<fep3::native::LocalSchedulerRegistry>(
        std::move(local_clock_based_scheduler));
}

fep3::Result LocalSchedulerService::create()
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

    FEP3_RETURN_IF_FAILED(_configuration.initConfiguration(*configuration_service));

    const auto service_bus = components->getComponent<IServiceBus>();
    if (!service_bus) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }
    const auto rpc_server = service_bus->getServer();
    if (!rpc_server) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }

    FEP3_RETURN_IF_FAILED(setupRPCSchedulerService(*rpc_server));

    return {};
}

fep3::Result LocalSchedulerService::destroy()
{
    _logger.reset();
    _logger_wrapper_forward->setLogger(_logger);

    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            "No IComponents set, can not get logging and configuration interface");
    }
    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration service is not registered");
    }

    _configuration.deinitConfiguration();

    return {};
}

fep3::Result LocalSchedulerService::initialize()
{
    return {};
}

fep3::Result LocalSchedulerService::tense()
{
    _configuration.updatePropertyVariables();

    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "access to components was not possible");
    }

    FEP3_RETURN_IF_FAILED(
        _scheduler_registry->setActiveScheduler(_configuration._active_scheduler_name));

    FEP3_RETURN_IF_FAILED(initScheduler(*components));

    return {};
}

fep3::Result LocalSchedulerService::initScheduler(const IComponents& components) const
{
    const auto health_service = components.getComponent<IHealthService>();
    if (!health_service) {
        const auto clock_service = components.getComponent<fep3::arya::IClockService>();
        if (!clock_service) {
            RETURN_ERROR_DESCRIPTION(
                ERR_NOT_FOUND,
                "%s is not part of the given component registry",
                fep3::arya::getComponentIID<fep3::arya::IClockService>().c_str());
        }

        const auto job_registry = components.getComponent<fep3::IJobRegistry>();
        if (!job_registry) {
            RETURN_ERROR_DESCRIPTION(ERR_POINTER,
                                     "access to component IJobRegistry was not possible");
        }

        const auto jobs = job_registry->getJobs();

        FEP3_RETURN_IF_FAILED(_scheduler_registry->initializeActiveScheduler(*clock_service, jobs));
    }
    else {
        FEP3_RETURN_IF_FAILED(_scheduler_registry->initializeActiveScheduler(components));
    }
    return {};
}

fep3::Result LocalSchedulerService::relax()
{
    stop();
    FEP3_RETURN_IF_FAILED(_scheduler_registry->deinitializeActiveScheduler());

    return {};
}

fep3::Result LocalSchedulerService::registerScheduler(std::unique_ptr<arya::IScheduler> scheduler)
{
    return addSchedulerToRegistry(std::move(scheduler));
}

fep3::Result LocalSchedulerService::registerScheduler(
    std::unique_ptr<catelyn::IScheduler> scheduler)
{
    return addSchedulerToRegistry(std::move(scheduler));
}

fep3::Result LocalSchedulerService::unregisterScheduler(const std::string& scheduler_name)
{
    if (_started) {
        auto result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_STATE, "Unregistering a scheduler while running is not possible");

        result |= _logger->logError(result.getDescription());

        return result;
    }

    auto result = _scheduler_registry->unregisterScheduler(scheduler_name);

    if (ERR_NOT_FOUND == result) {
        result |= _logger->logError(result.getDescription());
    }
    else if (!result) {
        result |= _logger->logWarning(result.getDescription());
    }

    return result;
}

std::list<std::string> LocalSchedulerService::getSchedulerNames() const
{
    return _scheduler_registry->getSchedulerNames();
}

std::string LocalSchedulerService::getActiveSchedulerName() const
{
    return _scheduler_registry->getActiveSchedulerName();
}

fep3::Result LocalSchedulerService::start()
{
    _started = true;
    return _scheduler_registry->startActiveScheduler();
}

fep3::Result LocalSchedulerService::stop()
{
    auto result = _scheduler_registry->stopActiveScheduler();
    _started = false;

    return result;
}

fep3::Result LocalSchedulerService::setupLogger(const IComponents& components)
{
    auto logging_service = components.getComponent<arya::ILoggingService>();
    if (!logging_service) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Logging service is not registered");
    }

    _logger = logging_service->createLogger("scheduler_service.component");
    _logger_wrapper_forward->setLogger(_logger);

    return {};
}

fep3::Result LocalSchedulerService::setupRPCSchedulerService(
    IServiceBus::IParticipantServer& rpc_server)
{
    if (!_rpc_scheduler_service) {
        _rpc_scheduler_service = std::make_shared<RPCSchedulerService>(*this);
    }

    FEP3_RETURN_IF_FAILED(rpc_server.registerService(
        rpc::IRPCSchedulerServiceDef::getRPCDefaultName(), _rpc_scheduler_service));

    return {};
}

} // namespace native
} // namespace fep3
