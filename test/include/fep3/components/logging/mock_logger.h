/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/logging/logging_types.h>
#include <fep3/components/logging/logger_intf.h>

#include <gmock/gmock.h>

#include <regex>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

/**
 * @brief Mock class for @ref fep3::arya::ILogger
 */
struct Logger : fep3::arya::ILogger {
    MOCK_METHOD(fep3::Result, logInfo, (const std::string&), (const, override));
    MOCK_METHOD(fep3::Result, logWarning, (const std::string&), (const, override));
    MOCK_METHOD(fep3::Result, logError, (const std::string&), (const, override));
    MOCK_METHOD(fep3::Result, logFatal, (const std::string&), (const, override));
    MOCK_METHOD(fep3::Result, logDebug, (const std::string&), (const, override));
    MOCK_METHOD(bool, isInfoEnabled, (), (const, override));
    MOCK_METHOD(bool, isWarningEnabled, (), (const, override));
    MOCK_METHOD(bool, isErrorEnabled, (), (const, override));
    MOCK_METHOD(bool, isFatalEnabled, (), (const, override));
    MOCK_METHOD(bool, isDebugEnabled, (), (const, override));
};

/**
 * @brief Regular expresssion matcher for a log string
 */
MATCHER_P(LogStringRegexMatcher, regex, "Regex matcher for log string")
{
    return std::regex_search(arg, std::regex(regex));
}

} // namespace arya
using arya::Logger;
using arya::LogStringRegexMatcher;

inline bool areEqual(const fep3::LogMessage& lhs, const fep3::LogMessage& rhs)
{
    return lhs._timestamp == rhs._timestamp && lhs._severity == rhs._severity &&
           lhs._participant_name == rhs._participant_name && lhs._logger_name == rhs._logger_name &&
           std::regex_search(lhs._message, std::regex(rhs._message));
}

/**
 * @brief Matcher for a log message.
 * The log message's message can be a regex.
 */
MATCHER_P(LogMessageMatcher, message, "Matcher for log message")
{
    return areEqual(arg, message);
}

} // namespace mock
} // namespace fep3

///@endcond nodoc