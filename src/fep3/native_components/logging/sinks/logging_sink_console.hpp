/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#pragma once

#include "logging_sink_common.hpp"

#include <a_util/concurrency/mutex.h>

#include <fep3/base/properties/properties.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <iostream>
#include <mutex>
#ifdef WIN32
#include <windows.h>
#endif

namespace fep3 {
/**
 * @brief Implementation of the console logging. Can be used as a base class for a custom sink.
 *        This sink will write fatal and error to stderr and everything else to stdout.
 */
class LoggingSinkConsole : public base::Properties<ILoggingService::ILoggingSink> {
public:
    LoggingSinkConsole() {}
    fep3::Result log(LogMessage log) const override
    {
        std::string log_msg;
        native::formatLoggingString(log_msg, log);

        std::unique_lock<std::mutex> guard(getConsoleMutex());
        if (LoggerSeverity::error == log._severity || LoggerSeverity::fatal == log._severity)
        {
            std::cerr << log_msg << std::endl;
#ifdef WIN32
            ::OutputDebugString(log_msg.c_str());
#endif
        }
        else
        {
#ifdef WIN32
#ifdef _DEBUG
            ::OutputDebugString(log_msg.c_str());
#endif
#endif
            std::cout << log_msg << std::endl;
        }
        return{};
    }

protected:
    /**
    * @brief Mutex for the console to ensure single point access (Meyers' singleton)
    * @return The static mutex variable
    */
    static a_util::concurrency::mutex& getConsoleMutex()
    {
        static a_util::concurrency::mutex _console_mutex;
        return _console_mutex;
    }
};
} // namespace fep3
