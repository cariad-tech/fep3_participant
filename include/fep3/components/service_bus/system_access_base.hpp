/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/service_bus/service_bus_intf.h>

#include <algorithm>

/**
 * the default timeout of the address discovery within the
 * fep3::base::SystemAccessBase::getRequester call
 */
#define FEP3_SERVICE_BUS_GET_REQUESTER_TIMEOUT std::chrono::milliseconds(1000)

namespace fep3 {
namespace base {
namespace arya {

/**
 * the default number of tries of the address discovery within the
 * fep3::base::SystemAccessBase::getRequester call
 */
constexpr int fep3_service_bus_get_requester_max_tries = 10;

/**
 * Helper base implementation class to create a ISystemAccess implementation
 * 3 methods to implement:
 * @li @c SystemAccessBase::createAServer
 * @li @c SystemAccessBase::createARequester
 * @li @c SystemAccessBase::getDiscoveredServices
 */
class SystemAccessBase : public fep3::catelyn::IServiceBus::ISystemAccess,
                         public fep3::base::arya::EasyLogging {
public:
    /**
     * Class interface to obtain certain default urls for system access and server
     * @see fep3::IServiceBus
     */
    class ISystemAccessBaseDefaultUrls {
    protected:
        /// DTOR
        ~ISystemAccessBaseDefaultUrls() = default;

    public:
        /**
         * retrieve the default URL for the system access
         * @see fep3::IServiceBus::createSystemAccess
         * @return std::string the default url for a system
         */
        virtual std::string getDefaultSystemUrl() const = 0;
        /**
         * retrieve the default URL for the server access
         * @see fep3::IServiceBus::ISystemAccess::createServer
         * @return std::string the default url for a server
         */
        virtual std::string getDefaultServerUrl() const = 0;
    };

protected:
    /**
     * CTOR
     * @param[in] system_name the systems name
     * @param[in] system_url the systems url
     * @param[in] default_urls the service bus implementation default urls ... implement @c
     * ISystemAccessBaseDefaultUrls
     */
    SystemAccessBase(const std::string& system_name,
                     const std::string& system_url,
                     const std::shared_ptr<ISystemAccessBaseDefaultUrls>& default_urls)
        : _system_name(system_name), _system_url(system_url), _access_default_urls(default_urls)
    {
        _locked = false;
    }

    /**
     * assignment CTOR
     */
    SystemAccessBase(const SystemAccessBase&) = delete;

    /**
     * assignment CTOR
     */
    SystemAccessBase(SystemAccessBase&&) = delete;

    /**
     * assignment operator
     * @return SystemAccessBase default return value of a assignment operator
     */
    SystemAccessBase& operator=(const SystemAccessBase&) = delete;

    /**
     * move operator
     * @return SystemAccessBase default return value of a move operator
     */
    SystemAccessBase& operator=(SystemAccessBase&&) = delete;

public:
    /**
     * @brief create the server
     *
     * @param server_name name of the server that appear in the system
     * @param server_url url of the server if necessary
     * @param discovery_active flag indicating if the server is discoverable or not
     * @return the created server. throw if error occurs.
     */
    virtual std::shared_ptr<fep3::arya::IServiceBus::IParticipantServer> createAServer(
        const std::string& server_name, const std::string& server_url, bool discovery_active) = 0;
    /**
     * @brief create the requester
     *
     * @param far_server_name name of the far server that appears in the same system
     * @param far_server_url url of the far server if necessary
     * @return the created server. throw if error occurs.
     */
    virtual std::shared_ptr<fep3::arya::IServiceBus::IParticipantRequester> createARequester(
        const std::string& far_server_name, const std::string& far_server_url) const = 0;

    /**
     * @brief retrieves a multimap with pairs of names of the server and their addresses
     *
     * @param timeout the time waiting for the discover message answers
     * @return the multimap with pairs of names of the server and their addresses
     */
    virtual std::multimap<std::string, std::string> getDiscoveredServices(
        std::chrono::milliseconds timeout) const = 0;

    /**
     * @brief retrieves a multimap with pairs of names of the curenntley discovered server and their
     * addresses does not wait any discovery to be performed as in @ref
     * fep3::base::arya::SystemAccessBase::getDiscoveredServices
     * @return the multimap with pairs of names of the server and their addresses
     */
    virtual std::multimap<std::string, std::string> getCurrentlyDiscoveredServices() const = 0;

public:
    /**
     * @copydoc fep3::catelyn::IServiceBus::ISystemAccess::createServer
     */
    fep3::Result createServer(const std::string& server_name,
                              const std::string& server_url,
                              bool discovery_active) override
    {
        if (_locked) {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "service bus: Can not create server. Invalid state for "
                                     "creation of '%s' while creating '%s' - %s",
                                     _system_name.c_str(),
                                     server_name.c_str(),
                                     server_url.c_str());
        }
        try {
            _server.reset();
            auto server = createAServer(server_name, server_url, discovery_active);

            FEP3_LOG_INFO(a_util::strings::format(
                "Created participant server %s with url %s which is%sdiscoverable",
                server_name.c_str(),
                server_url.c_str(),
                discovery_active ? " " : " not "));
            if (server) {
                _server = server;
            }
            else {
                RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED,
                                         "Could not create participant server %s with url %s",
                                         server_name.c_str(),
                                         server_url.c_str());
            }
            return {};
        }
        catch (const std::exception& ex) {
            RETURN_ERROR_DESCRIPTION(ERR_BAD_DEVICE,
                                     "Could not create participant server %s with url %s - %s",
                                     server_name.c_str(),
                                     server_url.c_str(),
                                     ex.what());
        }
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::createServer
     */
    fep3::Result createServer(const std::string& server_name,
                              const std::string& server_url) override
    {
        // keep old behavior
        if (_system_url.empty()) {
            return createServer(server_name, server_url, false);
        }
        else {
            return createServer(server_name, server_url, true);
        }
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::releaseServer
     */
    void releaseServer() override final
    {
        if (_locked) {
            return;
        }
        _server.reset();

        FEP3_LOG_INFO(a_util::strings::format("Released participant server %s with url %s",
                                              _server->getName().c_str(),
                                              _server->getUrl().c_str()));
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::getServer
     */
    std::shared_ptr<fep3::arya::IServiceBus::IParticipantServer> getServer() const override final
    {
        return _server;
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::getRequester
     */
    std::shared_ptr<fep3::arya::IServiceBus::IParticipantRequester> getRequester(
        const std::string& far_participant_name) const override final
    {
        std::string found_url;
        if (_server && far_participant_name == _server->getName()) {
            // stay local!
            // at least this server is in the system
            found_url = _server->getUrl();
        }
        else {
            // look for the requester without active discovering
            auto found_services = getCurrentlyDiscoveredServices();
            for (const auto& found_service: found_services) {
                if (found_service.first == far_participant_name) {
                    found_url = found_service.second;
                    break;
                }
            }
            // if it is still empty, discover it
            if (found_url.empty()) {
                for (int try_count = 0; try_count < fep3_service_bus_get_requester_max_tries;
                     ++try_count) {
                    found_services = discover(FEP3_SERVICE_BUS_GET_REQUESTER_TIMEOUT);
                    const auto it =
                        std::find_if(found_services.begin(),
                                     found_services.end(),
                                     [&far_participant_name](const auto& discovered_service) {
                                         return discovered_service.first == far_participant_name;
                                     });
                    if (it != found_services.end()) {
                        found_url = it->second;
                        break;
                    }
                }
            }
        }

        if (found_url.empty()) {
            FEP3_LOG_ERROR(
                a_util::strings::format("Could not find nor create a requester for participant %s",
                                        far_participant_name.c_str()));
            return {};
        }

        try {
            auto requester = createARequester(far_participant_name, found_url);
            FEP3_LOG_DEBUG(
                a_util::strings::format("Created requester for participant %s and url %s",
                                        far_participant_name.c_str(),
                                        found_url.c_str()));
            return requester;
        }
        catch (std::exception& exception) {
            FEP3_LOG_ERROR(a_util::strings::format("Exception caught while getting a requester for "
                                                   "participant %s. Exception message: %s",
                                                   far_participant_name.c_str(),
                                                   exception.what()));
            FEP3_LOG_ERROR(a_util::strings::format("Failed to get a requester for participant %s",
                                                   far_participant_name.c_str()));
            return {};
        }
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::discover
     */
    std::multimap<std::string, std::string> discover(
        std::chrono::milliseconds timeout) const override final
    {
        return getDiscoveredServices(timeout);
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::getName
     */
    std::string getName() const override final
    {
        return _system_name;
    }

    /**
     * @brief Get the Url of the system_access
     *
     * @return std::string the url as string
     */
    std::string getUrl() const
    {
        return _system_url;
    }

public:
    /**
     * @brief lock creation of the server
     */
    void lock()
    {
        _locked = true;
    }

    /**
     * @brief unlock creation of the server
     */
    void unlock()
    {
        _locked = false;
    }

protected:
    /**
     * @brief Get the object to retrieve the default url.
     *
     * @return std::shared_ptr<ISystemAccessBaseDefaultUrls> the default. see also CTOR.
     */
    std::shared_ptr<arya::SystemAccessBase::ISystemAccessBaseDefaultUrls> getDefaultUrls() const
    {
        return _access_default_urls;
    }

private:
    std::string _system_name;
    std::string _system_url;
    // the current server if created
    std::shared_ptr<fep3::arya::IServiceBus::IParticipantServer> _server;
    // the default url object
    std::shared_ptr<arya::SystemAccessBase::ISystemAccessBaseDefaultUrls> _access_default_urls;
    // locked server creation
    std::atomic<bool> _locked;
};
} // namespace arya
using arya::SystemAccessBase;
} // namespace base
} // namespace fep3
