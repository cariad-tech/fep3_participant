/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "service_bus_configuration.h"

#include <fep3/components/base/component.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>

namespace fep3::native {

class HttpSystemAccess;
class IServiceDiscoveryFactory;
class LoggerProxy;

class ServiceBus
    : public fep3::base::Component<fep3::arya::IServiceBus, fep3::catelyn::IServiceBus> {
public:
    ServiceBus(std::shared_ptr<fep3::ILogger> logger = nullptr);
    virtual ~ServiceBus();
    ServiceBus(const ServiceBus&) = delete;
    ServiceBus(ServiceBus&&) = delete;
    ServiceBus& operator=(const ServiceBus&) = delete;
    ServiceBus& operator=(ServiceBus&&) = delete;

private:
    std::shared_ptr<fep3::IServiceBus::ISystemAccess> getDefaultAccess() const;
    void lock();
    void unlock();

public: // the arya ServiceBus interface
    fep3::Result createSystemAccess(const std::string& system_name,
                                    const std::string& system_discovery_url,
                                    bool is_default = false) override final;
    fep3::Result releaseSystemAccess(const std::string& system_name) override final;

    std::shared_ptr<IParticipantServer> getServer() const override final;
    std::shared_ptr<IParticipantRequester> getRequester(
        const std::string& far_server_name) const override final;

    std::shared_ptr<fep3::arya::IServiceBus::ISystemAccess> getSystemAccess(
        const std::string& system_name) const override final;
    std::shared_ptr<ISystemAccess> getSystemAccessCatelyn(
        const std::string& system_name = std::string()) const override final;

    std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_server_url,
                                                        bool is_url) const override final;

public: // override base::Component
    fep3::Result create() override;
    fep3::Result destroy() override;

private:
    ServiceBusConfiguration _configuration;
    std::vector<std::shared_ptr<HttpSystemAccess>> _system_accesses;
    std::shared_ptr<fep3::IServiceBus::ISystemAccess> _default_system_access;
    std::atomic<bool> _locked;
    std::shared_ptr<LoggerProxy> _logger_proxy;
    std::shared_ptr<IServiceDiscoveryFactory> _service_discovery_factory;
};

} // namespace fep3::native
