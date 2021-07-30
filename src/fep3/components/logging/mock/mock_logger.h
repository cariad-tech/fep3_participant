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

#include <gmock/gmock.h>

#include <functional>
#include <regex>

#include <fep3/fep3_errors.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/base/component.h>

namespace fep3
{
namespace mock
{

struct LoggerMock : fep3::arya::ILogger
{
    MOCK_CONST_METHOD1(logInfo, fep3::Result(const std::string&));
    MOCK_CONST_METHOD1(logWarning, fep3::Result(const std::string&));
    MOCK_CONST_METHOD1(logError, fep3::Result(const std::string&));
    MOCK_CONST_METHOD1(logFatal, fep3::Result(const std::string&));
    MOCK_CONST_METHOD1(logDebug, fep3::Result(const std::string&));
    MOCK_CONST_METHOD0(isInfoEnabled, bool());
    MOCK_CONST_METHOD0(isWarningEnabled, bool());
    MOCK_CONST_METHOD0(isErrorEnabled, bool());
    MOCK_CONST_METHOD0(isFatalEnabled, bool());
    MOCK_CONST_METHOD0(isDebugEnabled, bool());
};

struct Logger : public LoggerMock
{
    Logger()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isWarningEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isErrorEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled())
            .WillByDefault(Return(true));
    }
};

// Logger mock which simulates not being configured for any logging severity
// and therefore logs nothing
struct InactiveLogger : public LoggerMock
{
    InactiveLogger()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled())
            .WillByDefault(Return(false));
        ON_CALL(*this, isWarningEnabled())
            .WillByDefault(Return(false));
        ON_CALL(*this, isErrorEnabled())
            .WillByDefault(Return(false));
        ON_CALL(*this, isFatalEnabled())
            .WillByDefault(Return(false));
        ON_CALL(*this, isDebugEnabled())
            .WillByDefault(Return(false));
    }
};

// Logger mock which simulates to be configured for warning, error and fatal logging severities
// and therefore logs the corresponding messages
struct WarningLogger : public LoggerMock
{
    WarningLogger()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled())
            .WillByDefault(Return(false));
        ON_CALL(*this, isWarningEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isErrorEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled())
            .WillByDefault(Return(false));
    }
};

// Logger mock which simulates to be configured for error and fatal logging severities
// and therefore logs the corresponding messages
struct ErrorLogger : public LoggerMock
{
    ErrorLogger()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled())
            .WillByDefault(Return(false));
        ON_CALL(*this, isWarningEnabled())
            .WillByDefault(Return(false));
        ON_CALL(*this, isErrorEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled())
            .WillByDefault(Return(false));
    }
};

struct LoggerWithDefaultBehaviour : public Logger
{
    LoggerWithDefaultBehaviour()
    {
        using namespace ::testing;

        ON_CALL(*this, logInfo(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logWarning(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logError(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logFatal(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logDebug(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, isInfoEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isWarningEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isErrorEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled())
            .WillByDefault(Return(true));
    }
};

MATCHER_P(LogStringRegexMatcher, regex, "Regex matcher for log string")
{
    return std::regex_search(arg, std::regex(regex));
}

} // namespace mock
} // namespace fep3
