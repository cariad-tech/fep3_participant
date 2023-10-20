/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "http_systemaccess.h"

#include "../../base/environment_variable/include/environment_variable.h"
#include "http_client.h"
#include "http_server.h"
#include "http_url_getter.h"
#include "service_update_sink_registry.h"

#include <a_util/strings.h>

#include <cxx_url.h>
#include <notification_waiting.h>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace fep3 {
namespace native {

constexpr const char* fep3_network_interface_env = "FEP3_NETWORK_INTERFACE";

struct ServiceVec {
public:
    void update(const fep3::native::ServiceUpdateEvent& update_event,
                const std::string& system_name,
                ServiceUpdateSinkRegistry& service_update_sink_registry)

    {
        // special system_name
        const bool search_all_systems =
            (system_name == fep3::IServiceBus::ISystemAccess::_discover_all_systems);
        // the unique service name will be server@system in FEP 3
        std::string received_service_name;
        std::string received_system_name;

        auto service_at_system = a_util::strings::split(update_event._service_name, "@", true);
        if (service_at_system.size() >= 2) {
            received_service_name = service_at_system[0];
            received_system_name = service_at_system[1];
        }

        if (search_all_systems) {
            updateServices(
                update_event._service_name, update_event._event_id, update_event._host_url);
            service_update_sink_registry.updateEvent(
                fep3::IServiceBus::ServiceUpdateEvent{received_service_name,
                                                      received_system_name,
                                                      update_event._host_url,
                                                      update_event._event_id});
        }
        // we only have a look on the received_system_name if is equal to the system_name
        else if (system_name == received_system_name) {
            updateServices(received_service_name, update_event._event_id, update_event._host_url);
            service_update_sink_registry.updateEvent(
                fep3::IServiceBus::ServiceUpdateEvent{received_service_name,
                                                      received_system_name,
                                                      update_event._host_url,
                                                      update_event._event_id});
        }
        else {
            // this is a server belongs to another system
        }
    }

    void removeOldDevices()
    {
        std::unique_lock<std::recursive_mutex> lo(_my_mutex);
        decltype(_services)::const_iterator current = _services.cbegin();
        while (current != _services.cend()) {
            if ((system_clock::now() - current->second.first) > 20s) {
                _services.erase(current->first);
                current = _services.cbegin();
            }
            else {
                ++current;
            }
        }
    }

    std::multimap<std::string, std::string> getDiscoveredServices() const
    {
        std::multimap<std::string, std::string> result_map = {};

        {
            std::unique_lock<std::recursive_mutex> lo(_my_mutex);
            for (const auto& current: _services) {
                result_map.emplace(current.first, current.second.second);
            }
        }
        return result_map;
    }

private:
    void updateServices(const std::string& service_name,
                        fep3::IServiceBus::ServiceUpdateEventType event_id,
                        const std::string& host_url)
    {
        if (event_id == fep3::IServiceBus::ServiceUpdateEventType::notify_alive ||
            event_id == fep3::IServiceBus::ServiceUpdateEventType::response) {
            std::unique_lock<std::recursive_mutex> lo(_my_mutex);
            _services[service_name] = {system_clock::now(), host_url};
        }
        else if (event_id == fep3::IServiceBus::ServiceUpdateEventType::notify_byebye) {
            std::unique_lock<std::recursive_mutex> lo(_my_mutex);
            _services.erase(service_name);
        }
    }

    std::map<std::string, std::pair<system_clock::time_point, std::string>> _services;
    mutable std::recursive_mutex _my_mutex;
};

struct HttpSystemAccess::Impl {
    Impl() = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    // TODO: Make it more robust and return exceptions while init
    // If exception raises while loop the loop will immediately stop
    Impl(const std::string& system_url,
         const std::string& system_name,
         std::chrono::seconds interval,
         std::shared_ptr<ILogger> logger,
         std::shared_ptr<IServiceDiscoveryFactory> service_discovery_factory)
        : _system_name(system_name),
          _system_url(system_url),
          _interval(interval),
          _services(),
          _logger(std::move(logger)),
          _service_discovery_factory(std::move(service_discovery_factory))
    {
        startDiscovering();
    }
    ~Impl()
    {
        // we only wait if the service finder was created
        _discovery_wait.notify();
        if (_service_finder) {
            _service_finder->disableDiscovery();
            _stop_loop = true;
            if (_loop.joinable()) {
                _loop.join();
            }
        }
    }

    std::multimap<std::string, std::string> getCurrentlyDiscoveredServices()
    {
        return _services.getDiscoveredServices();
    }

    std::multimap<std::string, std::string> getDiscoveredServices(std::chrono::milliseconds timeout)
    {
        if (timeout.count() != 0) {
            // wait that at least one msearch is done
            if (timeout.count() <= 0) {
                timeout = 100ms;
            }
            // we do not care if we have a timeout or not we return the discovered
            _discovery_wait.waitForNotificationWithTimeout(timeout);
        }
        return getCurrentlyDiscoveredServices();
    }

    void startDiscovering()
    {
        if (!_system_url.empty()) {
            _service_finder = _service_discovery_factory->getServiceFinder(
                _logger,
                _system_url,
                HttpSystemAccess::getNetworkInterface(),
                FEP3_PARTICIPANT_LIBRARY_VERSION_ID,
                FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                HttpServer::_discovery_search_target);

            _stop_loop = false;
            _loop = std::move(std::thread([&] {
                try {
                    seconds send_msearch_interval = _interval;
                    auto last_time = std::chrono::system_clock::now();
                    // send search first

                    searchNow();
                    do {
                        auto now = system_clock::now();
                        bool m_search_now = ((now - last_time) >= send_msearch_interval);
                        if (m_search_now) {
                            last_time = now;
                            removeOldDevices();
                            searchNow();
                        }
                        checkForServices(1s);
                        std::this_thread::sleep_for(1ms);
                    } while (!_stop_loop);
                }
                catch (const std::exception& ex) {
                    _logger->logInfo(ex.what());
                }
            }));
        }
    }

    fep3::Result registerUpdateEventSink(
        fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink)
    {
        return _service_update_sink_registry.registerUpdateEventSink(update_event_sink);
    }

    fep3::Result deregisterUpdateEventSink(
        fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink)
    {
        return _service_update_sink_registry.deregisterUpdateEventSink(update_event_sink);
    }

private:
    void searchNow()
    {
        if (!_service_finder->sendMSearch()) {
            _logger->logInfo(_service_finder->getLastSendErrors());
        }
    }
    void checkForServices(std::chrono::seconds how_long)
    {
        _service_finder->checkForServices(
            [this](const fep3::native::ServiceUpdateEvent& update_event) {
                _services.update(update_event, _system_name, _service_update_sink_registry);
            },
            how_long);
    }
    void removeOldDevices()
    {
        _services.removeOldDevices();
    }

private:
    mutable std::unique_ptr<fep3::native::IServiceFinder> _service_finder;
    std::atomic<bool> _stop_loop;
    ServiceVec _services;
    std::string _system_name;
    std::string _system_url;
    std::chrono::seconds _interval;
    std::thread _loop;
    NotificationWaiting _discovery_wait;
    std::shared_ptr<ILogger> _logger;
    std::shared_ptr<IServiceDiscoveryFactory> _service_discovery_factory;
    ServiceUpdateSinkRegistry _service_update_sink_registry;
};

HttpSystemAccess::HttpSystemAccess(
    const std::string& system_name,
    const std::string& system_url,
    const std::shared_ptr<ISystemAccessBaseDefaultUrls>& defaults,
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IServiceDiscoveryFactory> service_discovery_factory)
    : SystemAccessBase(system_name, system_url, defaults),
      _service_discovery_factory(service_discovery_factory),
      _logger(logger),
      _impl(std::make_unique<Impl>(system_url,
                                   system_name,
                                   std::chrono::seconds(5),
                                   std::move(logger),
                                   _service_discovery_factory))
{
}

HttpSystemAccess::~HttpSystemAccess()
{
}

std::shared_ptr<IServiceBus::IParticipantServer> HttpSystemAccess::createAServer(
    const std::string& server_name, const std::string& server_url, bool discovery_active)
{
    std::shared_ptr<arya::IRPCServer> server_to_set;
    {
        std::string _used_server_url;
        try {
            _used_server_url =
                fep3::helper::getServerUrl(server_url, *SystemAccessBase::getDefaultUrls().get());
        }
        catch (const fep3::helper::Url::parse_error& exc) {
            throw std::runtime_error(a_util::strings::format(
                "service bus: can not create server '%s'. url '%s' is not well formed. %s",
                server_name.c_str(),
                server_url.c_str(),
                exc.what()));
        }
        if (_used_server_url.empty()) {
            throw std::runtime_error(
                a_util::strings::format("service bus: can not create server '%s' with url '%s' ",
                                        server_name.c_str(),
                                        server_url.c_str()));
        }
        auto server = std::make_shared<HttpServer>(server_name,
                                                   _used_server_url,
                                                   getName(),
                                                   getUrl(),
                                                   _logger,
                                                   _service_discovery_factory,
                                                   discovery_active);
        // very important to call!!
        server->initialize();
        return server;
    }
}

std::shared_ptr<IServiceBus::IParticipantRequester> HttpSystemAccess::createARequester(
    const std::string& far_server_name, const std::string& far_server_url) const
{
    try {
        fep3::helper::Url url_check(far_server_url);
        auto scheme = url_check.scheme();
        if (scheme != "http") {
            throw std::runtime_error(
                a_util::strings::format("service bus: can not create server '%s'. Server does only "
                                        "support http, but it is called with '%s'",
                                        far_server_name.c_str(),
                                        far_server_url.c_str()));
        }
        else {
            std::string use_url = far_server_url;
            if (url_check.host() == "0.0.0.0") {
                use_url = "http://127.0.0.1:" + url_check.port();
            }
            return std::make_shared<HttpClientConnector>(use_url);
        }
    }
    catch (const std::exception& exc) {
        throw std::runtime_error(a_util::strings::format(
            "service bus: can not create server '%s'. url '%s' is not well formed. %s",
            far_server_name.c_str(),
            far_server_url.c_str(),
            exc.what()));
    }
}

std::multimap<std::string, std::string> HttpSystemAccess::getDiscoveredServices(
    std::chrono::milliseconds millisec) const
{
    return _impl->getDiscoveredServices(millisec);
}

std::multimap<std::string, std::string> HttpSystemAccess::getCurrentlyDiscoveredServices() const
{
    return _impl->getCurrentlyDiscoveredServices();
}

const std::string HttpSystemAccess::getNetworkInterface()
{
    const auto& interface_env = environment_variable::get(fep3_network_interface_env);
    if (interface_env.has_value()) {
        return interface_env.value();
    }
    else {
        return "";
    }
}

fep3::Result HttpSystemAccess::registerUpdateEventSink(
    fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink)
{
    return _impl->registerUpdateEventSink(update_event_sink);
}

fep3::Result HttpSystemAccess::deregisterUpdateEventSink(
    fep3::IServiceBus::IServiceUpdateEventSink* update_event_sink)
{
    return _impl->deregisterUpdateEventSink(update_event_sink);
}

} // namespace native
} // namespace fep3
