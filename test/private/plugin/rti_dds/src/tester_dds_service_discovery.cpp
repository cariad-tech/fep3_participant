/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */
#include "dds_test_service_discovery_helpers.h"
#include "service_discovery_factory_dds.h"

#include <a_util/process/process.h>

#include <boost/asio.hpp>
#include <boost/process.hpp>

TEST(TestConnextDDSS_ServiceDiscovery, DiscoveryServicerDiscoveryFinderPairTest)
{
    // open a local server so that the host name resolution can work
    boost::asio::io_service service;
    boost::asio::ip::tcp::acceptor acceptor(
        service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0));
    unsigned short port = acceptor.local_endpoint().port();

    fep3::native::ServiceDiscoveryFactory discovery_factory{};

    const std::string location_url = "http://127.0.0.1:" + std::to_string(port);
    const std::string unique_service_name = "service1";

    using namespace std::literals::chrono_literals;
    auto service_discovery = discovery_factory.getServiceDiscovery(
        "", "", 0s, std::make_pair(location_url, true), unique_service_name, "", "", "", "", "");

    ProcessedDiscoverySampleWaiter processed_sample_waiter_1({"service1"},
                                                             "Processed update event from server");

    auto service_finder = discovery_factory.getServiceFinder(
        processed_sample_waiter_1.getLogger(), "", "", "", "", "");
    processed_sample_waiter_1.waitForProcessedDiscoverySample(20s);

    std::vector<fep3::native::ServiceUpdateEvent> update_events;
    auto callback = [&](const fep3::native::ServiceUpdateEvent& update_event) {
        update_events.emplace_back(update_event);
    };

    service_finder->checkForServices(callback, 1s);
    ASSERT_GT(update_events.size(), 0);
    ASSERT_EQ(update_events[0]._service_name, unique_service_name);
}

TEST(TestConnextDDSS_ServiceDiscovery, DiscoverySetDifferentDomainIds)
{
    boost::asio::io_service service;
    boost::asio::ip::tcp::acceptor acceptor(
        service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0));
    unsigned short port = acceptor.local_endpoint().port();

    fep3::native::ServiceDiscoveryFactory discovery_factory{};

    const std::string location_url = "http://127.0.0.1:" + std::to_string(port);
    const std::string unique_service_name = "service1";

    using namespace std::literals::chrono_literals;
    auto service_discovery = discovery_factory.getServiceDiscovery(
        "", "", 0s, std::make_pair(location_url, true), unique_service_name, "", "", "", "", "");

    ProcessedDiscoverySampleWaiter processed_sample_waiter_1({"service1"},
                                                             "Processed update event from server");

    a_util::process::setEnvVar("FEP3_SERVICE_BUS_DOMAIN_ID", "2");
    fep3::native::ServiceDiscoveryFactory discovery_factory_finder{};

    auto service_finder = discovery_factory_finder.getServiceFinder(
        processed_sample_waiter_1.getLogger(), "", "", "", "", "");

    std::vector<fep3::native::ServiceUpdateEvent> update_events;
    auto callback = [&](const fep3::native::ServiceUpdateEvent& update_event) {
        update_events.emplace_back(update_event);
    };

    service_finder->checkForServices(callback, 1s);
    ASSERT_EQ(update_events.size(), 0);
    a_util::process::setEnvVar("FEP3_SERVICE_BUS_DOMAIN_ID", "");
}
