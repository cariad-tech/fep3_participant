/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../test/private/utils/common/gtest_asserts.h"
#include "tester_logging_sink_file.h"

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/native_components/logging/logging_service.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>
#include <fep3/rpc_services/logging/logging_client_stub.h>
#include <fep3/rpc_services/logging/logging_rpc_sink_service_service_stub.h>

#include <boost/regex.hpp>

#include <future>
#include <thread>

typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingClientStub,
                                    fep3::rpc::IRPCLoggingServiceDef>
    LoggingServiceClient;

namespace {
void writeLogEntry(std::shared_ptr<fep3::ILogger> iLogger, const std::string string_id)
{
    for (int i = 0; i < 100; ++i) {
        ASSERT_FEP3_NOERROR(iLogger->logWarning(string_id + std::to_string(i)));
    }
}
} // namespace

struct TestLoggingServiceFile : public TestLoggingSinkFile {
    std::shared_ptr<fep3::native::LoggingService> _logging{
        std::make_shared<fep3::native::LoggingService>()};
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{
        std::make_shared<fep3::native::ServiceBus>()};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{
        std::make_shared<fep3::ComponentRegistry>()};
    std::unique_ptr<LoggingServiceClient> _logging_service_client;
    fep3::ComponentVersionInfo dummy_component_version_info{"3.0.1", "dummyPath", "3.1.0"};

    void SetUp()
    {
        ASSERT_NO_FATAL_FAILURE(TestLoggingSinkFile::SetUp());

        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_EQ(_component_registry->registerComponent<fep3::IServiceBus>(
                      _service_bus, dummy_component_version_info),
                  fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->registerComponent<fep3::ILoggingService>(
                      _logging, dummy_component_version_info),
                  fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->create(), fep3::ERR_NOERROR);

        _logging_service_client = std::make_unique<LoggingServiceClient>(
            fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
            _service_bus->getRequester(fep3::native::testing::participant_name_default));
    }

    void TearDown()
    {
        ASSERT_EQ(_component_registry->destroy(), fep3::ERR_NOERROR);
        _component_registry->clear();
        _logging.reset();
        _service_bus.reset();
        _logging_service_client.reset();

        // clean up leftover files...
        ASSERT_NO_FATAL_FAILURE(TestLoggingSinkFile::TearDown());
    }

    void readJsonFile(unsigned char index, const std::string& severity, const std::string& message)
    {
        Json::Value root;
        std::ifstream t(_test_log_file_string);
        ASSERT_NO_THROW(t >> root);

        ASSERT_EQ(root[index]["severity_level"].asString(), severity);
        ASSERT_EQ(root[index]["logger_name"].asString(), "FileLogger.LoggingService.Tester");
        ASSERT_EQ(root[index]["message"].asString(), message);
        ASSERT_EQ(root[index]["participant_name"].asString(),
                  fep3::native::testing::participant_name_default);
    }
};

/**
 * The file logger must create a file if it doesn't exist during configuration and write the correct
 * messages into it during logging
 * @req_id ???
 */
TEST_F(TestLoggingServiceFile, TestFileLog)
{
    std::shared_ptr<fep3::ILogger> logger =
        _logging->createLogger("FileLogger.LoggingService.Tester");
    ASSERT_TRUE(logger);

    ASSERT_NO_THROW(
        _logging_service_client->setLoggerFilter("file,console",
                                                 "FileLogger.LoggingService.Tester",
                                                 static_cast<int>(fep3::LoggerSeverity::warning)));
    ASSERT_NO_THROW(_logging_service_client->setSinkProperty(
        "file_path", "file", "string", _test_log_file_string));

    ASSERT_EQ(logger->logError("First message"), fep3::ERR_NOERROR);
    ASSERT_EQ(logger->logWarning("Second message"), fep3::ERR_NOERROR);
    // severity == info should not appear because it's not configured
    ASSERT_EQ(logger->logInfo("Test log: must not appear in file"), fep3::ERR_NOERROR);

    // wait until the logs are executed from queue
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

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
 * Two or more loggers using the same log file must not interfere with each other, but not loose any
 * logs either
 * @req_id ???
 */
TEST_F(TestLoggingServiceFile, TestFileStress)
{
    std::shared_ptr<fep3::ILogger> logger_first =
        _logging->createLogger("FileLogger.LoggingService.Tester");
    std::shared_ptr<fep3::ILogger> logger_second =
        _logging->createLogger("FileLogger.LoggingService.Tester");
    ASSERT_NO_THROW(
        _logging_service_client->setLoggerFilter("file",
                                                 "FileLogger.LoggingService.Tester",
                                                 static_cast<int>(fep3::LoggerSeverity::warning)));
    ASSERT_NO_THROW(_logging_service_client->setSinkProperty(
        "file_path", "file", "string", _test_log_file_string));

    std::future<void> thread1_future = std::async(
        std::launch::async, [logger_first]() { return writeLogEntry(logger_first, "First:  "); });
    std::future<void> thread2_future = std::async(std::launch::async, [logger_second]() {
        return writeLogEntry(logger_second, "Second:  ");
    });

    // make sure all threads have returned
    thread1_future.get();
    thread2_future.get();
    // wait until the logs are executed from queue, has to be fixed in the future
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // validate file content
    ASSERT_TRUE(readFileContent());

    ASSERT_NO_FATAL_FAILURE(findStringInFile("FileLogger.LoggingService.Tester"));
    ASSERT_NO_FATAL_FAILURE(findStringInFile("Warning"));

    for (int i = 0; i < 100; ++i) {
        ASSERT_NO_FATAL_FAILURE(findStringInFile("First:  " + std::to_string(i)));
        ASSERT_NO_FATAL_FAILURE(findStringInFile("Second:  " + std::to_string(i)));
    }
}

/**
 *The file logger should be able to write in json format
 */
TEST_F(TestLoggingServiceFile, TestFileLogJson)
{
    std::shared_ptr<fep3::ILogger> logger =
        _logging->createLogger("FileLogger.LoggingService.Tester");
    ASSERT_TRUE(logger);

    ASSERT_NO_THROW(
        _logging_service_client->setLoggerFilter("file_json",
                                                 "FileLogger.LoggingService.Tester",
                                                 static_cast<int>(fep3::LoggerSeverity::warning)));
    ASSERT_NO_THROW(_logging_service_client->setSinkProperty(
        "file_path", "file_json", "string", _test_log_file_string));

    ASSERT_FEP3_NOERROR(logger->logError("First message"));
    ASSERT_FEP3_NOERROR(logger->logWarning("Second message"));
    // severity == info should not appear because it's not configured
    ASSERT_FEP3_NOERROR(logger->logInfo("Test log: must not appear in file"));

    // wait until the logs are executed from queue
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // clean everything so that the destructor of LoggingFormaterJson is called
    ASSERT_EQ(_component_registry->destroy(), fep3::ERR_NOERROR);
    _component_registry->clear();
    _logging.reset();
    _service_bus.reset();
    _logging_service_client.reset();

    // validate file content
    ASSERT_NO_FATAL_FAILURE(readJsonFile(0, "Error", "First message"));
    ASSERT_NO_FATAL_FAILURE(readJsonFile(1, "Warning", "Second message"));
}
