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

#include <fep3/fep3_participant_version.h>
#include "fep3/components/base/component_registry.h"
#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include "fep3/native_components/logging/logging_service.h"
#include "fep3/native_components/service_bus/service_bus.h"
#include "fep3/native_components/service_bus/testing/service_bus_testing.hpp"
#include "fep3/rpc_services/logging/logging_service_rpc_intf_def.h"
#include "fep3/rpc_services/logging/logging_client_stub.h"
#include "fep3/native_components/logging/sinks/logging_sink_common.hpp"
#include "fep3/components/clock/mock/mock_clock_service.h"

#include <a_util/system/system.h>
#include <chrono>

typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingClientStub, fep3::rpc::IRPCLoggingServiceDef> LoggingServiceClient;

struct TestLoggingServiceConsole : public ::testing::Test
{
    std::shared_ptr<fep3::native::LoggingService> _logging{ std::make_shared<fep3::native::LoggingService>() };
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{ std::make_shared<fep3::native::ServiceBus>() };
    std::shared_ptr<fep3::mock::ClockService<>> _clock_service{ std::make_shared < fep3::mock::ClockService<> >() };
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{ std::make_shared<fep3::ComponentRegistry>() };
    std::unique_ptr<LoggingServiceClient> _logging_service_client;
private:
    const fep3::ComponentVersionInfo _dummy_component_version_info{FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};

public:
    TestLoggingServiceConsole()
    {
    }

    void SetUp()
    {
        EXPECT_CALL(*_clock_service.get(), getTime())
            .WillRepeatedly(testing::Return(std::chrono::nanoseconds(0)));
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_EQ(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus, _dummy_component_version_info), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->registerComponent<fep3::ILoggingService>(_logging, _dummy_component_version_info), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->registerComponent<fep3::IClockService>(_clock_service, _dummy_component_version_info), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->create(), fep3::ERR_NOERROR);

        _logging_service_client = std::make_unique<LoggingServiceClient>(fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
            _service_bus->getRequester(fep3::native::testing::participant_name_default));
    }

    void TearDown()
    {
        // The Service Bus object will be destroyed by the component registry during destruction

    }
};

/**
* Error and Fatal logs must appear in stderr and not stdout
* @req_id ???
*/
TEST_F(TestLoggingServiceConsole, TestConsoleLogErr)
{
    std::shared_ptr<fep3::ILogger> logger = _logging->createLogger("ConsoleErrorLogger.LoggingService.Tester");
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("console",
        "ConsoleErrorLogger.LoggingService.Tester",
        static_cast<int>(fep3::LoggerSeverity::warning)));

    // gtest is AWESOME!
    testing::internal::CaptureStderr();

    ASSERT_EQ(logger->logError("First message"), fep3::ERR_NOERROR);
    ASSERT_EQ(logger->logFatal("Second message"), fep3::ERR_NOERROR);
    // severity != fatal or error should not appear in stderr
    ASSERT_EQ(logger->logWarning("Test log: must not appear in stderr"), fep3::ERR_NOERROR);

    // wait until the logs are executed from queue
    a_util::system::sleepMilliseconds(100);

    // validate console content
    std::string strng = testing::internal::GetCapturedStderr();

    //ASSERT_TRUE(strng.find("TestClient") != std::string::npos);
    ASSERT_NE(strng.find("ConsoleErrorLogger.LoggingService.Tester"), std::string::npos);
    ASSERT_NE(strng.find("Error"), std::string::npos);
    ASSERT_NE(strng.find("First message"), std::string::npos);
    ASSERT_NE(strng.find("Fatal"), std::string::npos);
    ASSERT_NE(strng.find("Second message"), std::string::npos);

    ASSERT_EQ(strng.find("Warning"), std::string::npos);
    ASSERT_EQ(strng.find("must not appear in stderr"), std::string::npos);
}

/**
* Warning, Info and Debug must appear in stdout, but only if they are configured
* @req_id ???
*/
TEST_F(TestLoggingServiceConsole, TestConsoleLogStd)
{
    std::shared_ptr<fep3::ILogger> logger = _logging->createLogger("ConsoleLogger.LoggingService.Tester");
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("console",
        "ConsoleLogger.LoggingService.Tester",
        static_cast<int>(fep3::LoggerSeverity::info))
    );

    // gtest is AWESOME!
    testing::internal::CaptureStdout();

    ASSERT_EQ(logger->logWarning("First message"), fep3::ERR_NOERROR);
    ASSERT_EQ(logger->logInfo("Second message"), fep3::ERR_NOERROR);
    // severity == error or fatal should not appear in stdout, but stderr.
    ASSERT_EQ(logger->logError("Test log: must not appear in stdout"), fep3::ERR_NOERROR);
    // severity == debug should not appear because it's not configured
    ASSERT_EQ(logger->logDebug("Test log: must not appear at all"), fep3::ERR_NOERROR);

    // wait until the logs are executed from queue
    a_util::system::sleepMilliseconds(100);

    // validate console content
    std::string strng = testing::internal::GetCapturedStdout();

    //ASSERT_TRUE(strng.find("TestClient") != std::string::npos);
    ASSERT_NE(strng.find("ConsoleLogger.LoggingService.Tester"), std::string::npos);
    ASSERT_NE(strng.find("Warning"), std::string::npos);
    ASSERT_NE(strng.find("First message"), std::string::npos);
    ASSERT_NE(strng.find("Info"), std::string::npos);
    ASSERT_NE(strng.find("Second message"), std::string::npos);

    ASSERT_EQ(strng.find("Error"), std::string::npos);
    ASSERT_EQ(strng.find("must not appear in stdout"), std::string::npos);
    ASSERT_EQ(strng.find("Debug"), std::string::npos);
    ASSERT_EQ(strng.find("must not appear at all"), std::string::npos);
}

/**
* Clock timestamp should correctly be converted and labeled
* @req_id ???
*/
TEST_F(TestLoggingServiceConsole, TestTimestampIsNS)
{
    std::shared_ptr<fep3::ILogger> logger = _logging->createLogger("ConsoleLogger.LoggingService.Tester");

    EXPECT_CALL(*_clock_service.get(), getTime())
        .WillRepeatedly(testing::Return(std::chrono::nanoseconds(12345)));

    testing::internal::CaptureStdout();

    ASSERT_EQ(logger->logWarning("Some message"), fep3::ERR_NOERROR);

    // wait until the logs are executed from queue
    a_util::system::sleepMilliseconds(100);

    std::string strng = testing::internal::GetCapturedStdout();
    ASSERT_NE(strng.find("12345[ns]"), std::string::npos);
}
