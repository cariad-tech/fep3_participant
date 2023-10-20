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

#pragma once

#include <fep3/components/base/component_iid.h>
#include <fep3/components/service_bus/rpc/rpc_intf.h>

/**
 * @brief The service bus main property tree entry node
 */
#define FEP3_SERVICE_BUS_CONFIG "service_bus"

/**
 * @brief The fep system initialize priority property name
 */
#define FEP3_SERVICE_BUS_INIT_PRIORITY "init_priority"

/**
 * @brief The fep system start priority property name
 */
#define FEP3_SERVICE_BUS_START_PRIORITY "start_priority"

/**
 * @brief The fep system initialize priority configuration node
 */
#define FEP3_SERVICE_BUS_INIT_PRIORITY_CONFIG                                                      \
    FEP3_SERVICE_BUS_CONFIG "/" FEP3_SERVICE_BUS_INIT_PRIORITY

/**
 * @brief The fep system start priority configuration node
 */
#define FEP3_SERVICE_BUS_START_PRIORITY_CONFIG                                                     \
    FEP3_SERVICE_BUS_CONFIG "/" FEP3_SERVICE_BUS_START_PRIORITY

/**
 * @brief Default value of initialize priority property.
 */
#define FEP3_SERVICE_BUS_INIT_PRIORITY_DEFAULT_VALUE 0

/**
 * @brief Default value of start priority property.
 */
#define FEP3_SERVICE_BUS_START_PRIORITY_DEFAULT_VALUE 0

#include <chrono>
#include <map>

namespace fep3 {
namespace arya {
/**
 * @brief Service Bus component interface definition
 */
class IServiceBus {
protected:
    /**
     * @brief DTOR
     * @note This DTOR is explicitly protected to prevent destruction via this interface.
     */
    ~IServiceBus() = default;

public:
    /**
     * @brief definition of the component interface identifier for the IServiceBus
     * @see IComponents
     */
    FEP_COMPONENT_IID("service_bus.arya.fep3.iid");

public:
    /**
     * @brief Alias class that represent a Participant in the System
     */
    using IParticipantServer = arya::IRPCServer;
    /**
     * @brief Alias class that represents the possibility to request service functionality of a far
     * participant
     */
    using IParticipantRequester = arya::IRPCRequester;
    /**
     * @brief the system access may represent one participant within one system.
     * A participant will only appear within this system if the @c createServer was called!
     */
    class ISystemAccess {
    protected:
        /// DTOR
        ~ISystemAccess() = default;

    public:
        /**
         * @brief create a server object
         * The server itself is one access point to the service bus, where services can be
         * registered and unregistered.
         *
         * @param[in] server_name name of the server (within FEP 3 this is the participant name to
         *                        appear) within one service bus instance this name has to be
         *                        unique. Usually the implementation should also prevent to create a
         *                        server with the same name twice within the discovery_network_url
         *                        (but this depends on the used protocol if this is possible)
         * @param[in] server_url address of the server. If environment variable
         *                       FEP3_SERVICEBUS_SERVER_URL is set, then it takes precedence over
         *                       the server_url argument. Example values of the environment variable
         *                       or argument:
         *                       @li this will be http://localhost:9090 or http://0.0.0.0:9090 to
         *                           appear in all networks
         *                       @li this will be http://192.168.1.2:9090 to appear only in
         *                           http://192.168.1.2.x networks
         *
         * @return if an error occurs the server object of the system access is empty
         * @retval ERR_NOERROR successfully created
         *
         * @remark if this function is called while a server was already created before, the current
         * server will be release. if creating of the new server fails the server object will be
         * empty.
         */
        virtual fep3::Result createServer(const std::string& server_name,
                                          const std::string& server_url) = 0;

        /**
         * @brief releases the server
         * every service connection is stopped!
         */
        virtual void releaseServer() = 0;

        /**
         * @brief get the participant server with the given name
         *
         * @return the server if created already
         * @retval null std::shared_ptr<IRPCServer>() no server created yet. call @c createServer
         * before!
         */
        virtual std::shared_ptr<IParticipantServer> getServer() const = 0;

        /**
         * @brief get a requester to request service call at the participant with @c
         * far_participant_name by within the system access
         * @remark the participant must belong to the same system otherwise no communication is
         * possible
         *
         * @param[in] far_participant_name name of the far server
         * @return a requester to request messages from
         * @retval null std::shared_ptr<IRPCRequester>() if far_participant_name address cannot be
         * retrieved
         */
        virtual std::shared_ptr<IParticipantRequester> getRequester(
            const std::string& far_participant_name) const = 0;

        /**
         * @brief discover servers on the given systems address and system name
         * @param[in] timeout the timeout waiting on the discover message answers
         *
         * @return returns a set of server name/server address pairs
         */
        virtual std::multimap<std::string, std::string> discover(
            std::chrono::milliseconds timeout) const = 0;

        /**
         * @brief retrieves the name of the system access (used as system_name)
         *
         * @return returns the name of the system access
         */
        virtual std::string getName() const = 0;

        /**
         * @brief default option for addresses within the functions:
         * @li ISystemAccess::discover
         * @li ISystemAccess::createServer
         */
        static constexpr const char* const _use_default_url = "use_default_url";

        /**
         * @brief default option for service_discovery of ALL System within the functions:
         * @li IServiceBus::createSystemAccess
         * @li will influence the result of ISystemAccess::discover
         */
        static constexpr const char* const _discover_all_systems = "fep3:search_all_systems";
    };

    /**
     * @brief create a system access point to create a server (to be part of the system) or to
     * discover and request within this system. The server itself is one access point to the service
     * bus, where services can be registered and unregistered to.
     *
     * @param[in] system_name the name of the system this server is belonging to
     *                        @li leave empty if there is no system this participant belongs to
     *                        @li set the name of the system. this usually is to discover only
     *                            servers with the same system name
     * @param[in] system_discovery_url the network address url to communicate with if discovery is
     *                                 used. If environment variable FEP3_SERVICEBUS_SYSTEM_URL is
     *                                 set, then it takes precedence over the system_discovery_url
     *                                 argument. The environment variable or argument are evaluated
     *                                 as follows:
     *                                 @li if is set to "use_default_url", the default multicast
     *                                     address http://230.230.230.1:9900 will be used
     *                                 @li for http we could set up i.e. a multicast address and a
     *                                     port, for example http:/230.230.230.1:9900
     *                                 @li for rti dds this argument is ignored and an error will be
     *                                     reported if it is not empty or not set to use_default_url
     *                                 @li if empty no service discovery will be performed. The
     *                                     service will not be able to discover other services. The
     *                                     service will (or not) be discoverable can be set in the
     *                                     @c createServer call.
     * @param[in] is_default create this access point as default system access to have easy access
     *                       via IServiceBus::getServer and IServiceBus::getRequester which is used
     *                       within the RPC implementation templates
     *
     * @retval ERR_NOERROR no error occurred
     * @return depending on the service bus solution it will return with an error. see details of
     * that error
     */
    virtual fep3::Result createSystemAccess(const std::string& system_name,
                                            const std::string& system_discovery_url,
                                            bool is_default = false) = 0;

    /**
     * @brief releases the system access with the given @p system_name
     *
     *
     * @param[in] system_name name of the system access
     * @retval ERR_NOERROR no error occurred
     */
    virtual fep3::Result releaseSystemAccess(const std::string& system_name) = 0;

    /**
     * @brief get the participants server at the default system access
     * @return the server if it does exist
     * @retval null std::shared_ptr<IRPCServer>() if the server is not set
     * @remark this will only return a valid value if a system access  is created with
     *         @c createSystemAccess(..., ..., true) and within this access the server was created
     */
    virtual std::shared_ptr<IParticipantServer> getServer() const = 0;

    /**
     * @brief get a requester to connect a @p far_participant_server_name within the default system
     * access depending on the implementation this is used only together with discovery switched on
     *
     * @param[in] far_participant_server_name name of the far server
     * @return a requester to request messages from
     * @retval null std::shared_ptr<IRPCRequester>() if it could not be created
     */
    virtual std::shared_ptr<IParticipantRequester> getRequester(
        const std::string& far_participant_server_name) const = 0;

    /**
     * @brief get the default server with the given name
     * @param[in] system_name name of the server or empty for default interface
     * @return the server if it does exist. will return default system access, if system_name is
     * empty.
     * @retval null std::shared_ptr<ISystemAccess>() the server if it does not exist
     */
    virtual std::shared_ptr<ISystemAccess> getSystemAccess(
        const std::string& system_name = std::string()) const = 0;

    /**
     * @brief get a requester to connect a server with the full URL
     *
     * @param[in] far_server_url address of the far server to get a request for
     * @param[in] is_url mark the function to be the is_url version (this param will not be used!)
     * @return a requester to request messages from
     * @retval null std::shared_ptr<IRPCRequester>() if it could not be created
     * @throws std::runtime_error if url is not supported
     * @throws other for url parse error
     */
    virtual std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_server_url,
                                                                bool is_url) const = 0;
};
} // namespace arya

namespace catelyn {
/**
 * @brief Service Bus component interface definition
 */
class IServiceBus : virtual public arya::IServiceBus {
public:
    /**
     * @brief definition of the component interface identifier for the IServiceBus
     * @see IComponents
     */
    FEP_COMPONENT_IID("service_bus.catelyn.fep3.iid");

    /**
     * @brief Event code
     */
    enum class ServiceUpdateEventType
    {
        /**
         * *NOTIFIY* message with alive received
         */
        notify_alive,
        /**
         * *NOTIFIY* message with byebye received
         */
        notify_byebye,
        /**
         * response *OK* message was received
         */
        response
    };

    /**
     * @brief Struct storing the messages coming from service discovery.
     */
    struct ServiceUpdateEvent {
        /** @brief The name of the service that sent the update event */
        std::string service_name;
        /** @brief System that the service belongs */
        std::string system_name;
        /** @brief Url of the service that sent the service update message */
        std::string host_url;
        /** @brief Type of the service update message */
        ServiceUpdateEventType event_type;
    };

    /**
     * @brief Class for receiving the service discovery update events.
     */
    class IServiceUpdateEventSink {
    public:
        /**
         * @brief Inform the sink that a service discovery update message has been received.
         *
         * @param[in] service_update_event the service discovery update message
         */
        virtual void updateEvent(const ServiceUpdateEvent& service_update_event) = 0;

    protected:
        /// DTOR
        ~IServiceUpdateEventSink() = default;
    };

    /**
     * @brief the system access may represent one participant within one system.
     * A participant will only appear within this system if the @c createServer was called!
     */
    class ISystemAccess : public arya::IServiceBus::ISystemAccess {
    public:
        /**
         * .Register an event sink, IServiceUpdateEventSink::updateEvent method will be called when
         * a service discovery update message has been received.
         *
         * @param[in] update_event_sink Pointer to the sink that will receive the update messages.
         * @return ERR_FAILED if sink is already registered, ERR_NOERROR otherwise
         */
        virtual fep3::Result registerUpdateEventSink(
            IServiceUpdateEventSink* update_event_sink) = 0;

        /**
         * .Deregister an event sink.
         *
         * @param[in] update_event_sink Pointer to the sink to be deregistered.
         * @return ERR_FAILED if sink is not registered, ERR_NOERROR otherwise
         */
        virtual fep3::Result deregisterUpdateEventSink(
            IServiceUpdateEventSink* update_event_sink) = 0;

        /**
         * @brief create a server object
         * The server itself is one access point to the service bus, where services can be
         * registered and unregistered.
         *
         * @param[in] server_name name of the server (within FEP 3 this is the participant name to
         *                        appear) within one service bus instance this name has to be
         *                        unique. Usually the implementation should also prevent to create a
         *                        server with the same name twice within the discovery_network_url
         *                        (but this depends on the used protocol if this is possible)
         * @param[in] server_url address of the server. If environment variable
         *                       FEP3_SERVICEBUS_SERVER_URL is set, then it takes precedence over
         *                       the server_url argument. Values of the environment variable or
         *                       argument can be:
         *                       @li for example http://0.0.0.0:9090 to appear in all networks
         *                       @li for example http://192.168.1.2:9090 to appear only in
         *                           http://192.168.1.x networks
         * @param[in] discovery_active This flags indicates whether the service discovery will be
         *                             started. If set to false, the service will not be
         *                             discoverable.
         * @return if an error occurs the server object of the system access is empty
         * @retval ERR_NOERROR successfully created
         *
         * @remark if this function is called while a server was already created before, the current
         *         server will be release. if creating of the new server fails the server object
         *         will be empty.
         */
        virtual fep3::Result createServer(const std::string& server_name,
                                          const std::string& server_url,
                                          bool discovery_active) = 0;

        using fep3::arya::IServiceBus::ISystemAccess::createServer;
    };

    /**
     * @brief get the default server with the given name
     * @param[in] system_name name of the server or empty for default interface
     * @return the server if it does exist. will return default system access, if system_name is
     * empty.
     * @retval null std::shared_ptr<ISystemAccess>() the server if it does not exist
     */
    virtual std::shared_ptr<catelyn::IServiceBus::ISystemAccess> getSystemAccessCatelyn(
        const std::string& system_name = std::string()) const = 0;
};
} // namespace catelyn

using catelyn::IServiceBus;
} // namespace fep3
