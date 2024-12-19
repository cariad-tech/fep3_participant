/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../include/service_discovery_dds_topic_listener.h"

#include "host_name_resolver.h"

namespace fep3 {
namespace native {
ServiceDiscTopicListener::ServiceDiscTopicListener(
    std::shared_ptr<ILogger> logger,
    const DdsSdParticipant& dds_sd_participant,
    std::unique_ptr<IHostNameResolver> host_name_resolver)
    : _host_name_resolver(std::move(host_name_resolver)), _logger(std::move(logger))
{
    if (nullptr == _host_name_resolver) {
        _host_name_resolver = std::move(std::make_unique<fep3::native::HostNameResolver>(_logger));
    }

    _dds_service_update_handler = std::move(
        std::make_unique<DssServiceUpdateHandler>(std::move(_host_name_resolver), _logger));

    auto update_callback = [&](const DdsServiceDiscovery& arg1) { updateCallback(arg1); };

    try {
        _topic_handler = std::move(std::make_unique<fep3::native::DDSTopicHandler>(
            update_callback, "service_discovery", dds_sd_participant, DdsServiceDiscovery{}));
    }
    catch (std::exception& e) {
        using namespace std::literals::string_literals;
        _logger->logDebug("Exception during DDSTopicHandler creation "s + e.what());
    }
}

ServiceDiscTopicListener::~ServiceDiscTopicListener()
{
    _working = false;
    // delete the topic readers so that they do not trigger any new callbacks
    _topic_handler.reset();
}

void ServiceDiscTopicListener::updateCallback(const DdsServiceDiscovery& participant_data)
{
    if (_working) {
        _dds_service_update_handler->addWork(participant_data);
    }
}

std::vector<ServiceUpdateEvent> ServiceDiscTopicListener::getProcessedUpdates()
{
    return _dds_service_update_handler->getProcessedUpdates();
}

} // namespace native
} // namespace fep3
