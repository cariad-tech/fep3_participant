/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifdef WIN32
    #include <Windows.h>
#endif

#include "console_logger.h"

#include <iostream>

using namespace fep3;

ConsoleLogger::ConsoleLogger(LoggerSeverity minimum_severity) : _minimum_severity(minimum_severity)
{
}

fep3::Result ConsoleLogger::logInfo(const std::string& message) const
{
    return log(message, LoggerSeverity::info);
}

fep3::Result ConsoleLogger::logWarning(const std::string& message) const
{
    return log(message, LoggerSeverity::warning);
}

fep3::Result ConsoleLogger::logError(const std::string& message) const
{
    return log(message, LoggerSeverity::error);
}

fep3::Result ConsoleLogger::logFatal(const std::string& message) const
{
    return log(message, LoggerSeverity::fatal);
}

fep3::Result ConsoleLogger::logDebug(const std::string& message) const
{
    return log(message, LoggerSeverity::debug);
}

bool ConsoleLogger::isInfoEnabled() const
{
    return (LoggerSeverity::info <= _minimum_severity);
}

bool ConsoleLogger::isWarningEnabled() const
{
    return (LoggerSeverity::warning <= _minimum_severity);
}

bool ConsoleLogger::isErrorEnabled() const
{
    return (LoggerSeverity::error <= _minimum_severity);
}

bool ConsoleLogger::isFatalEnabled() const
{
    return (LoggerSeverity::fatal <= _minimum_severity);
}

bool ConsoleLogger::isDebugEnabled() const
{
    return (LoggerSeverity::debug <= _minimum_severity);
}

fep3::Result ConsoleLogger::log(const std::string& message, LoggerSeverity severity) const
{
    const auto& log_string = getString(severity) + " " + message;

    if (LoggerSeverity::error == severity || LoggerSeverity::fatal == severity) {
        std::cerr << log_string << std::endl;
#ifdef WIN32
        ::OutputDebugString(log_string.c_str());
#endif
    }
    else {
#ifdef WIN32
    #ifdef _DEBUG
        ::OutputDebugString(log_string.c_str());
    #endif
#endif
        std::cout << log_string << std::endl;
    }

    return {};
}
