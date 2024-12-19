/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "service_discovery_factory_intf.h"

#include <fep3/components/service_bus/system_access_base.hpp>

namespace fep3 {

namespace native {

class LoggerProxy;
class IServiceDiscoveryFactory;

class HttpSystemAccess : public fep3::base::SystemAccessBase {
public:
    explicit HttpSystemAccess(const std::string& system_name,
                              const std::string& system_url,
                              const std::shared_ptr<ISystemAccessBaseDefaultUrls>& defaults,
                              std::shared_ptr<ILogger> startup_logger,
                              std::shared_ptr<IServiceDiscoveryFactory> service_discovery_factory);
    ~HttpSystemAccess();

    std::shared_ptr<IServiceBus::IParticipantServer> createAServer(const std::string& server_name,
                                                                   const std::string& server_url,
                                                                   bool discovery_active) override;

    std::shared_ptr<IServiceBus::IParticipantRequester> createARequester(
        const std::string& far_server_name, const std::string& far_server_url) const override;

    std::multimap<std::string, std::string> getDiscoveredServices(
        std::chrono::milliseconds timeout) const override;
    std::multimap<std::string, std::string> getCurrentlyDiscoveredServices() const override;

    fep3::Result registerUpdateEventSink(
        fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink) override;
    fep3::Result deregisterUpdateEventSink(
        fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink) override;

    static constexpr const char* const _default_url = "http://230.230.230.1:9990";

    static const std::string getNetworkInterface();

private:
    struct Impl;
    std::shared_ptr<IServiceDiscoveryFactory> _service_discovery_factory;
    std::shared_ptr<ILogger> _logger;
    std::unique_ptr<Impl> _impl;
};

} // namespace native
} // namespace fep3
