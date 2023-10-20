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

#include <fep3/base/logging/logging_types.h>
#include <fep3/components/logging/logger_intf.h>

namespace fep3 {

class ConsoleLogger : public ILogger {
    friend class LoggingService;

public:
    ConsoleLogger(LoggerSeverity minimum_severity);

    fep3::Result logInfo(const std::string& message) const override;
    fep3::Result logWarning(const std::string& message) const override;
    fep3::Result logError(const std::string& message) const override;
    fep3::Result logFatal(const std::string& message) const override;
    fep3::Result logDebug(const std::string& message) const override;
    bool isInfoEnabled() const override;
    bool isWarningEnabled() const override;
    bool isErrorEnabled() const override;
    bool isFatalEnabled() const override;
    bool isDebugEnabled() const override;

private:
    fep3::Result log(const std::string& message, LoggerSeverity severity) const;

private:
    LoggerSeverity _minimum_severity;
};

} // namespace fep3
