/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/service_bus/service_bus_intf.h>

#include <mutex>
#include <set>
#include <threaded_executor.h>

namespace fep3::native {

struct UpdateEventSinkProxy;

class ServiceUpdateSinkRegistry {
public:
    ServiceUpdateSinkRegistry();
    ~ServiceUpdateSinkRegistry();
    fep3::Result registerUpdateEventSink(
        fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink);

    fep3::Result deregisterUpdateEventSink(
        fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink);

    void updateEvent(const fep3::IServiceBus::ServiceUpdateEvent& service_update_event);

private:
    std::vector<std::shared_ptr<UpdateEventSinkProxy>> _service_update_sinks;
    std::mutex _mtx;
    ThreadPoolExecutor _thread_pool;
};

} // namespace fep3::native
