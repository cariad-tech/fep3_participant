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

#include <fep3/components/scheduler/mock/sceduled_task_mock.h>

#include <dds_service_update_handler.h>

using ::testing::Invoke;
using namespace ::testing;
using namespace std::literals::chrono_literals;
using ::testing::StrictMock;

void FindEvent(const std::vector<fep3::native::ServiceUpdateEvent>& processed_events,
               fep3::IServiceBus::ServiceUpdateEventType event_type,
               const std::string& url,
               const std::string& service_name)
{
    auto it = std::find_if(processed_events.begin(),
                           processed_events.end(),
                           [&](const fep3::native::ServiceUpdateEvent& update_event) {
                               return update_event._event_id == event_type;
                           });

    ASSERT_FALSE(it == processed_events.end());
    ASSERT_EQ(it->_service_name, service_name);
    ASSERT_EQ(it->_host_url, url);
}

TEST(TestConnextDDSS_ServiceUpdateHandler, SimpleTest)
{
    auto mock_host_name_resovler = std::make_unique<StrictMock<MockHostNameResolver>>();
    MockHostNameResolver* mock_pointer = mock_host_name_resovler.get();

    ProcessedDiscoverySampleWaiter processed_sample_waiter({"service1", "service2", "service3"},
                                                           "Processed update event from server");

    fep3::native::DssServiceUpdateHandler dds_service_update_handler(
        std::move(mock_host_name_resovler), processed_sample_waiter.getLogger());

    ParticipantData partData1{"service1", "http://host1:9090"},
        partData2{"service2", "http://host2:9091"}, partData3{"service3", "http://host3:9092"};
    auto done = std::make_shared<::test::helper::Notification>();

    EXPECT_CALL(*mock_pointer, findIp("host1", "9090")).WillOnce(Return("ip1"));

    EXPECT_CALL(*mock_pointer, findIp("host2", "9091")).WillOnce(Return("ip2"));

    EXPECT_CALL(*mock_pointer, findIp("host3", "9092")).WillOnce(Return("ip3"));
    DdsServiceDiscovery sd_data;

    dds_service_update_handler.addWork(
        DdsServiceDiscovery{"id1", partData1, ServiceDiscoveryResponseType::ResponseDiscover});
    dds_service_update_handler.addWork(
        DdsServiceDiscovery{"id2", partData2, ServiceDiscoveryResponseType::ResponseAlive});
    dds_service_update_handler.addWork(
        DdsServiceDiscovery{"id3", partData3, ServiceDiscoveryResponseType::ResponseBye});

    using namespace std::literals::chrono_literals;
    processed_sample_waiter.waitForProcessedDiscoverySample(5s);

    auto processed_updated = dds_service_update_handler.getProcessedUpdates();
    ASSERT_NO_FATAL_FAILURE(FindEvent(processed_updated,
                                      fep3::IServiceBus::ServiceUpdateEventType::response,
                                      "http://ip1:9090",
                                      "service1"));
    ASSERT_NO_FATAL_FAILURE(FindEvent(processed_updated,
                                      fep3::IServiceBus::ServiceUpdateEventType::notify_alive,
                                      "http://ip2:9091",
                                      "service2"));
    ASSERT_NO_FATAL_FAILURE(FindEvent(processed_updated,
                                      fep3::IServiceBus::ServiceUpdateEventType::notify_byebye,
                                      "http://ip3:9092",
                                      "service3"));
}

TEST(TestConnextDDSS_ServiceUpdateHandler, getProcessedUpdates__inOrder)
{
    auto mock_host_name_resovler = std::make_unique<StrictMock<MockHostNameResolver>>();
    MockHostNameResolver* mock_pointer = mock_host_name_resovler.get();

    ProcessedDiscoverySampleWaiter processed_sample_waiter({"service1", "service2", "service3"},
                                                           "Processed update event from server");

    auto thread_executor = std::make_unique<fep3::mock::MockThreadPoolExecutor>();

    fep3::native::DssServiceUpdateHandler dds_service_update_handler(
        std::move(mock_host_name_resovler),
        processed_sample_waiter.getLogger(),
        std::move(thread_executor));

    ParticipantData partData1{"service1", "http://host1:9090"},
        partData2{"service2", "http://host2:9091"}, partData3{"service3", "http://host3:9092"};
    auto done = std::make_shared<::test::helper::Notification>();

    EXPECT_CALL(*mock_pointer, findIp("host1", "9090")).WillOnce(Return("ip1"));

    EXPECT_CALL(*mock_pointer, findIp("host2", "9091")).WillOnce(Return("ip2"));

    EXPECT_CALL(*mock_pointer, findIp("host3", "9092")).WillOnce(Return("ip3"));
    DdsServiceDiscovery sd_data;

    dds_service_update_handler.addWork(
        DdsServiceDiscovery{"id1", partData1, ServiceDiscoveryResponseType::ResponseDiscover});
    dds_service_update_handler.addWork(
        DdsServiceDiscovery{"id2", partData2, ServiceDiscoveryResponseType::ResponseAlive});
    dds_service_update_handler.addWork(
        DdsServiceDiscovery{"id3", partData3, ServiceDiscoveryResponseType::ResponseBye});

    auto processed_updated = dds_service_update_handler.getProcessedUpdates();
    ASSERT_THAT(processed_updated[0],
                AllOf(Field(&fep3::native::ServiceUpdateEvent::_event_id,
                            fep3::IServiceBus::ServiceUpdateEventType::response),
                      Field(&fep3::native::ServiceUpdateEvent::_host_url, "http://ip1:9090"),
                      Field(&fep3::native::ServiceUpdateEvent::_service_name, "service1")));
    ASSERT_THAT(processed_updated[1],
                AllOf(Field(&fep3::native::ServiceUpdateEvent::_event_id,
                            fep3::IServiceBus::ServiceUpdateEventType::notify_alive),
                      Field(&fep3::native::ServiceUpdateEvent::_host_url, "http://ip2:9091"),
                      Field(&fep3::native::ServiceUpdateEvent::_service_name, "service2")));
    ASSERT_THAT(processed_updated[2],
                AllOf(Field(&fep3::native::ServiceUpdateEvent::_event_id,
                            fep3::IServiceBus::ServiceUpdateEventType::notify_byebye),
                      Field(&fep3::native::ServiceUpdateEvent::_host_url, "http://ip3:9092"),
                      Field(&fep3::native::ServiceUpdateEvent::_service_name, "service3")));
}
