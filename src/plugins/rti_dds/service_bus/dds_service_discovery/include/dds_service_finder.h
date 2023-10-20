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
#pragma once

#include "../../../../../fep3/native_components/service_bus/rpc/service_discovery/service_discovery_intf/logger_proxy.h"
#include "../../../../../fep3/native_components/service_bus/rpc/service_discovery/service_discovery_intf/service_discovery_intf.h"

namespace fep3 {
namespace native {
class ServiceDiscTopicListener;
class DdsSdParticipant;

class ServiceFinderDDS {
public:
    ServiceFinderDDS(std::shared_ptr<ILogger> logger, const DdsSdParticipant& dds_sd_participant);
    ~ServiceFinderDDS();
    bool checkForServices(const std::function<void(const ServiceUpdateEvent&)>& update_callback,
                          std::chrono::milliseconds timeout);
    bool sendMSearch();
    std::string getLastSendErrors() const;
    void disableDiscovery();

    ServiceFinderDDS(ServiceFinderDDS&&) = delete;
    ServiceFinderDDS& operator=(ServiceFinderDDS&&) = delete;
    ServiceFinderDDS& operator=(const ServiceFinderDDS&) = delete;

private:
    bool pollServiceDiscovery(const std::function<void(const ServiceUpdateEvent&)>& update_callback,
                              std::chrono::milliseconds interval,
                              std::chrono::milliseconds duration);

    std::shared_ptr<ILogger> _logger;
    std::unique_ptr<ServiceDiscTopicListener> _service_disc_topic_listener;
    std::atomic<bool> _discovery_active{true};
    const std::chrono::milliseconds _default_poll_interval{100};
};

} // namespace native
} // namespace fep3
