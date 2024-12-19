/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */
#pragma once

#include <fep3/native_components/logging/sinks/logging_formater_intf.hpp>

#include <gmock/gmock.h>

class MockLoggingFormater : public fep3::native::ILoggingFormater {
public:
    MOCK_METHOD(std::string,
                formatLogMessage,
                (const fep3::arya::LogMessage& log),
                (const, override));
    MOCK_METHOD(std::string, GetStreamBegin, (), (const, override));
    MOCK_METHOD(void, StreamEnd, (std::ostream&), (const, override));
    MOCK_METHOD(bool, IsStreamAppendable, (std::iostream&), (override));
};
