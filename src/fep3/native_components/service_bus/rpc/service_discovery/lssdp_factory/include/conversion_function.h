#pragma once

#include "service_discovery_intf.h"

#include <lssdpcpp.h>

namespace fep3 {
namespace native {

template <typename T>
struct ConversionFunction {
};

template <>
struct ConversionFunction<lssdp::ServiceFinder> {
    using UpdateEvent = lssdp::ServiceFinder::ServiceUpdateEvent;
    static fep3::native::ServiceUpdateEvent convertEvent(const UpdateEvent& update_event)
    {
        fep3::native::ServiceUpdateEvent converted_event;
        converted_event._host_url = update_event._service_description.getLocationURL();
        converted_event._service_name = update_event._service_description.getUniqueServiceName();

        switch (update_event._event_id) {
        case UpdateEvent::notify_alive:
            converted_event._event_id = fep3::IServiceBus::ServiceUpdateEventType::notify_alive;
            break;
        case UpdateEvent::notify_byebye:
            converted_event._event_id = fep3::IServiceBus::ServiceUpdateEventType::notify_byebye;
            break;
        case UpdateEvent::response:
            converted_event._event_id = fep3::IServiceBus::ServiceUpdateEventType::response;
            break;
        }

        return converted_event;
    }
};

} // namespace native
} // namespace fep3
