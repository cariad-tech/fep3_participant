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

#include "logging_service.h"

#include "../../logging/default_severity_from_env_variable.h"
#include "logging_queue.h"
#include "sinks/logging_sink_console.hpp"
#include "sinks/logging_sink_file.hpp"

#include <fep3/native_components/clock/variant_handling/clock_service_handling.h>

using namespace fep3;
using namespace fep3::native;

LoggingService::Logger::Logger(LoggingService& logging_service, const std::string& logger_name)
    : _logging_service(&logging_service), _logger_name(logger_name)
{
}

void LoggingService::Logger::releaseLogService()
{
    std::lock_guard<std::recursive_mutex> lock(_sync_service_access);
    _logging_service = nullptr;
}

fep3::Result LoggingService::Logger::logInfo(const std::string& message) const
{
    return log(message, LoggerSeverity::info);
}

fep3::Result LoggingService::Logger::logWarning(const std::string& message) const
{
    return log(message, LoggerSeverity::warning);
}

fep3::Result LoggingService::Logger::logError(const std::string& message) const
{
    return log(message, LoggerSeverity::error);
}

fep3::Result LoggingService::Logger::logFatal(const std::string& message) const
{
    return log(message, LoggerSeverity::fatal);
}

fep3::Result LoggingService::Logger::logDebug(const std::string& message) const
{
    return log(message, LoggerSeverity::debug);
}

bool LoggingService::Logger::isInfoEnabled() const
{
    std::lock_guard<std::recursive_mutex> service_lock(_sync_service_access);
    std::lock_guard<a_util::concurrency::mutex> config_lock(_logging_service->_sync_config);
    return (_logging_service &&
            (LoggerSeverity::info <= _logging_service->getInternalFilter(_logger_name)._severity));
}

bool LoggingService::Logger::isWarningEnabled() const
{
    std::lock_guard<std::recursive_mutex> service_lock(_sync_service_access);
    std::lock_guard<a_util::concurrency::mutex> config_lock(_logging_service->_sync_config);
    return (_logging_service && (LoggerSeverity::warning <=
                                 _logging_service->getInternalFilter(_logger_name)._severity));
}

bool LoggingService::Logger::isErrorEnabled() const
{
    std::lock_guard<std::recursive_mutex> service_lock(_sync_service_access);
    std::lock_guard<a_util::concurrency::mutex> config_lock(_logging_service->_sync_config);
    return (_logging_service &&
            (LoggerSeverity::error <= _logging_service->getInternalFilter(_logger_name)._severity));
}

bool LoggingService::Logger::isFatalEnabled() const
{
    std::lock_guard<std::recursive_mutex> service_lock(_sync_service_access);
    std::lock_guard<a_util::concurrency::mutex> config_lock(_logging_service->_sync_config);
    return (_logging_service &&
            (LoggerSeverity::fatal <= _logging_service->getInternalFilter(_logger_name)._severity));
}

bool LoggingService::Logger::isDebugEnabled() const
{
    std::lock_guard<std::recursive_mutex> service_lock(_sync_service_access);
    std::lock_guard<a_util::concurrency::mutex> config_lock(_logging_service->_sync_config);
    return (_logging_service &&
            (LoggerSeverity::debug <= _logging_service->getInternalFilter(_logger_name)._severity));
}

fep3::Result LoggingService::Logger::log(const std::string& message, LoggerSeverity severity) const
{
    std::lock_guard<std::recursive_mutex> service_lock(_sync_service_access);
    fep3::Result result = ERR_NOERROR;

    // Get Metadata and create Log Message
    std::string timestamp{"0"};
    std::string part_name{};
    if (_logging_service) {
        if (_logging_service->_time_getter) {
            timestamp = a_util::strings::toString(_logging_service->_time_getter().count());
        }

        LogMessage log_message = {
            timestamp, severity, _logging_service->_participant_name, _logger_name, message};

        // Get Filter and log to all enabled sinks
        std::lock_guard<a_util::concurrency::mutex> config_lock(_logging_service->_sync_config);
        const LoggerFilterInternal& filter = _logging_service->getInternalFilter(_logger_name);
        if (severity <= filter._severity) {
            for (const auto& logging_sink: filter._logging_sinks) {
                std::unique_lock<std::mutex> guard(_logging_service->_lock_queue);
                auto fcn = [log_message, logging_sink]() {
                    // No data race here since we are using capture by value and
                    // the reference count of the logging_sink shared_ptr ensures that the sink
                    // object still exists
                    logging_sink.second->log(log_message);
                };
                result |= _logging_service->_queue->add(fcn);
            }
        }
    }

    return result;
}

LoggingService::LoggingService()
    : base::Configuration(FEP3_LOGGING_SERVICE_CONFIG),
      _default_severity(static_cast<int32_t>(fep3::base::getDefaultLoggingSeverity()))
{
    _queue = std::make_unique<LoggingQueue>();

    registerPropertyVariable(_default_sinks, FEP3_LOGGING_DEFAULT_SINKS_PROPERTY);
    registerPropertyVariable(_default_severity, FEP3_LOGGING_DEFAULT_SEVERITY_PROPERTY);
    registerPropertyVariable(_default_file_sink_file, FEP3_LOGGING_DEFAULT_FILE_SINK_PROPERTY);

    // init the default sinks
    registerSink("console", std::make_shared<LoggingSinkConsole>());
    registerSink("file", std::make_shared<LoggingSinkFileCsv>());
    registerSink("file_json", std::make_shared<LoggingSinkFileJson>());

    std::lock_guard<a_util::concurrency::mutex> lock(_sync_config);
    setInternalFilter("",
                      {static_cast<LoggerSeverity>(static_cast<int32_t>(_default_severity)),
                       {{"console", getSink("console")}}});
}

LoggingService::~LoggingService()
{
    std::lock_guard<std::recursive_mutex> lock(_sync_loggers);
    // this is to make sure there is no logger in the world anymore which logs
    // on the reference of this loggingservice
    // this would make boom!
    for (auto& logger: _loggers) {
        logger->releaseLogService();
    }
    _loggers.clear();
}

fep3::Result LoggingService::create()
{
    auto components = _components.lock();
    if (components) {
        // clockservice is optional
        const auto [result, clock_service] = getClockServiceAdapter(*components, nullptr);
        if (result) {
            _time_getter = clock_service->getTimeGetter();
        }

        // service bus is not optional at the moment
        auto service_bus = components->getComponent<IServiceBus>();
        if (service_bus) {
            auto rpc_server = service_bus->getServer();
            if (rpc_server) {
                _participant_name = rpc_server->getName();
                _logging_rpc_service = std::make_shared<LoggingRPCService>(*this);
                FEP3_RETURN_IF_FAILED(rpc_server->registerService(
                    ::fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(), _logging_rpc_service));

                _rpc_sink = std::make_shared<LoggingSinkRPC>(*service_bus);
                // now we cann add the logging sink for the rpc
                registerSink("rpc", _rpc_sink);
                // we change the default properties to log every thing on console AND RPC
                getNode()
                    ->getChild(FEP3_LOGGING_DEFAULT_SINKS_PROPERTY)
                    ->setValue("console,rpc", base::PropertyType<std::string>::getTypeName());
                std::lock_guard<a_util::concurrency::mutex> lock(_sync_config);
                setInternalFilter(
                    "",
                    {static_cast<LoggerSeverity>(static_cast<int32_t>(_default_severity)),
                     {{"rpc", getSink("rpc")}, {"console", getSink("console")}}});
            }
            else {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
            }
        }
        else {
            // no rpc possible
            // and no participant name can be obtain!!
            // but i think this is okay
        }
        auto config_serv = components->getComponent<IConfigurationService>();
        if (config_serv) {
            const auto ret_val = initConfiguration(*config_serv);
            if (!ret_val) {
                return ret_val;
            }
        }
    }
    return {};
}

fep3::Result LoggingService::initialize()
{
    // Set default filter from the logging service properties
    if (!_default_sinks_changed) {
        updatePropertyVariables();
        std::vector<std::string> enabled_logging_sinks =
            a_util::strings::split(static_cast<std::string>(_default_sinks), ",");
        LoggerFilter filter{static_cast<LoggerSeverity>(static_cast<int32_t>(_default_severity)),
                            enabled_logging_sinks};

        setFilter("", filter, false);
    }

    // Set default file sink file
    if (!_default_file_sink_file.toString().empty()) {
        std::shared_ptr<ILoggingService::ILoggingSink> file_sink = getSink("file");
        if (!file_sink) {
            RETURN_ERROR_DESCRIPTION(
                ERR_NOT_FOUND,
                "Unable to set default file sink file: The sink \"file\" is not registered.");
        }
        file_sink->setProperty(
            "file_path", _default_file_sink_file.toString(), _default_file_sink_file.getTypeName());
    }

    return {};
}

fep3::Result LoggingService::destroy()
{
    unregisterSink("rpc");
    _rpc_sink->releaseServiceBus();
    deinitConfiguration();
    _time_getter = {};
    return {};
}

std::shared_ptr<ILogger> LoggingService::createLogger(const std::string& logger_name)
{
    std::lock_guard<std::recursive_mutex> lock(_sync_loggers);
    auto new_logger = std::make_shared<Logger>(*this, logger_name);
    _loggers.push_back(new_logger);
    return new_logger;
}

fep3::Result LoggingService::registerSink(const std::string& name,
                                          const std::shared_ptr<ILoggingSink>& sink)
{
    std::lock_guard<std::recursive_mutex> lock(_sync_sinks);
    const auto& found = _sinks.find(name);
    if (found != _sinks.cend()) {
        RETURN_ERROR_DESCRIPTION(
            ERR_RESOURCE_IN_USE, "A logging sink with the name %s already exists", name.c_str());
    }
    else {
        _sinks[name] = sink;
        return {};
    }
}

fep3::Result LoggingService::unregisterSink(const std::string& name)
{
    std::lock_guard<std::recursive_mutex> lock(_sync_sinks);
    const auto& found = _sinks.find(name);
    if (found != _sinks.cend()) {
        RETURN_ERROR_DESCRIPTION(
            ERR_NOT_FOUND, "A logging sink with the name %s does not exist", name.c_str());
    }
    else {
        _sinks.erase(name);
        return {};
    }
}

fep3::Result LoggingService::setFilter(const std::string& logger_name,
                                       const LoggerFilter& filter,
                                       bool overwrite)
{
    LoggerFilterInternal new_filter{filter._severity, {}};

    for (const auto& sink_name: filter._enabled_logging_sinks) {
        auto sink_found = getSink(sink_name);
        if (sink_found) {
            new_filter._logging_sinks[sink_name] = sink_found;
        }
        else {
            return ERR_NOT_FOUND;
        }
    }

    if (logger_name.empty()) {
        _default_sinks_changed = true;
    }

    std::lock_guard<a_util::concurrency::mutex> lock(_sync_config);
    setInternalFilter(logger_name, new_filter, overwrite);
    return {};
}

LoggerFilter LoggingService::getFilter(const std::string& logger_name) const
{
    std::lock_guard<a_util::concurrency::mutex> lock(_sync_config);
    auto internal_filter = getInternalFilter(logger_name);
    LoggerFilter filter = {internal_filter._severity, {}};
    for (const auto& current_sink: internal_filter._logging_sinks) {
        filter._enabled_logging_sinks.push_back(current_sink.first);
    }
    return filter;
}

void LoggingService::setInternalFilter(const std::string& logger_name,
                                       const native::LoggerFilterInternal& filter,
                                       bool overwrite)
{
    _configuration.setLoggerFilter(logger_name, filter, overwrite);
}

const native::LoggerFilterInternal& LoggingService::getInternalFilter(
    const std::string& logger_name) const
{
    return _configuration.getLoggerFilter(logger_name);
}

std::vector<std::string> LoggingService::getLoggers() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_loggers);
    std::vector<std::string> ret;
    for (const auto& logger: _loggers) {
        ret.push_back(logger->_logger_name);
    }
    return ret;
}

std::vector<std::string> LoggingService::getSinks() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_sinks);
    std::vector<std::string> ret;
    for (const auto& sink: _sinks) {
        ret.push_back(sink.first);
    }
    return ret;
}

std::shared_ptr<ILoggingService::ILoggingSink> LoggingService::getSink(
    const std::string& name) const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_sinks);
    const auto& it = _sinks.find(name);
    if (it != _sinks.cend()) {
        return it->second;
    }
    return {};
}
