/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/native_components/logging/logging_service.h>
#include <fep3/native_components/logging/sinks/logging_formater_fep.hpp>
#include <fep3/native_components/logging/sinks/logging_sink_rpc.hpp>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>
#include <fep3/rpc_services/logging/logging_client_stub.h>
#include <fep3/rpc_services/logging/logging_rpc_sink_client_service_stub.h>
#include <fep3/rpc_services/logging/logging_rpc_sink_service_client_stub.h>

#include <boost/thread/latch.hpp>

#include <notification_waiting.h>

// RPC Sink Client Client to send configurations to the logging service
typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingClientStub,
                                    fep3::rpc::IRPCLoggingServiceDef>
    LoggingServiceClient;
typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingRPCSinkServiceClientStub,
                                    fep3::rpc::IRPCLoggingSinkServiceDef>
    LoggingSinkServiceClient;

// RPC Sink Client Service to receive logs from the logging service
struct TestRPCSinkClient
    : public fep3::rpc::RPCService<fep3::rpc_stubs::RPCLoggingRPCSinkClientServiceStub,
                                   fep3::rpc::IRPCLoggingSinkClientDef> {
    TestRPCSinkClient(fep3::native::NotificationWaiting& notification) : _notification(notification)
    {
    }

    int onLog(const std::string& description,
              const std::string& logger_name,
              const std::string& participant,
              int severity,
              const std::string& timestamp) override
    {
        fep3::LogMessage log_message = {timestamp,
                                        static_cast<fep3::LoggerSeverity>(severity),
                                        participant,
                                        logger_name,
                                        description};
        const auto log_msg = _logging_formatter.formatLogMessage(log_message);
        std::cout << log_msg << std::endl;
        _messages.push_back(log_msg);
        if (_messages.size() >= 2)
            _notification.notify();
        return fep3::ERR_NOERROR.getCode();
    }
    std::vector<std::string> _messages;
    fep3::native::LoggingFormaterFep _logging_formatter;
    fep3::native::NotificationWaiting& _notification;
};

struct TestLoggingServiceRPC : public ::testing::Test {
    std::shared_ptr<fep3::native::LoggingService> _logging{
        std::make_shared<fep3::native::LoggingService>()};
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{
        std::make_shared<fep3::native::ServiceBus>()};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{
        std::make_shared<fep3::ComponentRegistry>()};
    std::unique_ptr<LoggingServiceClient> _logging_service_client;
    std::shared_ptr<TestRPCSinkClient> _test_sink_client;
    std::shared_ptr<LoggingSinkServiceClient> _sink_service;
    std::string _address;
    fep3::native::NotificationWaiting _notification{true};

private:
    const fep3::ComponentVersionInfo _dummy_component_version_info{
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};

public:
    TestLoggingServiceRPC()
    {
    }

    void SetUp()
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_EQ(_component_registry->registerComponent<fep3::IServiceBus>(
                      _service_bus, _dummy_component_version_info),
                  fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->registerComponent<fep3::ILoggingService>(
                      _logging, _dummy_component_version_info),
                  fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->create(), fep3::ERR_NOERROR);

        _logging_service_client = std::make_unique<LoggingServiceClient>(
            fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
            _service_bus->getRequester(fep3::native::testing::participant_name_default));

        auto rpc_server = _service_bus->getServer();
        if (rpc_server) {
            _test_sink_client = std::make_shared<TestRPCSinkClient>(_notification);
            ASSERT_EQ(
                rpc_server->registerService(
                    ::fep3::rpc::IRPCLoggingSinkClientDef::getRPCDefaultName(), _test_sink_client),
                fep3::ERR_NOERROR);
            _sink_service = std::make_shared<LoggingSinkServiceClient>(
                fep3::rpc::IRPCLoggingSinkServiceDef::getRPCDefaultName(),
                _service_bus->getRequester(fep3::native::testing::participant_name_default));
            _address = _service_bus->getServer()->getUrl();
        }
        else {
            ADD_FAILURE();
        }
    }

    void TearDown()
    {
        // The Service Bus object will be destroyed by the component registry during destruction
        ASSERT_EQ(_component_registry->destroy(), fep3::ERR_NOERROR);
    }
};

/**
 * Logs can be send to other participants via RPC and the receiver can also configure a filter for
 * received logs
 * @req_id ???
 */
TEST_F(TestLoggingServiceRPC, TestLoggingRPCSink)
{
    std::shared_ptr<fep3::ILogger> logger =
        _logging->createLogger("RPCLogger.LoggingService.Tester");
    try {
        _logging_service_client->setLoggerFilter(
            "rpc", "RPCLogger.LoggingService.Tester", static_cast<int>(fep3::LoggerSeverity::info));

        _sink_service->registerRPCLoggingSinkClient(_address,
                                                    "RPCLogger.LoggingService.Tester",
                                                    static_cast<int>(fep3::LoggerSeverity::info));
        // register an unreachable adrress
        _sink_service->registerRPCLoggingSinkClient("http://127.0.0.1:1111",
                                                    "RPCLogger.LoggingService.Tester",
                                                    static_cast<int>(fep3::LoggerSeverity::info));
    }
    catch (jsonrpc::JsonRpcException e) {
        ASSERT_TRUE(false) << e.what();
    }
    ASSERT_EQ(logger->logWarning("First message"), fep3::ERR_NOERROR);
    ASSERT_EQ(logger->logInfo("Second message"), fep3::ERR_NOERROR);
    // severity == debug should not appear because it's not configured
    ASSERT_EQ(logger->logDebug("Test log: must not appear at all"), fep3::ERR_NOERROR);

    // wait until the logs are executed from queue
    _notification.waitForNotification();

    ASSERT_EQ(_test_sink_client->_messages.size(), 2);

    const auto& ref_string1 = _test_sink_client->_messages[0];
    ASSERT_NE(ref_string1.find("RPCLogger.LoggingService.Tester"), std::string::npos);
    ASSERT_NE(ref_string1.find("Warning"), std::string::npos);
    ASSERT_NE(ref_string1.find("First message"), std::string::npos);

    const auto& ref_string2 = _test_sink_client->_messages[1];
    ASSERT_NE(ref_string2.find("RPCLogger.LoggingService.Tester"), std::string::npos);
    ASSERT_NE(ref_string2.find("Info"), std::string::npos);
    ASSERT_NE(ref_string2.find("Second message"), std::string::npos);

    _test_sink_client->_messages.clear();

    try {
        _sink_service->unregisterRPCLoggingSinkClient(_address);
    }
    catch (jsonrpc::JsonRpcException e) {
        ASSERT_TRUE(false) << e.what();
    }

    ASSERT_EQ(logger->logWarning("First message"), fep3::ERR_NOERROR);
    ASSERT_EQ(logger->logInfo("Second message"), fep3::ERR_NOERROR);
    // severity == debug should not appear because it's not configured
    ASSERT_EQ(logger->logDebug("Test log: must not appear at all"), fep3::ERR_NOERROR);

    // wait until the logs are executed from queue
    using namespace std::chrono_literals;
    _notification.waitForNotificationWithTimeout(1s);

    // it is still empty, because we unregistered
    ASSERT_EQ(_test_sink_client->_messages.size(), 0);
}
