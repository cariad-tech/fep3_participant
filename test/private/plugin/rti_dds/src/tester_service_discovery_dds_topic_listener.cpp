/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <dds_test_service_discovery_helpers.h>
#include <service_discovery_dds_topic_listener.h>
#include <service_discovery_topic_writer.h>

using namespace ::testing;

TEST(TestDDS_Service_Discovery, Test_Writer_Listener)
{
    fep3::native::DdsSdParticipant dds_sd_participant;

    fep3::native::ServiceDiscTopicWriter sd_topic_writer(
        std::make_pair("http://host1:9090", true), "service1", dds_sd_participant);
    fep3::native::ServiceDiscTopicWriter sd_topic_writer_2(
        std::make_pair("http://host1:9091", true), "service2", dds_sd_participant);

    auto mock_logger = std::make_shared<StrictMock<fep3::mock::Logger>>();

    auto mock_host_name_resovler = std::make_unique<StrictMock<MockHostNameResolver>>();
    MockHostNameResolver* mock_pointer = mock_host_name_resovler.get();

    ProcessedDiscoverySampleWaiter processed_sample_waiter({"service1", "service2"},
                                                           "Processed update event from server");
    fep3::native::ServiceDiscTopicListener test_sd_topic_listener(
        std::make_shared<fep3::native::LoggerProxy>(processed_sample_waiter.getLogger()),
        dds_sd_participant,
        std::move(mock_host_name_resovler));

    EXPECT_CALL(*mock_pointer, findIp(_, "9090")).WillRepeatedly(Return("ip1"));
    EXPECT_CALL(*mock_pointer, findIp(_, "9091")).WillRepeatedly(Return("ip2"));

    sd_topic_writer.sendAlive();
    sd_topic_writer_2.sendBye();

    using namespace std::literals::chrono_literals;
    processed_sample_waiter.waitForProcessedDiscoverySample(30s);

    std::vector<fep3::native::ServiceUpdateEvent> processed_updated =
        test_sd_topic_listener.getProcessedUpdates();

    // we have to receive at list one signal from each writer.
    ASSERT_GE(processed_updated.size(), 2);
}

TEST(TestDDS_Service_Discovery, Test_Writer_Listener_non_default_server_url)
{
    fep3::native::DdsSdParticipant dds_sd_participant;

    fep3::native::ServiceDiscTopicWriter sd_topic_writer(
        std::make_pair("http://127.0.0.1:9090", false), "service1", dds_sd_participant);
    fep3::native::ServiceDiscTopicWriter sd_topic_writer_2(
        std::make_pair("http://127.0.0.1:9091", false), "service2", dds_sd_participant);

    auto mock_logger = std::make_shared<StrictMock<fep3::mock::Logger>>();

    auto mock_host_name_resovler = std::make_unique<StrictMock<MockHostNameResolver>>();
    MockHostNameResolver* mock_pointer = mock_host_name_resovler.get();

    ProcessedDiscoverySampleWaiter processed_sample_waiter(
        {"with url: http://127.0.0.1:9090", "with url: http://127.0.0.1:9091"},
        "Processed update event from server");
    fep3::native::ServiceDiscTopicListener test_sd_topic_listener(
        std::make_shared<fep3::native::LoggerProxy>(processed_sample_waiter.getLogger()),
        dds_sd_participant,
        std::move(mock_host_name_resovler));

    EXPECT_CALL(*mock_pointer, findIp("127.0.0.1", "9090")).WillRepeatedly(Return("ip1"));
    EXPECT_CALL(*mock_pointer, findIp("127.0.0.1", "9091")).WillRepeatedly(Return("ip2"));

    sd_topic_writer.sendAlive();
    sd_topic_writer_2.sendBye();

    using namespace std::literals::chrono_literals;
    processed_sample_waiter.waitForProcessedDiscoverySample(30s);

    std::vector<fep3::native::ServiceUpdateEvent> processed_updated =
        test_sd_topic_listener.getProcessedUpdates();

    // we have to receive at list one signal from each writer.
    ASSERT_GE(processed_updated.size(), 2);
}
