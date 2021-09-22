/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#pragma once

#include <string>

#include <fep3/components/base/component.h>
#include <fep3/components/experimental/health_service_intf.h>
#include <fep3/rpc_services/health/health_service_stub.h>
#include <fep3/rpc_services/health/health_service_rpc_intf_def.h>
#include <fep3/components/service_bus/rpc/fep_rpc.h>
#include "fep3/components/service_bus/service_bus_intf.h"
#include "fep3/components/logging/easy_logger.h"

namespace fep3
{
namespace native
{

class HealthService;

class RPCHealthService : public rpc::RPCService<fep3::rpc_stubs::RPCHealthServiceStub, fep3::experimental::IRPCHealthServiceDef>
{
public:
    explicit RPCHealthService(HealthService& health_service) : _health_service(health_service) {}

    // Inherited via RPCService
    int getHealth() override;
    Json::Value resetHealth(const std::string& message) override;

private:
    HealthService& _health_service;
};

class HealthService
    : public fep3::base::Component<fep3::experimental::IHealthService>
    , public base::EasyLogging
{
public:
    HealthService();
    HealthService(const HealthService&) = delete;
    HealthService(HealthService&&) = delete;
    HealthService operator=(const HealthService&) = delete;
    HealthService operator=(HealthService&&) = delete;
    ~HealthService() = default;

    // Inherited via IComponent
    fep3::Result create() override;
    fep3::Result destroy() override;

    // Inherited via IHealthService
    Result setHealthToError(const std::string& message) override;
    Result resetHealth(const std::string& message) override;
    experimental::HealthState getHealth() override;

private:
    fep3::Result setupLogger(const IComponents& components);
    fep3::Result setupRPCHealthService(const IComponents& components);
    fep3::Result unregisterRPCHealthService(const IComponents& components) const;

private:
    experimental::HealthState _health_state;
    std::shared_ptr<IRPCServer::IRPCService> _rpc_service{ nullptr };
};

} // namespace native
} // namespace fep3
