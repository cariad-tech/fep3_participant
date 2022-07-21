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
#include "logging_formater_json.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

#include "boost/date_time.hpp"

namespace
{
    constexpr size_t timezone_string_size = 3;

    std::string getTimeZone()
    {
        std::stringstream ss;
#ifdef __linux__ 
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        ss << std::put_time(&tm, "%z") << '\n';
#elif _WIN32
        struct tm newtime;
        time_t now = time(0);
        localtime_s(&newtime, &now);
        ss << std::put_time(&newtime, "%z") << '\n';
#endif
        std::string timeZoneString = ss.str();
        timeZoneString.resize(timezone_string_size);
        return timeZoneString;
    }
}

namespace fep3
{

    LoggingFormaterJson::LoggingFormaterJson()
        : _timeZone(getTimeZone())
    {

    }

    std::string LoggingFormaterJson::formatLogMessage(const LogMessage& log) const
    {
        boost::posix_time::ptime localTime(boost::posix_time::microsec_clock::local_time());
        std::string  isoTimeFormat = boost::posix_time::to_iso_extended_string(localTime);
        checkFractionalSeconds(isoTimeFormat);
        isoTimeFormat.append(_timeZone);

        std::string log_msg;
        std::stringstream ss;

        boost::property_tree::ptree tree;

        tree.put("timestamp", isoTimeFormat);
        tree.put("severity_level", getString(log._severity));
        tree.put("logger_name", log._logger_name);
        tree.put("message", log._message);
        tree.put("participant_name", log._participant_name);
        tree.put("log_type", "message");

        boost::property_tree::write_json(ss, tree);
        log_msg = ss.str();
        boost::replace_all(log_msg, "\n", "");
        boost::replace_all(log_msg, "[", "");
        boost::replace_all(log_msg, "]", "");
        boost::replace_all(log_msg, "    ", "");
        log_msg.append(",");

        return log_msg;
    }

    void LoggingFormaterJson::checkFractionalSeconds(std::string& timeString) const
    {
        auto dotPos = timeString.find('.');
        if ((dotPos) == std::string::npos)
        {
            timeString.append(",0");
        }
        else
        {
            timeString.replace(dotPos, 1, ",");
        }

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

    bool  LoggingFormaterJson::IsStreamAppendable(std::iostream& log_stream)
    {
        log_stream.seekg(0, log_stream.beg);
        bool isAppendable = (log_stream.get() == '[');

        log_stream.seekg(-_last_char_offset, log_stream.end);
        isAppendable = isAppendable && (log_stream.get() == ']');

        if (isAppendable)
        {
            log_stream.seekp(-_last_char_offset, std::ios_base::end);
            log_stream.write(",", 1);
            _init_string = "";
        }

        log_stream.seekp(0, std::ios_base::end);
        return isAppendable;
    }

} // namespace fep3
