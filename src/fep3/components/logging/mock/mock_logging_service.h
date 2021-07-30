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

#include <regex>
#include <functional>

#include <gmock/gmock.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/base/component.h>
#include "mock_logger.h"

namespace fep3
{
namespace mock
{

struct LoggingSink : public fep3::arya::ILoggingService::ILoggingSink
{
    MOCK_CONST_METHOD1(log, fep3::Result(arya::LogMessage log));

    MOCK_METHOD3(setProperty, bool(const std::string&, const std::string&, const std::string&));
    MOCK_CONST_METHOD1(getProperty, std::string(const std::string&));
    MOCK_CONST_METHOD1(getPropertyType, std::string(const std::string&));
    MOCK_CONST_METHOD1(isEqual, bool(const IProperties&));
    MOCK_CONST_METHOD1(copyTo,void(IProperties&));
    MOCK_CONST_METHOD0(getPropertyNames, std::vector<std::string>());
};

struct MockLoggingService : public fep3::base::Component<fep3::arya::ILoggingService>
{
    MOCK_METHOD1(createLogger, std::shared_ptr<ILogger>(const std::string&));
    MOCK_METHOD2(registerSink, fep3::Result(const std::string&, const std::shared_ptr<ILoggingSink>&));
    MOCK_METHOD1(unregisterSink, fep3::Result(const std::string&));
};

struct LoggingService : public fep3::base::Component<ILoggingService>
{
public:
    LoggingService(const std::shared_ptr<ILogger>& logger) : _logger(logger)
    {
    }
    ~LoggingService() = default;
    std::shared_ptr<ILogger> createLogger(const std::string&)
    {
        return _logger;
    }
    fep3::Result registerSink(const std::string&,
        const std::shared_ptr<ILoggingSink>&)
    {
        return {};
    }
    fep3::Result unregisterSink(const std::string&)
    {
        return {};
    }
    std::shared_ptr<ILogger> _logger;
};

} // namespace mock
} // namespace fep3
