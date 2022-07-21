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

#include "fep3/native_components/logging/sinks/logging_sink_file.hpp"
#include "tester_logging_sink_folder_setup.h"
#include "common/gtest_asserts.h"
#include <json/json.h>

namespace
{
    const boost::filesystem::path test_json_logs_path = boost::filesystem::current_path() / "../files_json";
}

struct TestLoggingSinkFileJson : public TestLoggingSinkFolderSetup
{
protected:
    TestLoggingSinkFileJson()
        : _logging_sink_file_json(std::make_unique<fep3::LoggingSinkFileJson>())
    {
    }

    void checkJsonElement(const Json::Value& root, unsigned char element_index)
    {
        ASSERT_EQ(root[element_index]["severity_level"].asString(), "Debug");
        ASSERT_EQ(root[element_index]["logger_name"].asString(), _logger_name);
        ASSERT_EQ(root[element_index]["message"].asString(), _log_message_string);
        ASSERT_EQ(root[element_index]["participant_name"].asString(), _participant_name);
        ASSERT_EQ(root[element_index]["log_type"].asString(), "message");
    }

   void readJsonFile(unsigned char element_count)
    {
        Json::Value root;
        std::ifstream t(_test_log_file_string);
        ASSERT_NO_THROW(t >> root);

        for (unsigned char i = 0; i < element_count; ++i)
        {
            ASSERT_NO_FATAL_FAILURE(checkJsonElement(root, i));
        }
    }

    std::unique_ptr<fep3::LoggingSinkFileJson> _logging_sink_file_json;
    const std::string _timestamp = "30.08.2021";
    const std::string _logger_name = "logger";
    const std::string _participant_name = "participant";
    const std::string _log_message_string = "A log message";
    fep3::arya::LogMessage _log_message = { _timestamp, fep3::arya::LoggerSeverity::debug, _participant_name, _logger_name,_log_message_string };
};

/**
* Test logging a json message in a new file
* @req_id ???
*/
TEST_F(TestLoggingSinkFileJson, TestJsonWriteNewFile)
{
    ASSERT_NO_THROW(_logging_sink_file_json->setProperty(
        "file_path",
        _test_log_file_string,
        "string"));

    ASSERT_FEP3_NOERROR(_logging_sink_file_json->log(_log_message));
    _logging_sink_file_json.reset();

    ASSERT_NO_FATAL_FAILURE(readJsonFile(1)) << "Reading json log file failed";
}

/**
* Test logging a json message in an existing json compatible file
* @req_id ???
*/
TEST_F(TestLoggingSinkFileJson, TestJsonWriteExistingFile)
{
    for (int i = 0; i < 2; ++i)
    {
        ASSERT_NO_THROW(_logging_sink_file_json->setProperty(
            "file_path",
            _test_log_file_string,
            "string"));

        ASSERT_EQ(_logging_sink_file_json->log(_log_message), a_util::result::SUCCESS);
        _logging_sink_file_json = std::make_unique<fep3::LoggingSinkFileJson>();
    }
    _logging_sink_file_json.reset();
    ASSERT_NO_THROW(readJsonFile(2)) << "Reading json log file failed";
}

/**
* Test logging a json message in an existing non json compatible file
* Json format is appended at the end
* @req_id ???
*/
TEST_F(TestLoggingSinkFileJson, TestJsonWriteCoruptFile)
{
    {
        std::ofstream log_file(_test_log_file_string);
        log_file << "{\"timestamp\": \"31.08.2021T16:29:01\",\"severity_level\": \"Debug\",";
    }

    ASSERT_FALSE(_logging_sink_file_json->setProperty(
        "file_path",
        _test_log_file_string,
        "string"));

    ASSERT_EQ(_logging_sink_file_json->log(_log_message), a_util::result::SUCCESS);
    _logging_sink_file_json.reset();

    ASSERT_TRUE(readFileContent());
    ASSERT_NO_FATAL_FAILURE(findStringInFile("Log file is corrupt, appending will result in invalid logging format"));

    std::ifstream fileStream(_test_log_file_string);
    std::string logLine;
    // skip the first two lines that are not json format
    std::getline(fileStream, logLine);
    std::getline(fileStream, logLine);
    Json::Value root;
    fileStream >> root;
    ASSERT_NO_FATAL_FAILURE(checkJsonElement(root, 0));
}
