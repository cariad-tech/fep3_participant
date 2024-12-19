/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "DdsServiceDiscoveryTopic.hpp"
#include "dds_service_discovery_participant.h"

namespace fep3 {

namespace helper {
class Url;
} // namespace helper

namespace native {

class ServiceDiscTopicWriter {
public:
    ServiceDiscTopicWriter(std::pair<std::string, bool> location_url,
                           std::string unique_service_name,
                           const DdsSdParticipant& dds_sd_participant);

    ~ServiceDiscTopicWriter();

    void sendBye();

    void sendAlive();

    ServiceDiscTopicWriter(ServiceDiscTopicWriter&&) = delete;
    ServiceDiscTopicWriter& operator=(ServiceDiscTopicWriter&&) = delete;
    ServiceDiscTopicWriter& operator=(const ServiceDiscTopicWriter&) = delete;

private:
    template <typename TopicType>
    std::unique_ptr<dds::pub::DataWriter<TopicType>> createWriter(
        const DdsSdParticipant& dds_sd_participant,
        dds::core::policy::DurabilityKind durability,
        const std::string& topic_name)
    {
        dds::pub::qos::DataWriterQos writer_qos =
            dds::core::QosProvider::Default().datawriter_qos();
        writer_qos << dds::core::policy::Durability(durability);
        writer_qos << dds::core::policy::Reliability(dds::core::policy::ReliabilityKind::RELIABLE);
        writer_qos << dds::core::policy::History(dds::core::policy::HistoryKind::KEEP_LAST);

        return std::make_unique<dds::pub::DataWriter<TopicType>>(
            dds::pub::Publisher(dds_sd_participant.getDomainParticipant()),
            dds_sd_participant.getTopic<TopicType>(topic_name),
            writer_qos);
    }

    void writeSample(ServiceDiscoveryResponseType response_type);

    std::string getLocationUrl(const fep3::helper::Url& url, bool default_server_url);

    // dds writers
    std::unique_ptr<dds::pub::DataWriter<DdsServiceDiscovery>> writer;

    ParticipantData _part_data;
    static uint32_t _creation_counter;
};

} // namespace native
} // namespace fep3
