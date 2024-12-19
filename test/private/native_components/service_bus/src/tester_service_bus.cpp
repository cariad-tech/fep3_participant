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

#include <helper/gmock_async_helper.h>
#include <http_server.h>
#include <mock_logger_addons.h>
#include <service_bus.h>

using namespace ::testing;
using namespace std::chrono_literals;

TEST(ServiceBusServer, testGetRequester)
{
    fep3::native::ServiceBus bus;
    const std::string system_name = "sysname";
    const std::string server_name = "server_name";

    ASSERT_TRUE(bus.createSystemAccess(
        system_name, fep3::IServiceBus::ISystemAccess::_use_default_url, true));
    std::shared_ptr<fep3::arya::IServiceBus::ISystemAccess> system_access =
        bus.getSystemAccess(system_name);

    ASSERT_TRUE(system_access->createServer(
        server_name, fep3::arya::IServiceBus::ISystemAccess::_use_default_url));

    std::shared_ptr<fep3::arya::IServiceBus::IParticipantRequester> requester;

    ASSERT_NO_THROW(requester = bus.getRequester("non_existing_server"));
    ASSERT_EQ(nullptr, requester);

    ASSERT_NO_THROW(requester = bus.getRequester(server_name));
    ASSERT_NE(nullptr, requester);
}

/**
 * @detail Test the registration, unregistration and memorymanagment of the ServiceBus
 * @req_id FEPSDK-ServiceBus
 */
TEST(ServiceBusServer, testCreationAndDestroyingOfSystemAccess)
{
    fep3::native::ServiceBus bus;
    ASSERT_TRUE(
        bus.createSystemAccess("sysname", fep3::IServiceBus::ISystemAccess::_use_default_url));

    auto sys_access = bus.getSystemAccess("sysname");
    ASSERT_TRUE(sys_access);

    // not yet created
    auto sys_access2 = bus.getSystemAccess("sysname2");
    ASSERT_FALSE(sys_access2);

    ASSERT_TRUE(bus.createSystemAccess(
        "sysname2", fep3::IServiceBus::ISystemAccess::_use_default_url, true));

    // now the second one is created
    sys_access2 = bus.getSystemAccess("sysname2");
    ASSERT_TRUE(sys_access2);

    // failure because already exists
    ASSERT_FALSE(
        bus.createSystemAccess("sysname2", fep3::IServiceBus::ISystemAccess::_use_default_url));

    // failure because invalid scheme
    ASSERT_FALSE(bus.createSystemAccess("name_of_system_invalid_scheme", "foo://0.0.0.0:9091"));

    // failure because invalid url
    ASSERT_FALSE(bus.createSystemAccess("name_of_sys_invalid_url", "0.0.0.0:9091"));

    // destroy it
    ASSERT_TRUE(bus.releaseSystemAccess("sysname2"));

    // not accessible anymore!
    sys_access2 = bus.getSystemAccess("sysname2");
    ASSERT_FALSE(sys_access2);

    // can not destroy it ... it does not exist anymore
    ASSERT_FALSE(bus.releaseSystemAccess("sysname2"));
}

/**
 * @detail Test the registration, unregistration and memorymanagment of the ServiceBus
 * @req_id FEPSDK-ServiceBus
 */
TEST(ServiceBusServer, testCreationAndDestroyingOfServer)
{
    fep3::native::ServiceBus bus;

    // no default server set
    auto server = bus.getServer();
    ASSERT_FALSE(server);

    ASSERT_TRUE(bus.createSystemAccess(
        "sysname", fep3::IServiceBus::ISystemAccess::_use_default_url, true));

    auto sys_access = bus.getSystemAccess("sysname");
    ASSERT_TRUE(sys_access);

    // no default server set yet
    server = bus.getServer();
    ASSERT_FALSE(server);

    // now create the server
    ASSERT_TRUE(sys_access->createServer("name_of_server",
                                         fep3::IServiceBus::ISystemAccess::_use_default_url));

    // default server set now
    server = bus.getServer();
    ASSERT_TRUE(server);

    // default server is the same like in "sysname" system access
    auto server_same = sys_access->getServer();
    ASSERT_TRUE(server_same);

    ASSERT_EQ(server_same->getName(), server->getName());
    ASSERT_EQ(server_same->getUrl(), server->getUrl());

    // just make sure another system access will not override the default
    ASSERT_TRUE(bus.createSystemAccess(
        "sysname_for_failure_tests", fep3::IServiceBus::ISystemAccess::_use_default_url, false));

    // default server is still set (because we use it from the first system access)
    server = bus.getServer();
    ASSERT_TRUE(server);
    ASSERT_EQ(server_same->getName(), server->getName());
    ASSERT_EQ(server_same->getUrl(), server->getUrl());

    // failure test: because invalid scheme in server url for native service bus impl
    auto sys_access_for_failure_tests = bus.getSystemAccess("sysname_for_failure_tests");
    ASSERT_TRUE(sys_access);
    ASSERT_FALSE(sys_access_for_failure_tests->createServer("name_of_system_invalid_scheme",
                                                            "foo://0.0.0.0:9091"));

    // failure because invalid scheme in url
    ASSERT_FALSE(
        sys_access_for_failure_tests->createServer("name_of_server_invalid_url", "//0.0.0.0:9091"));
}

/**
 * @detail Test the registration, unregistration and memorymanagment of the ServiceBus
 * @req_id FEPSDK-ServiceBus
 */
TEST(ServiceBusServer, testDefaultLoadingOfServiceBus)
{
    fep3::native::ServiceBus bus;
    ASSERT_TRUE(bus.createSystemAccess("default_system",
                                       fep3::IServiceBus::ISystemAccess::_use_default_url));
    auto sys_access = bus.getSystemAccess("default_system");
    ASSERT_TRUE(sys_access->createServer("default_server",
                                         fep3::IServiceBus::ISystemAccess::_use_default_url));
}

/**
 * @detail Test the discovery methods of the native HTTP System Access and the creation of it
 * This test check if create will lock the creation and changing of the service bus content somehow
 * @req_id FEPSDK-ServiceBus
 */
TEST(ServiceBusServer, testServiceBusLocking)
{
#ifdef WIN32
    constexpr const char* const ADDR_USE_FOR_TEST = "http://230.231.0.0:9993";
#else
    constexpr const char* const ADDR_USE_FOR_TEST =
        fep3::IServiceBus::ISystemAccess::_use_default_url;
#endif

    // use a service bus
    fep3::native::ServiceBus* bus1 = new fep3::native::ServiceBus();
    ASSERT_TRUE(bus1->create());
    // this is not possible
    ASSERT_FALSE(bus1->createSystemAccess("test_sys", ADDR_USE_FOR_TEST));
    ASSERT_TRUE(bus1->destroy());
    // now it is possible
    ASSERT_TRUE(bus1->createSystemAccess("test_sys", ADDR_USE_FOR_TEST));

    ASSERT_TRUE(bus1->create());

    // this is still possible
    auto sys_access = bus1->getSystemAccess("test_sys");
    // and i can get the sys access
    ASSERT_TRUE(sys_access);

    // but the creation of createServer within this is locked!
    ASSERT_FALSE(sys_access->createServer("test_server",
                                          fep3::IServiceBus::ISystemAccess::_use_default_url));

    // call the destuctor is still possible also if everythis is locked
    ASSERT_NO_THROW(bus1->~ServiceBus());
}

#if defined(DDS_SERVICE_DISCOVERY)
/**
 * @brief In case that a user defined system url is passed an error is logged
 */
TEST(ServiceBusServer, testCustomServerUrlLogError)
{
    std::shared_ptr<::testing::NiceMock<fep3::mock::ErrorLogger>> _mock_logger =
        std::make_shared<::testing::NiceMock<fep3::mock::ErrorLogger>>();
    fep3::native::ServiceBus bus(_mock_logger);

    EXPECT_CALL(*_mock_logger, logError(_)).WillOnce(Return(a_util::result::Result()));

    ASSERT_TRUE(bus.createSystemAccess("default_system", "http://230.230.231.1:0"));
}
#endif
