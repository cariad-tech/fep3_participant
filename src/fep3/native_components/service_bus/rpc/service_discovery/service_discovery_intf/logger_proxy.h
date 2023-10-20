#pragma once

#include <fep3/components/logging/logging_service_intf.h>

#include <functional>
#include <shared_mutex>

// TODO Code duplicate with local_scheduler_service.h -> put this header in one place
namespace fep3 {
namespace native {
class LoggerProxy : public ILogger {
public:
    explicit LoggerProxy(std::shared_ptr<const fep3::ILogger> logger = nullptr)
        : _logger(std::move(logger))
    {
    }

    void setLogger(const std::shared_ptr<const fep3::ILogger>& logger)
    {
        std::unique_lock lock(_mutex);
        _logger = logger;
    }

    fep3::Result logInfo(const std::string& message) const override
    {
        return log(&fep3::ILogger::isInfoEnabled, &fep3::ILogger::logInfo, message);
    }

    fep3::Result logWarning(const std::string& message) const override
    {
        return log(&fep3::ILogger::isWarningEnabled, &fep3::ILogger::logWarning, message);
    }

    fep3::Result logError(const std::string& message) const override
    {
        return log(&fep3::ILogger::isErrorEnabled, &fep3::ILogger::logError, message);
    }

    fep3::Result logFatal(const std::string& message) const override
    {
        return log(&fep3::ILogger::isFatalEnabled, &fep3::ILogger::logFatal, message);
    }

    fep3::Result logDebug(const std::string& message) const override
    {
        return log(&fep3::ILogger::isDebugEnabled, &fep3::ILogger::logDebug, message);
    }

    bool isInfoEnabled() const override
    {
        return isSeverityEnabled(&fep3::ILogger::isInfoEnabled);
    }

    bool isWarningEnabled() const override
    {
        return isSeverityEnabled(&fep3::ILogger::isWarningEnabled);
    }

    bool isErrorEnabled() const override
    {
        return isSeverityEnabled(&fep3::ILogger::isErrorEnabled);
    }

    bool isFatalEnabled() const override
    {
        return isSeverityEnabled(&fep3::ILogger::isFatalEnabled);
    }

    bool isDebugEnabled() const override
    {
        return isSeverityEnabled(&fep3::ILogger::isDebugEnabled);
    }

private:
    bool isSeverityEnabled(
        std::function<bool(std::shared_ptr<const fep3::ILogger>)> get_severity_function) const
    {
        std::shared_lock lock(_mutex);
        return (_logger && std::invoke(get_severity_function, _logger));
    }

    fep3::Result log(
        std::function<bool(std::shared_ptr<const fep3::ILogger>)> get_severity_function,
        std::function<fep3::Result(std::shared_ptr<const fep3::ILogger>, const std::string&)>
            log_function,
        const std::string& message) const
    {
        if (isSeverityEnabled(get_severity_function)) {
            std::shared_lock lock(_mutex);
            return std::invoke(log_function, _logger, message);
        }
        else {
            return {};
        }
    }

private:
    std::shared_ptr<const fep3::ILogger> _logger;
    mutable std::shared_mutex _mutex;
};

} // namespace native
} // namespace fep3
