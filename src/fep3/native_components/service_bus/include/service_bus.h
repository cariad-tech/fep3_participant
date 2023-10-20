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

#include "service_bus_configuration.h"

#include <fep3/components/base/component.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>

namespace fep3 {
// not yet sure if this is really necessary within the implementation
namespace native {
class LoggerProxy;
// servicebus implementation supporting the arya service bus
class ServiceBus
    : public fep3::base::Component<fep3::arya::IServiceBus, fep3::catelyn::IServiceBus> {
public:
    ServiceBus(std::shared_ptr<fep3::ILogger> logger = nullptr);
    virtual ~ServiceBus();
    ServiceBus(const ServiceBus&) = delete;
    ServiceBus(ServiceBus&&) = delete;
    ServiceBus& operator=(const ServiceBus&) = delete;
    ServiceBus& operator=(ServiceBus&&) = delete;

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
    class Impl;
    std::shared_ptr<LoggerProxy> _logger_proxy;
    std::unique_ptr<Impl> _impl;
    ServiceBusConfiguration _configuration;
};
} // namespace native
} // namespace fep3
