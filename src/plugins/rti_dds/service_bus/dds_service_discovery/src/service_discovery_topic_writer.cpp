/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../include/service_discovery_topic_writer.h"

#include <a_util/system/system.h>

#include <cxx_url.h>

namespace fep3 {
namespace native {

ServiceDiscTopicWriter::ServiceDiscTopicWriter(std::pair<std::string, bool> location_url,
                                               std::string unique_service_name,
                                               const DdsSdParticipant& dds_sd_participant)
{
    writer = createWriter<DdsServiceDiscovery>(dds_sd_participant,
                                               dds::core::policy::DurabilityKind::TRANSIENT_LOCAL,
                                               "service_discovery");

    fep3::helper::Url location(location_url.first);
    const std::string http_serv_addr = std::string("http://") +
                                       getLocationUrl(location, location_url.second) + ":" +
                                       location.port();
    _part_data = ParticipantData{unique_service_name, http_serv_addr};
    // here we write our discovery data, this is reliable and durable so it will be written once and
    // received by all other participants
    writeSample(ServiceDiscoveryResponseType::ResponseDiscover);
}

ServiceDiscTopicWriter::~ServiceDiscTopicWriter()
{
    writer.reset();
}

void ServiceDiscTopicWriter::sendBye()
{
    writeSample(ServiceDiscoveryResponseType::ResponseBye);
}

void ServiceDiscTopicWriter::sendAlive()
{
    writeSample(ServiceDiscoveryResponseType::ResponseAlive);
}

void ServiceDiscTopicWriter::writeSample(ServiceDiscoveryResponseType response_type)
{
    DdsServiceDiscovery sample;
    sample.id(_part_data.service_name());
    sample.response_type(response_type);
    sample.content(_part_data);
    writer->write(sample);
}

std::string ServiceDiscTopicWriter::getLocationUrl(const fep3::helper::Url& url,
                                                   bool is_default_server_url)
{
    // standard url used
    if (is_default_server_url) {
        return a_util::system::getHostname();
    }
    else {
        return url.host();
    }
}

} // namespace native
} // namespace fep3
