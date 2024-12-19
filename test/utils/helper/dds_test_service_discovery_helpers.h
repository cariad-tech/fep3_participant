/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/logging/mock_logger.h>

#include <dds_service_discovery_topic_handler.h>
#include <gmock_async_helper.h>
#include <host_name_resolver_intf.h>

class DDSDiscoveryPublisher : public ::testing::Test {
protected:
    DDSDiscoveryPublisher() : domain_participant(0)
    {
    }

    void SetUp() override
    {
        dds::pub::qos::DataWriterQos writer_qos_disc =
            dds::core::QosProvider::Default().datawriter_qos();

        writer_qos_disc << dds::core::policy::Durability(
            dds::core::policy::DurabilityKind::TRANSIENT_LOCAL);
        writer_qos_disc << dds::core::policy::Reliability(
            dds::core::policy::ReliabilityKind::RELIABLE);

        writer.reset(new dds::all::DataWriter<DdsServiceDiscovery>(
            dds::all::Publisher(domain_participant),
            dds::all::Topic<DdsServiceDiscovery>(domain_participant, "service_discovery"),
            writer_qos_disc));
    }

    void writeServiceDiscovery(std::string id, ParticipantData participant_data)
    {
        DdsServiceDiscovery sample;
        sample.id(id);
        sample.content(participant_data);
        writer->write(sample);
    }

    dds::all::DomainParticipant domain_participant;

    std::unique_ptr<dds::all::DataWriter<DdsServiceDiscovery>> writer;
};

struct MockHostNameResolver : public fep3::native::IHostNameResolver {
    MOCK_METHOD2(findIp, std::string(const std::string&, const std::string&));
};

inline void waitForDiscoverySample(std::string sample_name,
                                   std::string service_name_contains,
                                   std::chrono::seconds wait_time)
{
    auto done = std::make_shared<::test::helper::Notification>();

    // no need to create multiple participants for one domain
    static fep3::native::DdsSdParticipant dds_sd_participant;

    std::function<void(const DdsServiceDiscovery&)> _callback =
        [&](const DdsServiceDiscovery& sample) {
            if (sample.content().service_name().find(service_name_contains) != std::string::npos) {
                std::cout << "notify sample received "
                          << "\n";
                done->notify();
            }
        };

    fep3::native::DDSTopicHandler _handler(
        _callback, std::move(sample_name), dds_sd_participant, DdsServiceDiscovery{});

    done->waitForNotificationWithTimeout(std::move(wait_time));
}

class ProcessedDiscoverySampleWaiter {
public:
    ProcessedDiscoverySampleWaiter(std::vector<std::string> services_to_wait_for,
                                   std::string log_string = "Received service update from service")
        : _log_string(std::move(log_string))
    {
        std::copy(services_to_wait_for.begin(),
                  services_to_wait_for.end(),
                  std::back_inserter(_services_to_wait_for));

        using namespace ::testing;

        EXPECT_CALL(*_mock_logger, isDebugEnabled()).WillRepeatedly(Return(true));
        EXPECT_CALL(*_mock_logger, logDebug(_))
            .WillRepeatedly(DoAll(Invoke([&](const std::string& log) { logCallBack(log); }),
                                  Return(a_util::result::Result())));
    }

    void logCallBack(const std::string& log)
    {
        std::unique_lock<std::mutex> lock(_vector_mutex);
        std::for_each(_services_to_wait_for.begin(),
                      _services_to_wait_for.end(),
                      [&](auto& service_log_counter) {
                          if (log.find(_log_string) != std::string::npos) {
                              if (log.find(service_log_counter._service_to_wait_for) !=
                                  std::string::npos) {
                                  ++service_log_counter._received_samples;
                              }
                          }
                      });

        if (std::all_of(_services_to_wait_for.begin(),
                        _services_to_wait_for.end(),
                        [&](const ServiceLogCounter& service_log_counter) {
                            return service_log_counter._received_samples > 0;
                        })) {
            if (_waiting) {
                _waiting = false;
                _done->notify();
            }
        }
    }
    std::shared_ptr<fep3::ILogger> getLogger()
    {
        return _mock_logger;
    }

    void waitForProcessedDiscoverySample(std::chrono::seconds wait_time)
    {
        _waiting = true;
        _done->waitForNotificationWithTimeout(std::move(wait_time));
    }

    void add_wait_service(std::string service_name)
    {
        std::unique_lock<std::mutex> lock(_vector_mutex);
        _services_to_wait_for.emplace_back(std::move(service_name));
    }

private:
    struct ServiceLogCounter {
        ServiceLogCounter(std::string service_to_wait_for)
            : _service_to_wait_for(std::move(service_to_wait_for)), _received_samples(0)
        {
        }

        std::string _service_to_wait_for;
        uint8_t _received_samples;
    };
    std::vector<ServiceLogCounter> _services_to_wait_for;
    std::shared_ptr<::test::helper::Notification> _done =
        std::make_shared<::test::helper::Notification>();
    std::shared_ptr<::testing::NiceMock<fep3::mock::Logger>> _mock_logger =
        std::make_shared<::testing::NiceMock<fep3::mock::Logger>>();
    std::mutex _vector_mutex;
    std::atomic<bool> _waiting{false};
    std::string _log_string;
};
