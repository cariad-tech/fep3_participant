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



#include "participant_info.h"
#include <a_util/result.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/components/logging/logging_service_intf.h>

namespace fep3
{
namespace native
{

    class ParticipantInfo::Impl
    {
    public:
        fep3::IServiceBus* _service_bus;
    };


    ParticipantInfo::ParticipantInfo() : _impl(std::make_unique<ParticipantInfo::Impl>())
    {
    }

    fep3::Result ParticipantInfo::create()
    {
        const auto components = _components.lock();
        if (!components)
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set");
        }

        _impl->_service_bus = components->getComponent<fep3::IServiceBus>();

        if (!_impl->_service_bus)
        {
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
        if (auto server = _impl->_service_bus->getServer())
        {
            return server->getName();
        }
        return {};
    }

    std::string ParticipantInfo::getSystemName() const
    {
        if (auto system_access = _impl->_service_bus->getSystemAccess())
        {
            return system_access->getName();
        }
        return {};
    }

}
}