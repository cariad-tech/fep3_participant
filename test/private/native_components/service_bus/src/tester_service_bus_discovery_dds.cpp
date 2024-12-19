/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "dds_test_service_discovery_helpers.h"

#include <a_util/process.h>

#include <list>
#include <service_bus.h>
#include <thread>

using namespace ::testing;

namespace {
bool contains(const std::multimap<std::string, std::string>& servers,
              const std::list<std::string>& list_of_content_to_check)
{
    for (const auto& current_string_to_check: list_of_content_to_check) {
        auto found = (servers.find(current_string_to_check) != servers.cend());
        if (!found) {
            // did not find it
            return false;
        }
    }
    return true;
}
} // namespace
/**
 * @detail Test the discovery methods of the native HTTP System Access and the creation of it
 * @req_id FEPSDK-ServiceBus
 */
TEST(ServiceBusServer, testHTTPSystemAccessDiscovery)
{
#ifdef WIN32
    constexpr const char* const ADDR_USE_FOR_TEST = "http://230.231.0.0:9993";
#else
    constexpr const char* const ADDR_USE_FOR_TEST =
        fep3::IServiceBus::ISystemAccess::_use_default_url;
#endif
    std::stringstream ss;
    ss << std::this_thread::get_id();

    std::string system_name_for_test_1 = "system_1_" +
                                         std::to_string(a_util::process::getCurrentProcessId()) +
                                         std::string("_") + ss.str();

    ProcessedDiscoverySampleWaiter processed_sample_waiter_1(
        {"server_1@" + system_name_for_test_1});

    // create a system access to the named system "system_name_for_test_1" on the default URL
    fep3::native::ServiceBus bus1(processed_sample_waiter_1.getLogger());

    ASSERT_TRUE(bus1.createSystemAccess(system_name_for_test_1, ADDR_USE_FOR_TEST));

    // create one server within this system_name_for_test_1 (so it is discoverable)
    auto sys_access1 = bus1.getSystemAccess(system_name_for_test_1);
    ASSERT_TRUE(
        sys_access1->createServer("server_1", fep3::IServiceBus::ISystemAccess::_use_default_url));

    // create another system access to the same system under the same discovery url in another
    // ServiceBus instance
    ProcessedDiscoverySampleWaiter processed_sample_waiter_2(
        {"server_1@" + system_name_for_test_1});
    fep3::native::ServiceBus bus2(processed_sample_waiter_2.getLogger());

    ASSERT_TRUE(bus2.createSystemAccess(system_name_for_test_1, ADDR_USE_FOR_TEST));

    auto sys_access2 = bus2.getSystemAccess(system_name_for_test_1);

    using namespace std::literals::chrono_literals;
    processed_sample_waiter_1.waitForProcessedDiscoverySample(10s);
    processed_sample_waiter_2.waitForProcessedDiscoverySample(10s);
    // usually this will discover the server in sys_access1, but is asyncronously
    // the discover will send a search and wait at least the given time for responses!
    auto list_of_discovered = sys_access2->discover(std::chrono::seconds(1));
    ASSERT_EQ(list_of_discovered.size(), 1);

    auto const& ref = list_of_discovered.begin();
    ASSERT_EQ(ref->first, "server_1");

    processed_sample_waiter_1.add_wait_service("server_2@" + system_name_for_test_1);
    processed_sample_waiter_2.add_wait_service("server_2@" + system_name_for_test_1);

    // create another server within this system_name_for_test_1 (so it is discoverable)
    ASSERT_TRUE(
        sys_access2->createServer("server_2", fep3::IServiceBus::ISystemAccess::_use_default_url));

    // make sure this both server is now discoverable thru both access points
    // this is now the first access point on bus1
    processed_sample_waiter_1.waitForProcessedDiscoverySample(10s);
    auto list_of_discovered_at_1 = sys_access1->discover(std::chrono::seconds(1));
    ASSERT_EQ(list_of_discovered_at_1.size(), 2);

    ASSERT_TRUE(contains(list_of_discovered_at_1, {"server_1", "server_2"}));

    // make sure this both server is now discoverable thru both access points
    // this is now the second access point
    processed_sample_waiter_2.waitForProcessedDiscoverySample(10s);
    auto list_of_discovered_at_2 = sys_access2->discover(std::chrono::milliseconds(5));
    ASSERT_EQ(list_of_discovered_at_2.size(), 2);

    ASSERT_TRUE(contains(list_of_discovered_at_2, {"server_1", "server_2"}));
}

/**
 * @detail Test the discovery methods of the native HTTP System Access and the creation of it
 * @req_id FEPSDK-ServiceBus
 */
TEST(ServiceBusServer, testHTTPSystemAccessDiscoveryAllSystems)
{
#ifdef WIN32
    constexpr const char* const ADDR_USE_FOR_TEST = "http://230.231.0.0:9993";
#else
    constexpr const char* const ADDR_USE_FOR_TEST =
        fep3::IServiceBus::ISystemAccess::_use_default_url;
#endif
    std::stringstream ss;
    ss << std::this_thread::get_id();

    std::string system_name_for_test_1 = "system_1_" +
                                         std::to_string(a_util::process::getCurrentProcessId()) +
                                         std::string("_") + ss.str();

    std::string system_name_for_test_2 = "system_2_" +
                                         std::to_string(a_util::process::getCurrentProcessId()) +
                                         std::string("_") + ss.str();

    // create a system access to the named system "system_name_for_test_1" on the default URL
    fep3::native::ServiceBus bus1;
    ASSERT_TRUE(bus1.createSystemAccess(system_name_for_test_1, ADDR_USE_FOR_TEST));

    // create a system access to the named system "system_name_for_test_2" on the default URL
    ASSERT_TRUE(bus1.createSystemAccess(system_name_for_test_2, ADDR_USE_FOR_TEST));

    // create one server within this system_name_for_test_1 (so it is discoverable)
    // so we have server1@system_name_for_test_1
    auto sys_access1 = bus1.getSystemAccess(system_name_for_test_1);
    ASSERT_TRUE(
        sys_access1->createServer("server_1", fep3::IServiceBus::ISystemAccess::_use_default_url));

    // create one server within this system_name_for_test_2 (so it is discoverable)
    // so we have server2@system_name_for_test_2
    //        AND server1@system_name_for_test_1
    auto sys_access2 = bus1.getSystemAccess(system_name_for_test_2);
    ASSERT_TRUE(
        sys_access2->createServer("server_2", fep3::IServiceBus::ISystemAccess::_use_default_url));

    // create another system access to the same system under the same discovery url in another
    // ServiceBus instance
    fep3::native::ServiceBus bus2;
    ASSERT_TRUE(bus2.createSystemAccess(system_name_for_test_1, ADDR_USE_FOR_TEST));

    // create one server within this system_name_for_test_1 (so it is discoverable)
    // so we have server3@system_name_for_test_1
    //        AND server2@system_name_for_test_2
    //        AND server1@system_name_for_test_1
    auto sys_access3 = bus2.getSystemAccess(system_name_for_test_1);
    ASSERT_TRUE(
        sys_access3->createServer("server_3", fep3::IServiceBus::ISystemAccess::_use_default_url));

    // create a system access to special discovery mode
    // "fep3::IServiceBus::ISystemAccess::_discover_all_systems"
    //  on given URL (where the above servers must be available to)
    ProcessedDiscoverySampleWaiter processed_sample_waiter({"server_1@" + system_name_for_test_1,
                                                            "server_2@" + system_name_for_test_2,
                                                            "server_3@" + system_name_for_test_1});
    fep3::native::ServiceBus bus3(processed_sample_waiter.getLogger());
    ASSERT_TRUE(bus3.createSystemAccess(fep3::IServiceBus::ISystemAccess::_discover_all_systems,
                                        ADDR_USE_FOR_TEST));
    // get this special discovery system name
    auto sys_access_all =
        bus3.getSystemAccess(fep3::IServiceBus::ISystemAccess::_discover_all_systems);

    using namespace std::literals::chrono_literals;
    processed_sample_waiter.waitForProcessedDiscoverySample(10s);

    auto list_of_discovered_at_discover_all_systems =
        sys_access_all->discover(std::chrono::seconds(1));
    // if we discover all we can not assure, that on other testsystem or other network nodes are no
    // participant available so we maybe discover also the others, but we make sure, that our test
    // servers are available
    ASSERT_GE(list_of_discovered_at_discover_all_systems.size(), 3);

    ASSERT_TRUE(contains(list_of_discovered_at_discover_all_systems,
                         {std::string(std::string("server_1@") + system_name_for_test_1),
                          std::string(std::string("server_2@") + system_name_for_test_2),
                          std::string(std::string("server_3@") + system_name_for_test_1)}));
}
