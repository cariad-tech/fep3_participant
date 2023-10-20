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

#include "logging_formater_intf.hpp"

#include <a_util/datetime.h>
#include <a_util/strings.h>

namespace fep3 {
namespace native {

/**
 * @brief Class for formatting log messages using a fep specific format.
 */
class LoggingFormaterFep : public ILoggingFormater {
public:
    std::string formatLogMessage(const LogMessage& log) const override
    {
        std::string log_msg;

        log_msg.append(a_util::strings::format(
            "[%s - %s]: ",
            a_util::datetime::getCurrentLocalDate().format("%d.%m.%Y").c_str(),
            a_util::datetime::getCurrentLocalTime().format("%H:%M:%S").c_str()));

        log_msg.append(a_util::strings::format(
            "%s@%s ", log._participant_name.c_str(), log._logger_name.c_str()));
        log_msg.append(a_util::strings::format(" ST: %s[ns]  ", log._timestamp.c_str()));

        log_msg.append(getString(log._severity));

        log_msg.append(a_util::strings::format(" %s", log._message.c_str()));

        return log_msg;
    }
};

} // namespace native
} // namespace fep3
