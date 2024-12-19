/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

@endverbatim
 */

#include "tester_logging_sink_file.h"

#include <fep3/components/logging/mock/mock_logging_formater.h>
#include <fep3/native_components/logging/sinks/logging_sink_file_base.hpp>

#include <common/gtest_asserts.h>

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrictMock;

namespace fep3 {
namespace arya {
bool operator==(const LogMessage& lhs, const LogMessage& rhs)
{
    return ((lhs._logger_name == rhs._logger_name) && (lhs._message == rhs._message) &&
            (lhs._participant_name == rhs._participant_name) && (lhs._severity == rhs._severity) &&
            (lhs._timestamp == rhs._timestamp));
}
} // namespace arya
} // namespace fep3

struct TestLoggingSinkFileBase : public TestLoggingSinkFile {
protected:
    TestLoggingSinkFileBase()
    {
        std::unique_ptr<MockLoggingFormater> logging_formatter =
            std::make_unique<StrictMock<MockLoggingFormater>>();
        _mock_logging_formatter = logging_formatter.get();
        _logging_sink_file_base =
            std::make_unique<fep3::native::LoggingSinkFileBase>(std::move(logging_formatter));
    }

    void SetUp()
    {
        TestLoggingSinkFile::SetUp();
        std::ofstream log_file(_test_log_file_string);
    }

    std::unique_ptr<fep3::native::LoggingSinkFileBase> _logging_sink_file_base;
    MockLoggingFormater* _mock_logging_formatter;

    const std::string _init_string = "InitString";
    const std::string _log_message = "A log message";
};

/**
 * Test setting the file_path, the init string is written at the beggining of the file
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileBase, TestSetPath)
{
    EXPECT_CALL(*_mock_logging_formatter, IsStreamAppendable(_)).WillOnce(Return(true));
    EXPECT_CALL(*_mock_logging_formatter, GetStreamBegin()).WillOnce(Return(_init_string));
    EXPECT_CALL(*_mock_logging_formatter, StreamEnd(_)).Times(1);

    ASSERT_TRUE(_logging_sink_file_base->setProperty("file_path", _test_log_file_string, "string"));
    _logging_sink_file_base.reset();

    ASSERT_TRUE(readFileContent());
    ASSERT_NO_FATAL_FAILURE(findStringInFile(_init_string));
}

/**
 * Test logging a message
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileBase, TestLogMessage)
{
    fep3::arya::LogMessage logMessage = {
        "30.08.2021", fep3::arya::LoggerSeverity::debug, "participant", "logger", _log_message};

    EXPECT_CALL(*_mock_logging_formatter, IsStreamAppendable(_)).WillOnce(Return(true));
    EXPECT_CALL(*_mock_logging_formatter, GetStreamBegin()).WillOnce(Return(_init_string));
    EXPECT_CALL(*_mock_logging_formatter, formatLogMessage(Eq(logMessage)))
        .WillOnce(Return(_log_message));
    EXPECT_CALL(*_mock_logging_formatter, StreamEnd(_)).WillOnce(Invoke([](std::ostream& os) {
        os << "endString";
    }));

    ASSERT_TRUE(_logging_sink_file_base->setProperty("file_path", _test_log_file_string, "string"));
    ASSERT_FEP3_NOERROR(_logging_sink_file_base->log(logMessage));
    _logging_sink_file_base.reset();

    ASSERT_TRUE(readFileContent());
    ASSERT_NO_FATAL_FAILURE(findStringInFile(_init_string + _log_message))
        << "Logged string not found in log file";
    ASSERT_NO_FATAL_FAILURE(findStringInFile("endString")) << "Logged string not found in log file";
}

/**
 * Test logging a message in a non existing file
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileBase, TestLogFileNotExisting)
{
    ASSERT_NO_THROW(boost::filesystem::remove(_test_log_file_string));
    EXPECT_CALL(*_mock_logging_formatter, GetStreamBegin()).WillOnce(Return(_init_string));
    EXPECT_CALL(*_mock_logging_formatter, StreamEnd(_)).Times(1);

    ASSERT_TRUE(_logging_sink_file_base->setProperty("file_path", _test_log_file_string, "string"));
    ASSERT_EQ(_test_log_file_string, _logging_sink_file_base->getProperty("file_path"));
    _logging_sink_file_base.reset();

    ASSERT_TRUE(readFileContent());
    ASSERT_NO_FATAL_FAILURE(findStringInFile(_init_string));
}

/**
 * Test logging a message in a non appendable file
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileBase, TestFileNotAppendable)
{
    EXPECT_CALL(*_mock_logging_formatter, IsStreamAppendable(_)).WillOnce(Return(false));
    EXPECT_CALL(*_mock_logging_formatter, GetStreamBegin()).WillOnce(Return(_init_string));
    EXPECT_CALL(*_mock_logging_formatter, StreamEnd(_)).Times(1);

    ASSERT_FALSE(
        _logging_sink_file_base->setProperty("file_path", _test_log_file_string, "string"));
    ASSERT_EQ("", _logging_sink_file_base->getProperty("file_path"));
    _logging_sink_file_base.reset();
}

/**
 * Test creation and destruction of LoggingSinkFileBase when no functions are called
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileBase, TestFileNoOperation)
{
}

/**
 * Test logging without setting a file path before
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileBase, TestFileLogNoFilePath)
{
    fep3::arya::LogMessage logMessage = {
        "30.08.2021", fep3::arya::LoggerSeverity::debug, "participant", "logger", _log_message};
    EXPECT_CALL(*_mock_logging_formatter, formatLogMessage(Eq(logMessage)))
        .WillOnce(Return(_log_message));

    ASSERT_EQ(_logging_sink_file_base->log(logMessage), fep3::ERR_BAD_DEVICE);
}
