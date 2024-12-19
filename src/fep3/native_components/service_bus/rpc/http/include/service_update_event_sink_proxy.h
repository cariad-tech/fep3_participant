/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/service_bus/service_bus_intf.h>

#include <memory>
#include <mutex>

namespace fep3::native {

struct UpdateEventSinkProxy : public std::enable_shared_from_this<UpdateEventSinkProxy> {
    UpdateEventSinkProxy(fep3::IServiceBus::IServiceUpdateEventSink* sink);

    void run(const fep3::IServiceBus::ServiceUpdateEvent& service_update_event);

    ~UpdateEventSinkProxy();

    void deactivate();

    bool operator==(const fep3::IServiceBus::IServiceUpdateEventSink* lhs);

    UpdateEventSinkProxy(UpdateEventSinkProxy&&) = default;

    UpdateEventSinkProxy& operator=(UpdateEventSinkProxy&&) = default;

    UpdateEventSinkProxy(const UpdateEventSinkProxy&) = delete;

    UpdateEventSinkProxy& operator=(const UpdateEventSinkProxy&) = delete;

private:
    fep3::IServiceBus::IServiceUpdateEventSink* _sink;
    std::recursive_mutex _mtx;
    bool _active = true;
};

} // namespace fep3::native