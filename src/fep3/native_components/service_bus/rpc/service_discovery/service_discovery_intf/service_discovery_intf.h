#pragma once

#include <fep3/components/service_bus/service_bus_intf.h>

#include <functional>

namespace fep3 {
namespace native {

struct ServiceUpdateEvent {
    /**
     * @brief The service information within the message
     * @remark The service description will usually not always completed.
     *         Depending on the message type the content of the message is set.
     *         But at least the *search_target (ST)* and the *unique_service_name (USN)* is set.
     */
    std::string _service_name;
    std::string _host_url;
    /**
     * @brief the event id code
     */
    fep3::IServiceBus::ServiceUpdateEventType _event_id;
};

class IServiceDiscovery {
public:
    virtual ~IServiceDiscovery() = default;
    virtual std::string getLastSendErrors() const = 0;
    virtual bool sendNotifyAlive() = 0;
    virtual bool sendNotifyByeBye() = 0;
    virtual bool checkForMSearchAndSendResponse(std::chrono::milliseconds timeout) = 0;
};

class IServiceFinder {
public:
    virtual ~IServiceFinder() = default;
    virtual std::string getLastSendErrors() const = 0;
    virtual bool sendMSearch() = 0;
    virtual bool checkForServices(
        const std::function<void(const ServiceUpdateEvent&)>& update_callback,
        std::chrono::milliseconds timeout) = 0;
    virtual void disableDiscovery() = 0;
};

} // namespace native
} // namespace fep3
