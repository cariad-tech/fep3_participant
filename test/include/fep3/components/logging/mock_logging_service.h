/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/components/base/component.h>
#include <fep3/components/logging/logging_service_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

struct LoggingService : public fep3::base::arya::Component<fep3::arya::ILoggingService> {
    struct LoggingSink : public fep3::arya::ILoggingService::ILoggingSink {
        MOCK_METHOD(fep3::Result, log, (fep3::arya::LogMessage log), (const, override));

        MOCK_METHOD(bool,
                    setProperty,
                    (const std::string&, const std::string&, const std::string&),
                    (override));
        MOCK_METHOD(std::string, getProperty, (const std::string&), (const, override));
        MOCK_METHOD(std::string, getPropertyType, (const std::string&), (const, override));
        MOCK_METHOD(bool, isEqual, (const IProperties&), (const, override));
        MOCK_METHOD(void, copyTo, (IProperties&), (const, override));
        MOCK_METHOD(std::vector<std::string>, getPropertyNames, (), (const, override));
    };

    MOCK_METHOD(std::shared_ptr<ILogger>, createLogger, (const std::string&), (override));
    MOCK_METHOD(fep3::Result,
                registerSink,
                (const std::string&, const std::shared_ptr<ILoggingSink>&),
                (override));
    MOCK_METHOD(fep3::Result, unregisterSink, (const std::string&), (override));
};

} // namespace arya
using arya::LoggingService;
} // namespace mock
} // namespace fep3

///@endcond nodoc