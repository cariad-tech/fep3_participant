/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <service_update_event_sink_proxy.h>

namespace fep3::native {

UpdateEventSinkProxy::UpdateEventSinkProxy(fep3::IServiceBus::IServiceUpdateEventSink* sink)
    : _sink(sink)
{
}

void UpdateEventSinkProxy::run(const fep3::IServiceBus::ServiceUpdateEvent& service_update_event)
{
    const std::lock_guard lock(_mtx);
    if (_active) {
        _sink->updateEvent(service_update_event);
    }
}

UpdateEventSinkProxy::~UpdateEventSinkProxy()
{
    deactivate();
}

void UpdateEventSinkProxy::deactivate()
{
    const std::lock_guard lock(_mtx);
    _active = false;
}

bool UpdateEventSinkProxy::operator==(const fep3::IServiceBus::IServiceUpdateEventSink* lhs)
{
    return _sink == lhs;
}

} // namespace fep3::native
