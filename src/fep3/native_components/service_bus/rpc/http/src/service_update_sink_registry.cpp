/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "service_update_sink_registry.h"

#include <service_update_event_sink_proxy.h>

namespace fep3::native {

#ifdef __linux__
    #pragma GCC diagnostic push
    // auto [_, result] causes an unused variable warning in gcc7 resulting in compile error
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

ServiceUpdateSinkRegistry::ServiceUpdateSinkRegistry()
{
    _thread_pool.start();
}

ServiceUpdateSinkRegistry::~ServiceUpdateSinkRegistry()
{
    // we are sure that after stop returns, no further calls to _service_update_sinks
    // are made
    _service_update_sinks.clear();
    _thread_pool.stop();
}

fep3::Result ServiceUpdateSinkRegistry::registerUpdateEventSink(
    fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink)
{
    const std::lock_guard<std::mutex> lock(_mtx);
    auto it = std::find_if(_service_update_sinks.begin(),
                           _service_update_sinks.end(),
                           [&](const std::shared_ptr<UpdateEventSinkProxy>& ptr) {
                               return *ptr == update_event_sink;
                           });

    if (it == _service_update_sinks.end()) {
        _service_update_sinks.emplace_back(
            std::make_shared<UpdateEventSinkProxy>(update_event_sink));
        return {};
    }
    else {
        RETURN_ERROR_DESCRIPTION(ERR_FAILED,
                                 "ServiceUpdateEventSink  pointer already registered found");
    }
}

#ifdef __linux__
    #pragma GCC diagnostic pop
#endif

fep3::Result ServiceUpdateSinkRegistry::deregisterUpdateEventSink(
    fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink)
{
    const std::lock_guard<std::mutex> lock(_mtx);
    auto it = std::find_if(_service_update_sinks.begin(),
                           _service_update_sinks.end(),
                           [&](const std::shared_ptr<UpdateEventSinkProxy>& ptr) {
                               return *ptr == update_event_sink;
                           });

    if (it == _service_update_sinks.end()) {
        RETURN_ERROR_DESCRIPTION(ERR_FAILED, "ServiceUpdateEventSink  pointer not found");
    }
    else {
        (*it)->deactivate();
        _service_update_sinks.erase(it);
        return {};
    }
}

void ServiceUpdateSinkRegistry::updateEvent(
    const fep3::IServiceBus::ServiceUpdateEvent& service_update_event)
{
    const std::lock_guard<std::mutex> lock(_mtx);
    for (auto service_update_sink: _service_update_sinks) {
        // using a copy of the shared_ptr here
        // in case it is called after deregistering of the sink
        // the proxy will not call the IServiceUpdateEventSink pointer
        _thread_pool.post([service_update_event, service_update_sink]() {
            service_update_sink->run(service_update_event);
        });
    }
}

} // namespace fep3::native
