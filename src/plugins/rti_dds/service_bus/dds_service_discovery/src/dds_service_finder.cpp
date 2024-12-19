/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../include/dds_service_finder.h"

#include "../include/service_discovery_dds_topic_listener.h"
#include "host_name_resolver.h"

namespace fep3 {
namespace native {

ServiceFinderDDS::ServiceFinderDDS(std::shared_ptr<ILogger> logger,
                                   const DdsSdParticipant& dds_sd_participant)
    : _logger(logger),
      _service_disc_topic_listener(std::make_unique<ServiceDiscTopicListener>(
          _logger, dds_sd_participant, std::make_unique<HostNameResolver>(logger)))
{
}

ServiceFinderDDS::~ServiceFinderDDS()
{
}

bool ServiceFinderDDS::checkForServices(
    const std::function<void(const ServiceUpdateEvent&)>& update_callback,
    std::chrono::milliseconds timeout)
{
    std::chrono::milliseconds poll_interval;
    if (timeout > _default_poll_interval) {
        poll_interval = _default_poll_interval;
    }
    else {
        poll_interval = timeout;
    }

    return pollServiceDiscovery(update_callback, poll_interval, timeout);
}

bool ServiceFinderDDS::pollServiceDiscovery(
    const std::function<void(const ServiceUpdateEvent&)>& update_callback,
    std::chrono::milliseconds interval,
    std::chrono::milliseconds duration)
{
    using namespace std::literals::chrono_literals;
    std::chrono::milliseconds wait_time = 0ms;

    while (_discovery_active && (wait_time <= duration)) {
        // we just poll _service_disc_topic_listener,
        //_service_disc_topic_listener thread will receive and process the discovery topics as they
        // are received
        const std::vector<ServiceUpdateEvent> currently_service_updates =
            _service_disc_topic_listener->getProcessedUpdates();
        for (const auto& service_update: currently_service_updates) {
            update_callback(service_update);
            _logger->logDebug("Received service update from service " +
                              service_update._service_name + " with host url " +
                              service_update._host_url + " event id " +
                              std::to_string(static_cast<int>(service_update._event_id)));
        }

        std::this_thread::sleep_for(interval);
        wait_time += interval;
    }

    return true;
}

bool ServiceFinderDDS::sendMSearch()
{
    return true;
}

std::string ServiceFinderDDS::getLastSendErrors() const
{
    return {};
}

void ServiceFinderDDS::disableDiscovery()
{
    _discovery_active = false;
}

} // namespace native
} // namespace fep3
