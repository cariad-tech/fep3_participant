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

#include "service_update_sink_registry.h"

namespace fep3::native {

#ifdef __linux__
    #pragma GCC diagnostic push
    // auto [_, result] causes an unused variable warning in gcc7 resulting in compile error
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

fep3::Result ServiceUpdateSinkRegistry::registerUpdateEventSink(
    fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink)
{
    const std::lock_guard<std::mutex> lock(_mtx);
    [[maybe_unused]] auto [_, result] = _service_update_sinks.emplace(update_event_sink);
    if (!result) {
        RETURN_ERROR_DESCRIPTION(ERR_FAILED, "ServiceUpdateEventSink  pointer already registered");
    }
    else {
        return {};
    }
}

#ifdef __linux__
    #pragma GCC diagnostic pop
#endif

fep3::Result ServiceUpdateSinkRegistry::deregisterUpdateEventSink(
    fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink)
{
    const std::lock_guard<std::mutex> lock(_mtx);
    auto elements_erased = _service_update_sinks.erase(update_event_sink);
    if (elements_erased == 0) {
        RETURN_ERROR_DESCRIPTION(ERR_FAILED, "ServiceUpdateEventSink  pointer already registered");
    }
    else {
        return {};
    }
}

void ServiceUpdateSinkRegistry::updateEvent(
    const fep3::IServiceBus::ServiceUpdateEvent& service_update_event)
{
    const std::lock_guard<std::mutex> lock(_mtx);

    for (auto& service_update_sink: _service_update_sinks) {
        service_update_sink->updateEvent(service_update_event);
    }
}

} // namespace fep3::native
