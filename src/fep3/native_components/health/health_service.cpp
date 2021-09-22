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


#include "health_service.h"

#include <rpc/rpc.h>

namespace fep3
{
namespace native
{

int RPCHealthService::getHealth()
{
    return static_cast<int>(_health_service.getHealth());
}

Json::Value RPCHealthService::resetHealth(const std::string& message)
{
    return ::rpc::cJSONConversions::result_to_json(_health_service.resetHealth(message));
}

HealthService::HealthService()
    : _health_state(experimental::HealthState::ok)
{
}

fep3::Result HealthService::create()
{
    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Component pointer is invalid");
    }

    FEP3_RETURN_IF_FAILED(setupRPCHealthService(*components));
    FEP3_RETURN_IF_FAILED(setupLogger(*components));

    return {};
}

fep3::Result HealthService::destroy()
{
    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Component pointer is invalid");
    }

    FEP3_RETURN_IF_FAILED(unregisterRPCHealthService(*components));

    return {};
}

Result HealthService::setHealthToError(const std::string& message)
{
    FEP3_LOG_DEBUG(a_util::strings::format("Participant health state set to 'error' due to: '%s'"
                                           , message.c_str()));

    _health_state = experimental::HealthState::error;

    return {};
}

Result HealthService::resetHealth(const std::string& message)
{
    FEP3_LOG_DEBUG(a_util::strings::format("Participant health state reset to 'ok' due to: '%s'"
                                           , message.c_str()));

    _health_state = experimental::HealthState::ok;

    return {};
}

experimental::HealthState HealthService::getHealth()
{
    return _health_state;
}

fep3::Result HealthService::setupLogger(const IComponents& components)
{
    FEP3_RETURN_IF_FAILED(initLogger(components, "health_service.component"));

    return {};
}

fep3::Result HealthService::setupRPCHealthService(const IComponents& components)
{
    const auto service_bus = components.getComponent<fep3::IServiceBus>();
    if (!service_bus)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }

    auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }

    if (!_rpc_service)
    {
        _rpc_service = std::make_shared<RPCHealthService>(*this);
        FEP3_RETURN_IF_FAILED(rpc_server->registerService(experimental::IRPCHealthServiceDef::getRPCDefaultName(),
            _rpc_service));
    }

    return {};
}

fep3::Result HealthService::unregisterRPCHealthService(const IComponents& components) const
{
    const auto* service_bus = components.getComponent<fep3::IServiceBus>();
    if (service_bus)
    {
        auto rpc_server = service_bus->getServer();
        if (rpc_server)
        {
            rpc_server->unregisterService(experimental::IRPCHealthServiceDef::getRPCDefaultName());
        }
    }

    return {};
}

} // namespace native
} // namespace fep3
