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

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <gtest/gtest.h>

#include <fstream>

struct TestLoggingSinkFile : public ::testing::Test {
    const std::string _log_path = TEST_LOGFILES_DIR;
    boost::filesystem::path _files_path;
    boost::filesystem::path _test_log_file;
    std::string _test_log_file_string;
    std::string _content;

    void SetUp()
    {
        ASSERT_NO_THROW(cleanUpBefore());
        setUpFilePaths();
    }

    void TearDown()
    {
        // clean up leftover files...
        ASSERT_NO_THROW(boost::filesystem::remove_all(_log_path))
            << " Log file folder files still in use";
    }

    void cleanUpBefore()
    {
        // clean up in case there are leftovers from previous tests.
        if (boost::filesystem::exists(_log_path)) {
            boost::filesystem::remove_all(_log_path);
        }
        boost::filesystem::create_directory(_log_path);
    }

    void setUpFilePaths()
    {
        _files_path = boost::filesystem::canonical(_log_path);
        _test_log_file = _files_path;
        _test_log_file.append("some_logfile.txt");
        _test_log_file_string = _test_log_file.string();
    }

    testing::AssertionResult readFileContent()
    {
        if (boost::filesystem::exists(_test_log_file_string)) {
            std::ifstream ifs(_test_log_file_string);
            _content = std::string(std::istreambuf_iterator<char>{ifs}, {});
            return testing::AssertionSuccess();
        }
        else {
            return testing::AssertionFailure()
                   << "Log file " << _test_log_file_string << " not found";
        }
    }

    void stringNotInFile(const std::string& unexpected_string)
    {
        ASSERT_EQ(_content.find(unexpected_string), std::string::npos)
            << "unexpected string " << unexpected_string << " found in the log file";
    }

    void findStringInFile(const std::string& expected_string)
    {
        ASSERT_NE(_content.find(expected_string), std::string::npos)
            << "expected string " << expected_string << " not found in the log file";
    }

    void checkIsoTime(const std::string& time_string)
    {
        boost::regex timestamp_iso8601_regex{
            "(\\d{4})-(\\d{2})-(\\d{2})T(\\d*):(\\d*):(\\d*),(\\d*)[\\+-](\\d{2})"};
        boost::smatch what;
        std::cout << time_string << std::endl;
        ASSERT_TRUE(boost::regex_search(time_string, what, timestamp_iso8601_regex))
            << "Json timestamp does not comply to ISO8601 time format";
    }
};
