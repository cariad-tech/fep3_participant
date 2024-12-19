/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "logging_formater_intf.hpp"

namespace fep3 {
namespace native {

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
    mutable std::string _init_string = "[";
    const typename std::char_traits<char>::off_type _last_char_offset =
#ifdef __linux__
        2;
#elif _WIN32
        3;
#endif
    bool _stream_checked = false;
    const std::string _time_zone;
};

} // namespace native
} // namespace fep3
