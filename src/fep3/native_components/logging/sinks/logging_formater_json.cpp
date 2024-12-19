/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "logging_formater_json.hpp"

#include "logging_formater_common.hpp"

#include <boost/date_time.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace fep3 {
namespace native {

LoggingFormaterJson::LoggingFormaterJson() : _time_zone(native::getTimeZone())
{
}

std::string LoggingFormaterJson::formatLogMessage(const LogMessage& log) const
{
    std::string log_msg;
    std::stringstream ss;

    boost::property_tree::ptree tree;

    tree.put("timestamp", native::getLocalTime().append(_time_zone));
    tree.put("simulation_time", log._timestamp);
    tree.put("participant_name", log._participant_name);
    tree.put("logger_name", log._logger_name);
    tree.put("severity_level", getString(log._severity));
    tree.put("message", log._message);

    boost::property_tree::write_json(ss, tree);
    log_msg = ss.str();
    boost::replace_all(log_msg, "\n", "");
    boost::replace_all(log_msg, "[", "");
    boost::replace_all(log_msg, "]", "");
    boost::replace_all(log_msg, "    ", "");
    log_msg.append(",");

    return log_msg;
}

std::string LoggingFormaterJson::GetStreamBegin() const
{
    const std::string ret_string = _init_string;
    _init_string = "";
    return ret_string;
}

void LoggingFormaterJson::StreamEnd(std::ostream& log_stream) const
{
    log_stream.seekp(-_last_char_offset, std::ios_base::end);
    log_stream.write("]", 1);
    log_stream.flush();
}

bool LoggingFormaterJson::IsStreamAppendable(std::iostream& log_stream)
{
    log_stream.seekg(0, log_stream.beg);
    bool isAppendable = (log_stream.get() == '[');

    log_stream.seekg(-_last_char_offset, log_stream.end);
    isAppendable = isAppendable && (log_stream.get() == ']');

    if (isAppendable) {
        log_stream.seekp(-_last_char_offset, std::ios_base::end);
        log_stream.write(",", 1);
        _init_string = "";
    }

    log_stream.seekp(0, std::ios_base::end);
    return isAppendable;
}

} // namespace native
} // namespace fep3
