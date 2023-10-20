/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "dds_service_discovery.h"

#include "service_discovery_topic_writer.h"

namespace fep3 {
namespace native {

ServiceDiscoveryDDS::ServiceDiscoveryDDS(std::pair<std::string, bool> location_url,
                                         std::string unique_service_name,
                                         const DdsSdParticipant& dds_sd_participant)
    : _service_disc_topic_writer(std::make_unique<ServiceDiscTopicWriter>(
          location_url, unique_service_name, dds_sd_participant))
{
}

ServiceDiscoveryDDS::~ServiceDiscoveryDDS()
{
}

bool ServiceDiscoveryDDS::sendNotifyAlive()
{
    _service_disc_topic_writer->sendAlive();
    return true;
}

bool ServiceDiscoveryDDS::sendNotifyByeBye()
{
    _service_disc_topic_writer->sendBye();
    return true;
}

bool ServiceDiscoveryDDS::checkForMSearchAndSendResponse(std::chrono::milliseconds)
{
    return true;
}

std::string ServiceDiscoveryDDS::getLastSendErrors() const
{
    return {};
}

} // namespace native
} // namespace fep3
