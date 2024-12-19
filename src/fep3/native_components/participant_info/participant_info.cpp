/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "participant_info.h"

#include <fep3/components/service_bus/service_bus_intf.h>

namespace fep3 {
namespace native {

class ParticipantInfo::Impl {
public:
    fep3::IServiceBus* _service_bus;
};

ParticipantInfo::ParticipantInfo() : _impl(std::make_unique<ParticipantInfo::Impl>())
{
}

ParticipantInfo::~ParticipantInfo() = default;

fep3::Result ParticipantInfo::create()
{
    const auto components = _components.lock();
    if (!components) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set");
    }

    _impl->_service_bus = components->getComponent<fep3::IServiceBus>();

    if (!_impl->_service_bus) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "Can not get servicebus interface");
    }

    return {};
}

fep3::Result ParticipantInfo::destroy()
{
    _impl->_service_bus = nullptr;
    return {};
}

std::string ParticipantInfo::getName() const
{
    if (auto server = _impl->_service_bus->getServer()) {
        return server->getName();
    }
    return {};
}

std::string ParticipantInfo::getSystemName() const
{
    if (auto system_access = _impl->_service_bus->getSystemAccess()) {
        return system_access->getName();
    }
    return {};
}

} // namespace native
} // namespace fep3