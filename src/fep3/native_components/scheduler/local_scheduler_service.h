/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/native_components/job_registry/local_job_registry.h>
#include <fep3/native_components/scheduler/local_scheduler_registry.h>
#include <fep3/rpc_services/scheduler_service/scheduler_service_rpc_intf_def.h>
#include <fep3/rpc_services/scheduler_service/scheduler_service_service_stub.h>

namespace fep3 {
namespace native {

class LoggerForward : public ILogger {
public:
    LoggerForward() = default;

    void setLogger(const std::shared_ptr<const fep3::ILogger>& logger)
    {
        _logger = logger;
    }

    fep3::Result logInfo(const std::string& message) const override
    {
        if (isInfoEnabled()) {
            return _logger->logInfo(message);
        }
        return {};
    }

    fep3::Result logWarning(const std::string& message) const override
    {
        if (isWarningEnabled()) {
            return _logger->logWarning(message);
        }
        return {};
    }

    fep3::Result logError(const std::string& message) const override
    {
        if (isErrorEnabled()) {
            return _logger->logError(message);
        }
        return {};
    }

    fep3::Result logFatal(const std::string& message) const override
    {
        if (isFatalEnabled()) {
            return _logger->logFatal(message);
        }
        return {};
    }

    fep3::Result logDebug(const std::string& message) const override
    {
        if (isDebugEnabled()) {
            return _logger->logDebug(message);
        }
        return {};
    }

    bool isInfoEnabled() const override
    {
        return (_logger && _logger->isInfoEnabled());
    }

    bool isWarningEnabled() const override
    {
        return (_logger && _logger->isWarningEnabled());
    }

    bool isErrorEnabled() const override
    {
        return (_logger && _logger->isErrorEnabled());
    }

    bool isFatalEnabled() const override
    {
        return (_logger && _logger->isFatalEnabled());
    }

    bool isDebugEnabled() const override
    {
        return (_logger && _logger->isDebugEnabled());
    }

private:
    std::shared_ptr<const fep3::ILogger> _logger;
};

class LocalSchedulerService;

class RPCSchedulerService : public rpc::RPCService<rpc_stubs::RPCSchedulerServiceServiceStub,
                                                   rpc::IRPCSchedulerServiceDef> {
public:
    explicit RPCSchedulerService(LocalSchedulerService& scheduler_service)
        : _scheduler_service(scheduler_service)
    {
    }

protected:
    std::string getSchedulerNames() override;
    std::string getActiveSchedulerName() override;

private:
    LocalSchedulerService& _scheduler_service;
};

/**
 * @brief Configuration for the LocalClockService
 */
struct SchedulerServiceConfiguration : public base::Configuration {
    SchedulerServiceConfiguration();
    ~SchedulerServiceConfiguration() = default;

    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;

public:
    base::PropertyVariable<std::string> _active_scheduler_name{FEP3_SCHEDULER_CLOCK_BASED};
};

class ClockBasedScheduler;

class LocalSchedulerService : public fep3::base::Component<fep3::arya::ISchedulerService,
                                                           fep3::catelyn::ISchedulerService> {
public:
    explicit LocalSchedulerService();
    ~LocalSchedulerService();

    // base::Component
    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result tense() override;
    fep3::Result start() override;
    fep3::Result stop() override;
    fep3::Result initialize() override;
    fep3::Result relax() override;

    // Inherited via ISchedulerRegistry
    using fep3::arya::ISchedulerService::registerScheduler;
    using fep3::catelyn::ISchedulerService::registerScheduler;
    fep3::Result registerScheduler(
        std::unique_ptr<fep3::arya::IScheduler> scheduler) override final;
    fep3::Result registerScheduler(std::unique_ptr<catelyn::IScheduler> scheduler) override final;
    fep3::Result unregisterScheduler(const std::string& scheduler_name) override final;
    std::list<std::string> getSchedulerNames() const override final;
    std::string getActiveSchedulerName() const override final;

private:
    virtual fep3::Result initScheduler(const IComponents& components) const;
    void createSchedulerRegistry();
    fep3::Result setupLogger(const IComponents& components);
    fep3::Result setupRPCSchedulerService(IServiceBus::IParticipantServer& rpc_server);

private:
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<fep3::arya::IScheduler, T>>>
    fep3::Result addSchedulerToRegistry(std::unique_ptr<T> scheduler)
    {
        if (_started) {
            auto result = CREATE_ERROR_DESCRIPTION(
                ERR_INVALID_STATE, "Registering a scheduler while running is not possible");

            result |= _logger->logError(result.getDescription());

            return result;
        }

        auto result = _scheduler_registry->registerScheduler(std::move(scheduler));
        if (ERR_RESOURCE_IN_USE == result) {
            result |= _logger->logError(result.getDescription());
        }
        else if (!result) {
            result |= _logger->logWarning(result.getDescription());
        }

        return result;
    }

    std::unique_ptr<fep3::native::ClockBasedScheduler> _local_clock_based_scheduler;
    std::shared_ptr<LoggerForward> _logger_wrapper_forward;
    SchedulerServiceConfiguration _configuration;
    std::shared_ptr<RPCSchedulerService> _rpc_scheduler_service{nullptr};

protected:
    std::shared_ptr<const fep3::ILogger> _logger;
    std::atomic_bool _started{false};
    std::unique_ptr<fep3::native::LocalSchedulerRegistry> _scheduler_registry;
};

} // namespace native
} // namespace fep3
