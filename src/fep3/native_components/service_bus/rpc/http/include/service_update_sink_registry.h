/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2022 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/components/service_bus/service_bus_intf.h>

#include <mutex>
#include <set>

namespace fep3::native {

class ServiceUpdateSinkRegistry {
public:
    fep3::Result registerUpdateEventSink(
        fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink);

    fep3::Result deregisterUpdateEventSink(
        fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink);

    void updateEvent(const fep3::IServiceBus::ServiceUpdateEvent& service_update_event);

private:
    std::set<fep3::IServiceBus::IServiceUpdateEventSink*> _service_update_sinks;
    std::mutex _mtx;
};

} // namespace fep3::native
