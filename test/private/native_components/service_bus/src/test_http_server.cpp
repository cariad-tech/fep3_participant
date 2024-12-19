/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tester_service_bus_native_and_base_mocks.h"

#if defined(LSSDP_SERVICE_DISCOVERY)
    #include <service_discovery_factory_lssdp.h>
#elif defined(DDS_SERVICE_DISCOVERY)
    #include <service_discovery_factory_dds.h>
#else
    #error No Service Discovery implementation defined
#endif

#include <fep3/components/logging/mock_logger.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>
#include <fep3/rpc_services/participant_info/participant_info_client_stub.h>

#include <boost/thread/barrier.hpp>

#include <future>
#include <gmock_async_helper.h>
#include <helper/notification_latch.h>
#include <http_server.h>
#include <thread_pool/fake_thread_pool_executor.h>

using namespace ::testing;
using namespace fep3;

class TestClient : public fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCParticipantInfoClientStub,
                                                      fep3::rpc::arya::IRPCParticipantInfoDef> {
private:
    typedef RPCServiceClient<fep3::rpc_stubs::RPCParticipantInfoClientStub,
                             fep3::rpc::arya::IRPCParticipantInfoDef>
        base_type;

public:
    using base_type::GetStub;

    TestClient(const std::string& server_object_name,
               const std::shared_ptr<fep3::IRPCRequester>& rpc_requester)
        : base_type(server_object_name, rpc_requester)
    {
    }
};

class TestLogger {
public:
    fep3::Result log(const std::string& message) const
    {
        // wait for the call from rpc request
        if (message.find("HttpServer will lock wrapper for getRegisteredServiceNames") !=
            std::string::npos) {
            _promise.set_value();
            // wait until the HttpServer desstructor is called and is about to call
            // cThreadedHttpServer::StopListening
            while (!_stop_listening_called) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        // HttpServer destructor is about to call the stop listening
        if (message.find("HttpServer StopListening") != std::string::npos) {
            _stop_listening_called = true;
        }

        return {};
    }

    mutable std::promise<void> _promise;
    mutable std::atomic<bool> _stop_listening_called = false;
};

// see FEPSDK-3427
TEST(TestJsonRpcServer, testDeadlock)
{
    auto logger = std::make_shared<NiceMock<fep3::mock::Logger>>();
    TestLogger test_logger;

    auto promise_f = test_logger._promise.get_future();
    EXPECT_CALL(*logger, isDebugEnabled()).WillRepeatedly(Return(true));
    EXPECT_CALL(*logger, logDebug(_)).WillRepeatedly(Invoke(&test_logger, &TestLogger::log));

    std::shared_ptr<native::ServiceBus> _service_bus{
        std::make_shared<fep3::native::ServiceBus>(logger)};

    ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
    auto _client = std::make_unique<TestClient>(
        fep3::rpc::arya::IRPCParticipantInfoDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::participant_name_default));

    auto async_fut = std::async([&]() { _client->getRPCServices(); });
    // first make sure the RPC call is made, then it waits for the destructor
    promise_f.get();
    // call the destructor
    _service_bus.reset();
    // the _http_server.StopListening(); should wait any active RPC requests
    async_fut.get();
}

TEST(tesHttpServerConstruction, testNonDiscoverableValidSysUrl)
{
    const std::string valid_sys_url = "url";
    const bool discoverable_flag = false;
    auto _service_discovery_factory = std::make_shared<NiceMock<ServiceDiscoveryFactoryMock>>();

    EXPECT_CALL(*_service_discovery_factory, getServiceDiscovery(_, _, _, _, _, _, _, _, _, _))
        .Times(0);

    auto server = std::make_unique<fep3::native::HttpServer>("s1",
                                                             "http://0.0.0.0:0",
                                                             "sys",
                                                             valid_sys_url,
                                                             nullptr,
                                                             _service_discovery_factory,
                                                             discoverable_flag);
}

TEST(tesHttpServerConstruction, testNonDiscoverableEmptySysUrl)
{
    const std::string empty_sys_url = "";
    const bool discoverable_flag = false;
    auto _service_discovery_factory = std::make_shared<NiceMock<ServiceDiscoveryFactoryMock>>();

    EXPECT_CALL(*_service_discovery_factory, getServiceDiscovery(_, _, _, _, _, _, _, _, _, _))
        .Times(0);

    auto server = std::make_unique<fep3::native::HttpServer>("s1",
                                                             "http://0.0.0.0:0",
                                                             "sys",
                                                             empty_sys_url,
                                                             nullptr,
                                                             _service_discovery_factory,
                                                             discoverable_flag);
}

TEST(tesHttpServerConstruction, testDiscoverableValidSysUrl)
{
    const std::string valid_sys_url = "url";
    const bool discoverable_flag = true;
    auto _service_discovery_factory = std::make_shared<NiceMock<ServiceDiscoveryFactoryMock>>();

    EXPECT_CALL(*_service_discovery_factory,
                getServiceDiscovery("url", _, _, _, "s1@sys", _, _, _, _, _))
        .WillOnce(Return(ByMove(nullptr)));

    auto server = std::make_unique<fep3::native::HttpServer>("s1",
                                                             "http://0.0.0.0:0",
                                                             "sys",
                                                             valid_sys_url,
                                                             nullptr,
                                                             _service_discovery_factory,
                                                             discoverable_flag);
}

TEST(tesHttpServerConstruction, testDiscoverableEmptySysUrl)
{
    const std::string empty_sys_url = "";
    const bool discoverable_flag = true;
    auto _service_discovery_factory = std::make_shared<NiceMock<ServiceDiscoveryFactoryMock>>();

    EXPECT_CALL(*_service_discovery_factory, getServiceDiscovery(_, _, _, _, _, _, _, _, _, _))
        .Times(0);

    ASSERT_THROW(auto server =
                     std::make_unique<fep3::native::HttpServer>("s1",
                                                                "http://0.0.0.0:0",
                                                                "sys",
                                                                empty_sys_url,
                                                                nullptr,
                                                                _service_discovery_factory,
                                                                discoverable_flag),
                 std::runtime_error);
}

TEST(tesHttpServerConstruction, testLogging)
{
    const std::string valid_sys_url = "url";
    const bool discoverable_flag = true;
    auto _service_discovery_factory = std::make_shared<NiceMock<ServiceDiscoveryFactoryMock>>();
    auto service_discovery = std::make_unique<NiceMock<ServiceDiscoveryMock>>();
    auto _logger_mock = std::make_shared<NiceMock<fep3::mock::Logger>>();
    test::helper::Notification notification;

    ON_CALL(*service_discovery.get(), sendNotifyAlive()).WillByDefault(Return(true));
    ON_CALL(*service_discovery.get(), checkForMSearchAndSendResponse(_))
        .WillByDefault(Return(true));
    ON_CALL(*_service_discovery_factory,
            getServiceDiscovery("url", _, _, _, "s1@sys", _, _, _, _, _))
        .WillByDefault(Return(ByMove(std::move(service_discovery))));
    ON_CALL(*_logger_mock.get(), isDebugEnabled()).WillByDefault(Return(true));

    EXPECT_CALL(*_logger_mock, logDebug(HasSubstr("HttpServer StopListening")))
        .WillOnce(Return(ERR_NOERROR));
    EXPECT_CALL(*_logger_mock, logDebug(HasSubstr("HttpServer StartListening on")))
        .WillOnce(Return(ERR_NOERROR));
    EXPECT_CALL(*_logger_mock,
                logDebug(HasSubstr("HttpServer stopping discovery and heartbeat threads")))
        .WillOnce(Return(ERR_NOERROR));

    EXPECT_CALL(*_logger_mock,
                logDebug(HasSubstr(
                    "HttpServer creating system discovery and heartbeat threads for system")))
        .WillOnce(Return(ERR_NOERROR));
    EXPECT_CALL(*_logger_mock,
                logDebug(HasSubstr("HttpServer starting discovery and heartbeat threads")))
        .WillOnce(Return(ERR_NOERROR));
    EXPECT_CALL(*_logger_mock, logDebug(HasSubstr("HttpServer sending notify alive")))
        .WillOnce(DoAll(InvokeWithoutArgs([&]() { notification.notify(); }), Return(ERR_NOERROR)));

    auto server = std::make_unique<fep3::native::HttpServer>("s1",
                                                             "http://0.0.0.0:0",
                                                             "sys",
                                                             valid_sys_url,
                                                             _logger_mock,
                                                             _service_discovery_factory,
                                                             discoverable_flag);

    notification.waitForNotification();
}

/**
 * @brief Test the port usage of http server
 */
TEST(ServiceBusServer, testHttpPorts)
{
    std::vector<std::thread> my_threads;
    std::set<std::string> urls;
    std::mutex mtx_set;
    const int number_of_servers = 100;
    boost::barrier bar(number_of_servers + 1);
    std::shared_ptr<fep3::native::IServiceDiscoveryFactory> service_discovery_factory =
        std::make_shared<fep3::native::ServiceDiscoveryFactory>();

    auto start_server = [&]() {
        fep3::native::HttpServer server(
            "s1", "http://0.0.0.0:0", "sys", "", nullptr, service_discovery_factory, false);
        {
            std::lock_guard<std::mutex> lk_set(mtx_set);
            urls.insert(server.getUrl());
        }

        // wait until main thread finishes
        bar.wait();
    };
    for (int i = 0; i < number_of_servers; ++i) {
        my_threads.emplace_back(start_server);
    }

    for (auto s: urls) {
        EXPECT_ANY_THROW(fep3::native::HttpServer server_X(
            "sX", s, "sys", "", nullptr, service_discovery_factory, false));
    }

    bar.wait();
    for (auto& t: my_threads) {
        t.join();
    }
    EXPECT_EQ(int(urls.size()), number_of_servers);
}

TEST(HttpServerTest, testHttpServerHeartbeat)
{
    using namespace std::chrono_literals;
    using ServiceDiscoveryNiceMock = NiceMock<ServiceDiscoveryMock>;

    std::shared_ptr<ServiceDiscoveryFactoryMock> _service_discovery_factory(
        std::make_shared<ServiceDiscoveryFactoryMock>());

    // The default heartbeat interval is 5 seconds.
    // We expect at least 2 heartheats in 6 seconds.
    {
        auto executor_unique_ptr =
            std::make_unique<fep3::native::mock::FakeThreadPoolExecutor>(100ms);
        fep3::native::mock::FakeThreadPoolExecutor* executor = executor_unique_ptr.get();
        auto service_discovery = std::make_unique<ServiceDiscoveryNiceMock>();

        EXPECT_CALL(*service_discovery.get(), sendNotifyAlive()).Times(AtLeast(2));
        EXPECT_CALL(*service_discovery.get(), checkForMSearchAndSendResponse(_))
            .WillRepeatedly(Return(true));

        EXPECT_CALL(*_service_discovery_factory, getServiceDiscovery(_, _, _, _, _, _, _, _, _, _))
            .WillOnce(Return(ByMove(std::move(service_discovery))));
        auto server = std::make_unique<fep3::native::HttpServer>("s1",
                                                                 "http://0.0.0.0:0",
                                                                 "sys",
                                                                 "url",
                                                                 nullptr,
                                                                 _service_discovery_factory,
                                                                 true,
                                                                 std::move(executor_unique_ptr));

        executor->run_until(6000ms);
    }

    // The heartbeat interval is changed to 100ms on the fly
    // We expect at least 10 heartbeats in 1100ms
    {
        auto executor_unique_ptr =
            std::make_unique<fep3::native::mock::FakeThreadPoolExecutor>(100ms);
        fep3::native::mock::FakeThreadPoolExecutor* executor = executor_unique_ptr.get();
        auto service_discovery = std::make_unique<ServiceDiscoveryNiceMock>();

        EXPECT_CALL(*service_discovery.get(), sendNotifyAlive()).Times(AtLeast(10));
        EXPECT_CALL(*service_discovery.get(), checkForMSearchAndSendResponse(_))
            .WillRepeatedly(Return(true));

        EXPECT_CALL(*_service_discovery_factory, getServiceDiscovery(_, _, _, _, _, _, _, _, _, _))
            .WillOnce(Return(ByMove(std::move(service_discovery))));

        auto server = std::make_unique<fep3::native::HttpServer>("s1",
                                                                 "http://0.0.0.0:0",
                                                                 "sys",
                                                                 "url",
                                                                 nullptr,
                                                                 _service_discovery_factory,
                                                                 true,
                                                                 std::move(executor_unique_ptr));

        server->setHeartbeatInterval(100ms);
        executor->run_for(1100ms);
    }
}
