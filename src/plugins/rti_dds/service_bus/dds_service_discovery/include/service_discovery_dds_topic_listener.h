/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "dds_service_discovery_topic_handler.h"
#include "dds_service_update_handler.h"

class ParticipantData;

namespace fep3 {
namespace native {

class IDSTopicHandler;
class IHostNameResolver;
class DdsSdParticipant;

class ServiceDiscTopicListener {
public:
    ServiceDiscTopicListener(std::shared_ptr<ILogger> logger,
                             const DdsSdParticipant& dds_sd_participant,
                             std::unique_ptr<IHostNameResolver> host_name_resolver = nullptr);

    std::vector<ServiceUpdateEvent> getProcessedUpdates();
    ~ServiceDiscTopicListener();
    ServiceDiscTopicListener(ServiceDiscTopicListener&&) = delete;
    ServiceDiscTopicListener& operator=(ServiceDiscTopicListener&&) = delete;
    ServiceDiscTopicListener& operator=(const ServiceDiscTopicListener&) = delete;

private:
    void updateCallback(const DdsServiceDiscovery&);

    std::unique_ptr<DDSTopicHandler> _topic_handler;
    std::unique_ptr<IHostNameResolver> _host_name_resolver;
    std::shared_ptr<ILogger> _logger;
    std::unique_ptr<DssServiceUpdateHandler> _dds_service_update_handler;
    std::atomic<bool> _working{true};
};

} // namespace native
} // namespace fep3
