/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

@endverbatim
 */

#pragma once

#include <boost/date_time.hpp>

namespace fep3 {
namespace native {

constexpr size_t timezone_string_size = 3;

inline std::string getTimeZone()
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

inline void checkFractionalSeconds(std::string& timeString)
{
    // A decimal mark, either a comma or a dot, is used as a separator between
    // the time element and its fraction. (Following ISO 80000-1 according to
    // ISO 8601:1-2019 it does not stipulate a preference except within
    // International Standards, but with a preference for a comma according to
    // ISO 8601:2004)
    // https://en.wikipedia.org/wiki/ISO_8601
    auto dotPos = timeString.find('.');
    if ((dotPos) == std::string::npos) {
        timeString.append(",0");
    }
    else {
        timeString.replace(dotPos, 1, ",");
    }
}

inline std::string getLocalTime()
{
    boost::posix_time::ptime localTime(boost::posix_time::microsec_clock::local_time());
    std::string isoTimeFormat = boost::posix_time::to_iso_extended_string(localTime);
    checkFractionalSeconds(isoTimeFormat);
    return isoTimeFormat;
}

} // namespace native
} // namespace fep3
