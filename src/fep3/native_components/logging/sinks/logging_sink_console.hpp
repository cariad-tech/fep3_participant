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

#ifdef WIN32
    #include <windows.h>
#endif

#include "logging_formater_fep.hpp"

#include <fep3/base/properties/properties.h>
#include <fep3/components/logging/logging_service_intf.h>

#include <iostream>
#include <mutex>

namespace fep3 {
namespace native {

/**
 * @brief Implementation of the console logging. Can be used as a base class for a custom sink.
 *        This sink will write fatal and error to stderr and everything else to stdout.
 *        Take care to synchronize access to stdout/stderr when using multiple console log sinks
 *        in parallel.
 */
class LoggingSinkConsole : public base::Properties<ILoggingService::ILoggingSink> {
public:
    LoggingSinkConsole()
    {
    }

    fep3::Result log(LogMessage log) const override final
    {
        const auto log_msg = _logging_formater.formatLogMessage(log);

        std::lock_guard<std::mutex> guard(_mutex);
        if (LoggerSeverity::error == log._severity || LoggerSeverity::fatal == log._severity) {
            std::cerr << log_msg << std::endl;
#ifdef WIN32
            ::OutputDebugString(log_msg.c_str());
#endif
        }
        else {
#ifdef WIN32
    #ifdef _DEBUG
            ::OutputDebugString(log_msg.c_str());
    #endif
#endif
            std::cout << log_msg << std::endl;
        }
        return {};
    }

private:
    LoggingFormaterFep _logging_formater;
    mutable std::mutex _mutex;
};

} // namespace native
} // namespace fep3
