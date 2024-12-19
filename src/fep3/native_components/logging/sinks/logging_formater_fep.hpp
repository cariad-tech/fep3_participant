/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "logging_formater_intf.hpp"

#include <a_util/strings.h>

#include <ctime>
#include <iomanip>
#include <sstream>

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

        std::time_t now = std::time(nullptr);
        std::tm tm{};

#if defined(_MSC_VER)
        localtime_s(&tm, &now);
#else
        localtime_r(&now, &tm);
#endif
        const auto date_string = std::put_time(&tm, "%d.%m.%Y");
        const auto time_string = std::put_time(&tm, "%H:%M:%S");
        std::stringstream ss_date;
        std::stringstream ss_time;
        ss_date << date_string;
        ss_time << time_string;

        log_msg.append(
            a_util::strings::format("[%s - %s]: ", ss_date.str().c_str(), ss_time.str().c_str()));

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
