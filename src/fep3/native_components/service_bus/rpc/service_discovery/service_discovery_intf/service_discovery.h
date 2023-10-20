#pragma once

#include "conversion_function.h"

namespace fep3 {
namespace native {

template <typename T>
class ServiceDiscovery : public IServiceDiscovery {
public:
    template <typename... Args>
    ServiceDiscovery(Args&&... args) : _sd(std::forward<Args>(args)...)
    {
    }

    std::string getLastSendErrors() const override
    {
        return _sd.getLastSendErrors();
    }

    bool sendNotifyAlive() override
    {
        return _sd.sendNotifyAlive();
    }

    bool sendNotifyByeBye() override
    {
        return _sd.sendNotifyByeBye();
    }

    bool checkForMSearchAndSendResponse(std::chrono::milliseconds timeout) override
    {
        return _sd.checkForMSearchAndSendResponse(timeout);
    }

private:
    T _sd;
};

template <typename T>
class ServiceFinder : public IServiceFinder {
public:
    template <typename... Args>
    ServiceFinder(Args&&... args) : _sf{std::forward<Args>(args)...}
    {
    }

    std::string getLastSendErrors() const override
    {
        return _sf.getLastSendErrors();
    }

    bool sendMSearch() override
    {
        return _sf.sendMSearch();
    }

    bool checkForServices(const std::function<void(const ServiceUpdateEvent&)>& update_callback,
                          std::chrono::milliseconds timeout) override
    {
        return _sf.checkForServices(
            [&](const typename ConversionFunction<T>::UpdateEvent& update_event) {
                update_callback(ConversionFunction<T>::convertEvent(update_event));
            },
            timeout);
    }

    void disableDiscovery() override
    {
        _sf.disableDiscovery();
    }

private:
    T _sf;
};

} // namespace native
} // namespace fep3
