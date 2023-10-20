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

#include "logging_config.h"
#include "logging_rpc_service.h"
#include "sinks/logging_sink_rpc.hpp"

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/clock/clock_service_intf.h>

#include <a_util/concurrency/mutex.h>

#include <functional>

namespace fep3 {
namespace native {

class LoggingServer;
class LoggingQueue;
class ClockServiceAdapter;

class LoggingService : public base::Component<ILoggingService>, base::Configuration {
public:
    class Logger : public ILogger {
        friend class LoggingService;

    public:
        explicit Logger(LoggingService& logging_service, const std::string& logger_name);

        fep3::Result logInfo(const std::string& message) const override final;
        fep3::Result logWarning(const std::string& message) const override final;
        fep3::Result logError(const std::string& message) const override final;
        fep3::Result logFatal(const std::string& message) const override final;
        fep3::Result logDebug(const std::string& message) const override final;
        bool isInfoEnabled() const override final;
        bool isWarningEnabled() const override final;
        bool isErrorEnabled() const override final;
        bool isFatalEnabled() const override final;
        bool isDebugEnabled() const override final;

    private:
        fep3::Result log(const std::string& message, LoggerSeverity severity) const;
        void releaseLogService();

    private:
        std::string _logger_name;
        LoggingService* _logging_service;
        mutable std::recursive_mutex _sync_service_access;
    };

    LoggingService();
    ~LoggingService();

    // Methods inherited from base::Component
    fep3::Result create() override;
    fep3::Result initialize() override;
    fep3::Result destroy() override;

    // Methods inherited from ILoggingService
    std::shared_ptr<ILogger> createLogger(const std::string& logger_name) override;
    fep3::Result registerSink(const std::string& name,
                              const std::shared_ptr<ILoggingSink>& sink) override;
    fep3::Result unregisterSink(const std::string& name) override;

public:
    // Methods of LoggingService
    /// Creates and sets an internal logging filter from a string reprensentation
    fep3::Result setFilter(const std::string& logger_name,
                           const LoggerFilter& filter,
                           bool overwrite = true);

    /// Gets a string reprensentation from an internal logging filter
    LoggerFilter getFilter(const std::string& logger_name) const;

    /// Sets a logging filter
    void setInternalFilter(const std::string& logger_name,
                           const native::LoggerFilterInternal& filter,
                           bool overwrite = true);

    /// Gets the internal logging filter
    const native::LoggerFilterInternal& getInternalFilter(const std::string& logger_name) const;

    std::shared_ptr<ILoggingSink> getSink(const std::string& name) const;
    std::vector<std::string> getLoggers() const;
    std::vector<std::string> getSinks() const;

private:
    /// RPC server object to set the logging filters for this participant
    std::shared_ptr<LoggingRPCService> _logging_rpc_service;
    /// Queue object so that loggers don't halt the main program
    std::unique_ptr<LoggingQueue> _queue;
    mutable a_util::concurrency::mutex _lock_queue;
    /// Configuration which logs should be filtered
    LoggingFilterTree _configuration;
    mutable a_util::concurrency::mutex _sync_config;
    /// Pointer to function for getting the current timestamp for the log
    std::function<fep3::Timestamp()> _time_getter;
    std::string _participant_name;

    std::vector<std::shared_ptr<Logger>> _loggers;
    mutable std::recursive_mutex _sync_loggers;

    std::shared_ptr<LoggingSinkRPC> _rpc_sink;
    std::map<std::string, std::shared_ptr<ILoggingSink>> _sinks;
    mutable std::recursive_mutex _sync_sinks;

    base::PropertyVariable<std::string> _default_sinks{std::string("console")};
    base::PropertyVariable<std::string> _default_file_sink_file{};
    base::PropertyVariable<int32_t> _default_severity;

    bool _default_sinks_changed{false};
};

} // namespace native
} // namespace fep3
