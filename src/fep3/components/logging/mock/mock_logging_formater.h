
#pragma once


#include "fep3/native_components/logging/sinks/logging_formater_intf.hpp"
#include <gmock/gmock.h>

class MockLoggingFormater : public fep3::ILoggingFormater {

public:
    MOCK_METHOD(std::string, formatLogMessage, (const fep3::arya::LogMessage& log), (const, override));
    MOCK_METHOD(std::string, GetStreamBegin, (), (const, override));
    MOCK_METHOD(void, StreamEnd, (std::ostream&), (const, override));
    MOCK_METHOD(bool, IsStreamAppendable, (std::iostream&), (override));
};
