#include "dds_test_service_discovery_helpers.h"

using namespace dds::all;
using namespace ::testing;

TEST_F(DDSDiscoveryPublisher, TestReceiveServiceDiscoveryTopic)
{
    const std::string ip = "someIpAddress";
    const ParticipantData part_data{"service_name", "host_url"};
    writeServiceDiscovery(ip, part_data);
    auto done = std::make_shared<::test::helper::Notification>();

    std::function<void(const DdsServiceDiscovery&)> _callback =
        [&](const DdsServiceDiscovery& sample) {
            if (("service_name" == sample.content().service_name()) &&
                ("host_url" == sample.content().host_url()))
                done->notify();
        };

    fep3::native::DdsSdParticipant dds_sd_participant;
    fep3::native::DDSTopicHandler _handler(
        _callback, "service_discovery", dds_sd_participant, DdsServiceDiscovery{});

    using namespace std::literals::chrono_literals;
    ASSERT_TRUE(done->waitForNotificationWithTimeout(30s))
        << R"(Did not receive discovery sample for "service_name" and "host_url")";
}
