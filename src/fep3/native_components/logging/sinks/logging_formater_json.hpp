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

#include "logging_formater_intf.hpp"

namespace fep3
{
    /**
     * @brief Class for formatting log messages into line oriented json.
     */
    class LoggingFormaterJson : public ILoggingFormater {
    public:
        LoggingFormaterJson();

        std::string formatLogMessage(const LogMessage& log) const override;

        std::string GetStreamBegin() const override;

        void StreamEnd(std::ostream& log_stream) const override;

        bool IsStreamAppendable(std::iostream& log_stream) override;

    private:
        void checkFractionalSeconds(std::string& timeString) const;

        mutable std::string _init_string = "[";
        const typename std::char_traits<char>::off_type _last_char_offset =
#ifdef __linux__ 
            2;
#elif _WIN32
            3;
#endif
        bool _stream_checked = false;
        const std::string _timeZone;
    };
} // namespace fep3
