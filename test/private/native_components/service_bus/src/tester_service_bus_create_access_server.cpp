/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <a_util/process/process.h>

#include <gmock_async_helper.h>
#include <service_bus.h>
#include <service_bust_multicast_receiver.h>

struct ServiceBusCreateSystemAccessAndServer : public ::testing::Test {
protected:
    void receive_callback(const std::vector<char>&)
    {
        done->notify();
    }

    void construct_receiver(const std::string& multicast_address, unsigned short port = 9990)
    {
        _receiver = std::make_shared<MulticastReceiver>(
            boost::asio::ip::address::from_string("0.0.0.0"),
            boost::asio::ip::address::from_string(multicast_address),
            [&](const std::vector<char>& data) { receive_callback(data); },
            port);
    }

    fep3::native::ServiceBus bus;
    const std::string system_name = "sysname";
    const std::string server_name = "server_name";
    std::shared_ptr<::test::helper::Notification> done =
        std::make_shared<::test::helper::Notification>();
    std::shared_ptr<MulticastReceiver> _receiver;
};

TEST_F(ServiceBusCreateSystemAccessAndServer, testSystemUrl)
{
    ASSERT_TRUE(bus.createSystemAccess(
        system_name, fep3::IServiceBus::ISystemAccess::_use_default_url, true));

    construct_receiver("230.230.230.1");

    _receiver->run();
    ASSERT_TRUE(done->waitForNotificationWithTimeout(std::chrono::seconds(15)))
        << "MSearch not received from multicast address";
    _receiver->stop();
}

TEST_F(ServiceBusCreateSystemAccessAndServer, testSystemUrlEnvVariable)
{
    const std::string multicast_address = "230.230.230.28";
    construct_receiver(multicast_address, 0);

    auto port = _receiver->get_port();
    const std::string multicast_http_address =
        "http://" + multicast_address + ":" + std::to_string(port);

    a_util::process::setEnvVar("FEP3_SERVICEBUS_SYSTEM_URL", multicast_http_address);

    ASSERT_TRUE(bus.createSystemAccess(
        system_name, fep3::IServiceBus::ISystemAccess::_use_default_url, true));

    _receiver->run();
    ASSERT_TRUE(done->waitForNotificationWithTimeout(std::chrono::seconds(15)))
        << "MSearch not received from multicast address defined in env variable";
    _receiver->stop();
    a_util::process::setEnvVar("FEP3_SERVICEBUS_SYSTEM_URL", "");
}

TEST_F(ServiceBusCreateSystemAccessAndServer, testSystemUrlUserInput)
{
    const std::string multicast_address = "230.230.230.30";
    construct_receiver(multicast_address, 0);

    auto port = _receiver->get_port();
    const std::string multicast_http_address =
        "http://" + multicast_address + ":" + std::to_string(port);

    ASSERT_TRUE(bus.createSystemAccess(system_name, multicast_http_address, true));

    _receiver->run();
    ASSERT_TRUE(done->waitForNotificationWithTimeout(std::chrono::seconds(15)))
        << "MSearch not received from multicast address defined in input argument";
    _receiver->stop();
}

TEST_F(ServiceBusCreateSystemAccessAndServer, testServerUrl)
{
    ASSERT_TRUE(bus.createSystemAccess(
        system_name, fep3::IServiceBus::ISystemAccess::_use_default_url, true));
    // bus.getSystemAccess("")->createServer("dummy_server",
    // fep3::IServiceBus::ISystemAccess::_use_default_url);
    //  non http schema throw
    ASSERT_FALSE(bus.getSystemAccess("")->createServer("dummy_server", "https:///123123.123123"));
    // url parser throws
    ASSERT_FALSE(bus.getSystemAccess("")->createServer("dummy_server", "1234:///12+++3123.123123"));
    // empty url / non http schema throw
    ASSERT_FALSE(bus.getSystemAccess("")->createServer("dummy_server", ""));

    // pass url through argument
    const std::string default_url = "http://0.0.0.0";
    ASSERT_NO_THROW(bus.getSystemAccess("")->createServer("dummy_server", default_url + ":0"));
    auto server_url = bus.getSystemAccess("")->getServer()->getUrl();
    ASSERT_NE(server_url.find(default_url), std::string::npos);

    // argument has wrong url, but environment variable the correct.
    a_util::process::setEnvVar("FEP3_SERVICEBUS_SERVER_URL", default_url + ":0");
    ASSERT_NO_THROW(
        bus.getSystemAccess("")->createServer("dummy_server", "1234:///12+++3123.123123"));
    server_url = bus.getSystemAccess("")->getServer()->getUrl();
    ASSERT_NE(server_url.find(default_url), std::string::npos);
    a_util::process::setEnvVar("FEP3_SERVICEBUS_SERVER_URL", "");
}
