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


#include <gtest/gtest.h>

#include "fep3/components/base/component_registry.h"
#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include "fep3/native_components/logging/logging_service.h"
#include "fep3/native_components/service_bus/service_bus.h"
#include "fep3/native_components/service_bus/testing/service_bus_testing.hpp"
#include "fep3/rpc_services/logging/logging_service_rpc_intf_def.h"
#include "fep3/rpc_services/logging/logging_client_stub.h"
#include "../test/private/utils/common/gtest_asserts.h"

#include <a_util/system/system.h>
#include "boost/filesystem.hpp"
#include <fstream>
#include <thread>
#include <future>

typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingClientStub, fep3::rpc::IRPCLoggingServiceDef> LoggingServiceClient;

namespace
{
    void writeLogEntry(std::shared_ptr<fep3::ILogger> iLogger, const std::string string_id)
    {
        for (int i = 0; i < 100; ++i)
        {
            ASSERT_FEP3_NOERROR(iLogger->logWarning(string_id + a_util::strings::toString(i)));
        }
    }
}

struct TestLoggingServiceFile : public ::testing::Test
{
    std::shared_ptr<fep3::native::LoggingService> _logging{ std::make_shared<fep3::native::LoggingService>() };
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{ std::make_shared<fep3::native::ServiceBus>() };
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{ std::make_shared<fep3::ComponentRegistry>() };
    std::unique_ptr<LoggingServiceClient> _logging_service_client;
    const std::string _rel_path = "../files";
    boost::filesystem::path _files_path;
    boost::filesystem::path _test_log_file;
    std::string _test_log_file_string;
    std::string _content;

    void SetUp()
    {
        ASSERT_NO_THROW(clean_up_before());
        set_up_file_paths();

        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_EQ(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->registerComponent<fep3::ILoggingService>(_logging), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->create(), fep3::ERR_NOERROR);

        _logging_service_client = std::make_unique<LoggingServiceClient>(fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
            _service_bus->getRequester(fep3::native::testing::participant_name_default));
    }

    void TearDown()
    {
        _component_registry->clear();
        _logging.reset();
        _service_bus.reset();
        _logging_service_client.reset();
        // clean up leftover files...
        ASSERT_NO_THROW(boost::filesystem::remove_all("../files")) << " Log file folder files still in use";
    }

    void clean_up_before()
    {
        // clean up in case there are leftovers from previous tests.
        if (boost::filesystem::exists("../files"))
        {
            boost::filesystem::remove_all("../files");
        }
        boost::filesystem::create_directory("../files");
    }

    void set_up_file_paths()
    {
        _files_path = boost::filesystem::canonical("../files");
        _test_log_file = _files_path;
        _test_log_file.append("some_logfile.txt");
        _test_log_file_string = _test_log_file.string();
    }

    testing::AssertionResult readFileContent()
    {
        if (boost::filesystem::exists(_test_log_file_string))
        {
            std::ifstream ifs(_test_log_file_string);
            _content = std::string(std::istreambuf_iterator<char>{ifs}, {});
            return testing::AssertionSuccess();
        }
        else
        {
            return testing::AssertionFailure() << "Log file " << _test_log_file_string << " not found";
        }
    }

    void findStringInFile(const std::string& expected_string)
    {
        ASSERT_NE(_content.find(expected_string), std::string::npos) << "expected string " << expected_string
            << " not found in the log file";
    }

    void stringNotInFile(const std::string& unexpected_string)
    {
        ASSERT_EQ(_content.find(unexpected_string), std::string::npos) << "unexpected string " << unexpected_string
            << " found in the log file";
    }
};

/**
* The file logger must create a file if it doesn't exist during configuration and write the correct messages into it during logging
* @req_id ???
*/
TEST_F(TestLoggingServiceFile, TestFileLog)
{
    std::shared_ptr<fep3::ILogger> logger = _logging->createLogger("FileLogger.LoggingService.Tester");
    ASSERT_TRUE(logger);

    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter(
        "file",
        "FileLogger.LoggingService.Tester",
        static_cast<int>(fep3::LoggerSeverity::warning)));
    ASSERT_NO_THROW(_logging_service_client->setSinkProperty(
        "file_path",
        "file",
        "string",
        _test_log_file_string));

    ASSERT_EQ(logger->logError("First message"), fep3::ERR_NOERROR);
    ASSERT_EQ(logger->logWarning("Second message"), fep3::ERR_NOERROR);
    // severity == info should not appear because it's not configured
    ASSERT_EQ(logger->logInfo("Test log: must not appear in file"), fep3::ERR_NOERROR);

    // wait until the logs are executed from queue
    a_util::system::sleepMilliseconds(300);

    // validate file content
    ASSERT_TRUE(readFileContent());

    ASSERT_NO_FATAL_FAILURE(findStringInFile("FileLogger.LoggingService.Tester"));
    ASSERT_NO_FATAL_FAILURE(findStringInFile("Error"));
    ASSERT_NO_FATAL_FAILURE(findStringInFile("First message"));
    ASSERT_NO_FATAL_FAILURE(findStringInFile("Warning"));
    ASSERT_NO_FATAL_FAILURE(findStringInFile("Second message"));

    ASSERT_NO_FATAL_FAILURE(stringNotInFile("Info"));
    ASSERT_NO_FATAL_FAILURE(stringNotInFile("must not appear in file"));
}

/**
* Two or more loggers using the same log file must not interfere with each other, but not loose any logs either
* @req_id ???
*/
TEST_F(TestLoggingServiceFile, TestFileStress)
{
    std::shared_ptr<fep3::ILogger> logger_first = _logging->createLogger("FileLogger.LoggingService.Tester");
    std::shared_ptr<fep3::ILogger> logger_second = _logging->createLogger("FileLogger.LoggingService.Tester");
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter(
        "file",
        "FileLogger.LoggingService.Tester",
        static_cast<int>(fep3::LoggerSeverity::warning)));
    ASSERT_NO_THROW(_logging_service_client->setSinkProperty(
        "file_path",
        "file",
        "string",
        _test_log_file_string));

    std::future<void> thread1_future = std::async(std::launch::async, [logger_first]() {return writeLogEntry(logger_first, "First:  "); });
    std::future<void> thread2_future = std::async(std::launch::async, [logger_second]() {return writeLogEntry(logger_second, "Second:  "); });

    // make sure all threads have returned
    thread1_future.get();
    thread2_future.get();
    // wait until the logs are executed from queue, has to be fixed in the future
     a_util::system::sleepMilliseconds(1000);
    // validate file content
    ASSERT_TRUE(readFileContent());

    ASSERT_NO_FATAL_FAILURE(findStringInFile("FileLogger.LoggingService.Tester"));
    ASSERT_NO_FATAL_FAILURE(findStringInFile("Warning"));

    for (int i = 0; i < 100; ++i)
    {
        ASSERT_NO_FATAL_FAILURE(findStringInFile("First:  " + a_util::strings::toString(i)));
        ASSERT_NO_FATAL_FAILURE(findStringInFile("Second:  " + a_util::strings::toString(i)));
    }
}
