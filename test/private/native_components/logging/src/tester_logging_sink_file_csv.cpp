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

#include "tester_logging_sink_file.h"

#include <fep3/native_components/logging/sinks/logging_sink_file.hpp>

#include <common/gtest_asserts.h>

namespace {
const boost::filesystem::path test_json_logs_path =
    boost::filesystem::current_path() / "../files_csv";
} // namespace

using fep3::arya::LoggerSeverity;

struct TestLoggingSinkFileCsv : public TestLoggingSinkFile {
protected:
    TestLoggingSinkFileCsv()
        : _logging_sink_file_csv(std::make_unique<fep3::native::LoggingSinkFileCsv>())
    {
    }

    void checkCsvContent(unsigned char lines_to_skip = 0)
    {
        std::ifstream log_file(_test_log_file_string);
        ASSERT_TRUE(log_file.is_open());
        std::string line;

        for (int i = 0; i < lines_to_skip; i++) {
            getline(log_file, line);
        }

        getline(log_file, line);
        std::string expected_header = fep3::native::csv_log_header;
        // erase "\n" as it is discarded by getline()
        ASSERT_EQ(line,
                  expected_header.erase(expected_header.find("\n"), expected_header.length()));

        getline(log_file, line);
        const auto local_time_string{line.substr(0, line.find("\t"))};
        checkIsoTime(local_time_string);

        const auto log_without_local_time_string{line.substr(line.find("\t"), line.size())};
        ASSERT_EQ(log_without_local_time_string,
                  a_util::strings::format("\t%s\t%s\t%s\t%s\t%s",
                                          _timestamp.c_str(),
                                          _participant_name.c_str(),
                                          _logger_name.c_str(),
                                          getString(_logger_severity).c_str(),
                                          _log_message_string.c_str()));
        log_file.close();
    }

    std::unique_ptr<fep3::native::LoggingSinkFileCsv> _logging_sink_file_csv;
    const std::string _timestamp = "1681215840350";
    const std::string _logger_name = "logger";
    const std::string _participant_name = "participant";
    const LoggerSeverity _logger_severity{LoggerSeverity::debug};
    const std::string _log_message_string = "A log message";
    fep3::arya::LogMessage _log_message = {_timestamp,
                                           fep3::arya::LoggerSeverity::debug,
                                           _participant_name,
                                           _logger_name,
                                           _log_message_string};
};

/**
 * Test logging a csv message in a new file
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileCsv, TestCsvWriteNewFile)
{
    ASSERT_NO_THROW(
        _logging_sink_file_csv->setProperty("file_path", _test_log_file_string, "string"));

    ASSERT_FEP3_NOERROR(_logging_sink_file_csv->log(_log_message));
    _logging_sink_file_csv.reset();

    checkCsvContent();
}

/**
 * Test logging a csv message in an existing csv compatible file
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileCsv, TestCsvWriteExistingFile)
{
    for (int i = 0; i < 2; ++i) {
        ASSERT_NO_THROW(
            _logging_sink_file_csv->setProperty("file_path", _test_log_file_string, "string"));

        ASSERT_EQ(_logging_sink_file_csv->log(_log_message), a_util::result::SUCCESS);
        _logging_sink_file_csv = std::make_unique<fep3::native::LoggingSinkFileCsv>();
    }
    _logging_sink_file_csv.reset();

    checkCsvContent(3);
}

/**
 * Test logging a csv message in an existing non csv compatible file
 * Csv format is appended at the end
 * @req_id ???
 */
TEST_F(TestLoggingSinkFileCsv, TestCsvWriteCorruptFile)
{
    {
        std::ofstream log_file(_test_log_file_string);
        log_file << "{\"timestamp\": \"31.08.2021T16:29:01\",\"severity_level\": \"Debug\",";
        log_file.close();
    }

    ASSERT_NO_THROW(
        _logging_sink_file_csv->setProperty("file_path", _test_log_file_string, "string"));

    ASSERT_EQ(_logging_sink_file_csv->log(_log_message), a_util::result::SUCCESS);
    _logging_sink_file_csv.reset();

    checkCsvContent(1);
}
