/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

@endverbatim
 */

#pragma once

#include "logging_formater_common.hpp"
#include "logging_formater_intf.hpp"

#include <a_util/datetime.h>
#include <a_util/strings.h>

#include <boost/date_time.hpp>

namespace fep3 {
namespace native {

constexpr auto csv_log_header =
    "# timestamp\tsimulation_time[ns]\tlogger_name\tparticipant_name\tseverity_level\tmessage\n";

/**
 * @brief Class for formatting log messages using a tab separated, csv like format.
 */
class LoggingFormaterCsv : public ILoggingFormater {
private:
    const std::string _time_zone;

public:
    LoggingFormaterCsv() : _time_zone(native::getTimeZone())
    {
    }

    std::string GetStreamBegin() const
    {
        return csv_log_header;
    };

    bool IsStreamAppendable(std::iostream& log_stream)
    {
        log_stream.seekg(0, log_stream.beg);
        if (log_stream.peek() == std::ifstream::traits_type::eof()) {
            return true;
        }
        log_stream.seekg(0, std::ios_base::end);

        log_stream.seekp(0, std::ios_base::end);
        log_stream.write("\n", 1);

        return true;
    }

    std::string formatLogMessage(const LogMessage& log) const override
    {
        std::string log_msg;

        log_msg.append(
            a_util::strings::format("%s\t", native::getLocalTime().append(_time_zone).c_str()));
        log_msg.append(a_util::strings::format("%s\t", log._timestamp.c_str()));
        log_msg.append(a_util::strings::format("%s\t", log._participant_name.c_str()));
        log_msg.append(a_util::strings::format("%s\t", log._logger_name.c_str()));
        log_msg.append(a_util::strings::format("%s\t", getString(log._severity).c_str()));
        log_msg.append(log._message.c_str());

        boost::replace_all(log_msg, "\n", "\\n");

        return log_msg;
    }
};

} // namespace native
} // namespace fep3
