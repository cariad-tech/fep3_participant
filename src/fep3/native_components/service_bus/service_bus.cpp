/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "service_bus.h"

#include "http_client.h"
#include "http_server.h"
#include "http_systemaccess.h"
#include "http_url_getter.h"
#include "service_bus_logger.hpp"

#include <fep3/components/configuration/configuration_service_intf.h>

#if defined(LSSDP_SERVICE_DISCOVERY)
    #include "service_discovery_factory_lssdp.h"
#elif defined(DDS_SERVICE_DISCOVERY)
    #include "service_discovery_factory_dds.h"
#else
    #error No Service Discovery implementation defined
#endif

#include <fep3/components/logging/easy_logger.h>

#include <cxx_url.h>

namespace fep3::native {

class ServiceBusDefaults : public fep3::base::SystemAccessBase::ISystemAccessBaseDefaultUrls {
public:
    std::string getDefaultSystemUrl() const override
    {
        return fep3::native::HttpSystemAccess::_default_url;
    }
    std::string getDefaultServerUrl() const override
    {
        return fep3::native::HttpServer::_use_default_url;
    }
};

fep3::Result ServiceBus::createSystemAccess(const std::string& system_name,
                                            const std::string& system_url,
                                            bool set_as_default)
{
    if (_locked) {
        SB_LOG_AND_RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            "Can not create system access. Invalid state for creation of '%s' on '%s'",
            system_name.c_str(),
            system_url.c_str());
    }

    auto server_found = getSystemAccessCatelyn(system_name);
    if (server_found) {
        SB_LOG_AND_RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            "Can not create system access point '%s'. System name '%s' already exists",
            system_name.c_str(),
            system_name.c_str());
    }
    std::shared_ptr<base::SystemAccessBase::ISystemAccessBaseDefaultUrls>
        service_bus_system_default = std::make_shared<ServiceBusDefaults>();
    std::string used_system_url;
    try {
        fep3::Result url_scheme_ok;
        // c++17 -> auto [url_scheme_ok, used_system_url] =
        // fep3::helper::getSystemUrl(system_url, service_bus_system_default.get());
        std::tie(url_scheme_ok, used_system_url) =
            fep3::helper::getSystemUrl(system_url, *service_bus_system_default.get());
        if (!url_scheme_ok) {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                                     "Can not create system access '%s'. This service bus does "
                                     "only support 'http' protocol, but it is called with '%s'",
                                     system_name.c_str(),
                                     system_url.c_str());
        }
    }
    catch (const std::exception& exc) {
        SB_LOG_AND_RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            "Can not create system access '%s'. url '%s' is not well formed. %s",
            system_name.c_str(),
            system_url.c_str(),
            exc.what());
    }

    std::shared_ptr<HttpSystemAccess> system_access = {};
    try {
        system_access = std::make_shared<HttpSystemAccess>(system_name,
                                                           used_system_url,
                                                           service_bus_system_default,
                                                           _logger_proxy,
                                                           _service_discovery_factory);

        auto components = _components.lock();
        if (!components ||
            !system_access->initLogger(*components, "http_system_access.service_bus.component")) {
            system_access->initLogger(_logger_proxy);
        }
    }
    catch (const std::exception& ex) {
        SB_LOG_AND_RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY,
                                            "Can not create system access '%s' - %s : %s",
                                            system_name.c_str(),
                                            used_system_url.c_str(),
                                            ex.what());
    }
    _system_accesses.push_back(system_access);

    FEP3_ARYA_LOGGER_LOG_DEBUG(
        _logger_proxy,
        a_util::strings::format("Created system access, system name: %s, url %s",
                                system_name.c_str(),
                                used_system_url.c_str()));

    if (set_as_default) {
        _default_system_access = system_access;
    }

    return {};
}

fep3::Result ServiceBus::releaseSystemAccess(const std::string& system_name)
{
    if (_locked) {
        SB_LOG_AND_RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_STATE,
            "Can not release system access '%s'. service bus locked",
            system_name.c_str());
    }
    for (decltype(_system_accesses)::iterator it = _system_accesses.begin();
         it != _system_accesses.end();
         ++it) {
        if ((*it)->getName() == system_name) {
            _system_accesses.erase(it);
            if (_default_system_access && _default_system_access->getName() == system_name) {
                _default_system_access.reset();
            }
            return {};
        }
    }
    SB_LOG_AND_RETURN_ERROR_DESCRIPTION(
        ERR_INVALID_ARG, "Can not find system access '%s' to destroy it", system_name.c_str());
}

std::shared_ptr<fep3::IServiceBus::ISystemAccess> ServiceBus::getSystemAccessCatelyn(
    const std::string& system_name) const
{
    for (const auto& current_access: _system_accesses) {
        if (current_access->getName() == system_name) {
            return current_access;
        }
    }

    if (system_name.empty()) {
        return getDefaultAccess();
    }

    return {};
}

std::shared_ptr<fep3::IServiceBus::ISystemAccess> ServiceBus::getDefaultAccess() const
{
    return _default_system_access;
}

void ServiceBus::lock()
{
    _locked = true;
    for (auto& sys_access: _system_accesses) {
        sys_access.get()->lock();
    }
}

void ServiceBus::unlock()
{
    for (auto& sys_access: _system_accesses) {
        sys_access.get()->unlock();
    }
    _locked = false;
}

ServiceBus::ServiceBus(std::shared_ptr<fep3::ILogger> logger)
    : _logger_proxy(std::make_shared<LoggerProxy>(logger)),
      _service_discovery_factory(std::make_shared<ServiceDiscoveryFactory>())
{
    _locked = false;
}

ServiceBus::~ServiceBus()
{
}

std::shared_ptr<fep3::arya::IServiceBus::ISystemAccess> ServiceBus::getSystemAccess(
    const std::string& system_name) const
{
    return getSystemAccessCatelyn(system_name);
}

std::shared_ptr<IServiceBus::IParticipantServer> ServiceBus::getServer() const
{
    auto system_access = getDefaultAccess();
    if (system_access) {
        return system_access->getServer();
    }
    return {};
}

std::shared_ptr<IServiceBus::IParticipantRequester> ServiceBus::getRequester(
    const std::string& far_server_name) const
{
    auto system_access = getDefaultAccess();
    if (system_access) {
        auto res = system_access->getRequester(far_server_name);
        if (res == nullptr) {
            using namespace std::string_literals;
            _logger_proxy->logError("Can not find far server: "s + far_server_name);
        }
        return res;
    }
    return {};
}

std::shared_ptr<IServiceBus::IParticipantRequester> ServiceBus::getRequester(
    const std::string& far_server_address, bool) const
{
    try {
        fep3::helper::Url url_check(far_server_address);
        auto scheme = url_check.scheme();
        if (scheme != "http") {
            _logger_proxy->logError("could not create requester for the " + far_server_address +
                                    ": invalid protocol. only http supported.");
        }
        else {
            return std::make_shared<HttpClientConnector>(far_server_address);
        }
    }
    catch (const std::exception& exc) {
        _logger_proxy->logError(std::string("could not create requester for the ") +
                                far_server_address + ": " + exc.what());
    }
    return {};
}

fep3::Result ServiceBus::create()
{
    auto components = _components.lock();
    if (components) {
        auto logging_service = components->getComponent<arya::ILoggingService>();
        if (logging_service) {
            auto logger = logging_service->createLogger("service_bus.component");
            _logger_proxy->setLogger(logger);
        }
        const auto configuration_service = components->getComponent<IConfigurationService>();
        if (configuration_service) {
            FEP3_RETURN_IF_FAILED(_configuration.initConfiguration(*configuration_service));
        }
    }
    lock();

    return {};
}

fep3::Result ServiceBus::destroy()
{
    _logger_proxy->setLogger(nullptr);
    unlock();
    return {};
}

} // namespace fep3::native
