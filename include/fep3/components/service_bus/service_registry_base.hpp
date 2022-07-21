/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */

#pragma once

#include <fep3/fep3_participant_version.h>
#include <string>
#include <memory>
#include <vector>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/rpc_services/participant_info/participant_info_rpc_intf_def.h>
#include <fep3/rpc_services/participant_info/participant_info_service_stub.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>


namespace fep3
{
namespace base
{
namespace arya
{
/**
 * @cond no_documentation
 * see documentation below for class ServiceRegistryBase
 */
namespace detail
{

class IServiceRegistryRPCService
{
protected:
    ~IServiceRegistryRPCService() = default;
public:
    virtual std::vector<std::string> getRegisteredServiceNames() const = 0;
    virtual std::shared_ptr<fep3::arya::IRPCServer::IRPCService> getServiceByName(const std::string& service_name) const = 0;
    virtual std::string getParticipantName() const = 0;
    virtual std::string getSystemName() const = 0;
};

class ServiceRegistryRPCService : public
    rpc::arya::RPCService<fep3::rpc_stubs::RPCParticipantInfoServiceStub,
    rpc::arya::IRPCParticipantInfoDef>
{
public:
    explicit ServiceRegistryRPCService(IServiceRegistryRPCService& info) : _info(info)
    {
    }
    std::string getRPCServices() override
    {
        auto names = _info.getRegisteredServiceNames();
        bool first = true;
        std::string result_string;
        for (const auto& current_value : names)
        {
            if (!first)
            {
                result_string += ";";
            }
            result_string += current_value;
            first = false;
        }
        return result_string;
    }
    std::string getRPCServiceIIDs(const std::string& rpc_service_name) override
    {
        auto service = _info.getServiceByName(rpc_service_name);
        if (service)
        {
            return service->getRPCServiceIIDs();
        }
        else
        {
            return std::string();
        }
    }
    std::string getRPCServiceInterfaceDefinition(const std::string& rpc_service_name, const std::string&)
    {
        auto service = _info.getServiceByName(rpc_service_name);
        if (service)
        {
            return service->getRPCInterfaceDefinition();
        }
        else
        {
            return std::string();
        }
    }
    std::string getName() override
    {
        return _info.getParticipantName();
    }
    std::string getSystemName()
    {
        return _info.getSystemName();
    }
    std::string getFEPVersion()
    {
        return FEP3_PARTICIPANT_LIBRARY_VERSION_STR;
    }
private:
    arya::detail::IServiceRegistryRPCService& _info;
};
}
/**
 * @endcond no_documentation
 */

/**
 * @brief ServiceRegistry Base class to implement the ParticipantInfo within
 *        the servicebus implementation
 * This class is only necessary if you want to implement your own IServiceBus::IParticipantServer
 *
 * If using it, you need to implement to retrieve information for the RPC call:
 * \li getRegisteredServiceNames
 * \li getServiceByName
* And implement for the IParticipantServer:
 * \li registerService
 * \li unregisterService
 * \li getUrl
 */
class ServiceRegistryBase : public fep3::arya::IServiceBus::IParticipantServer,
    public arya::detail::IServiceRegistryRPCService
{
public:
    /**
     * @brief Construct a Service Registry Base object
     *
     * @param name name of the participant server to register (this will be the IServiceBus::IParticipantServer)
     * @param system_name the participants system it belongs to (might change while runtime!!)
     */
    ServiceRegistryBase(const std::string& name,
        const std::string& system_name)
        : _name(name), _system_name(system_name)
    {
    }

    /**
     * @brief intializes the RPC ServiceRegistryRPCService and register it
     * @remark Unfortunately, this MUST be called separately since virtual functional calls are not possible by a CTOR
     *         We could also fake here, that the first registerService call will call also for IRPCParticipantInfoDef!
     * @return returns the error values of arya::IServiceBus::IParticipantServer::registerService
     */
    fep3::Result initialize()
    {
        auto rpc_info_service = std::make_shared<arya::detail::ServiceRegistryRPCService>(*this);
        return registerService(rpc::arya::IRPCParticipantInfoDef::DEFAULT_NAME, rpc_info_service);
    }

public: //rpc service ... needs to override it

    //virtual std::vector<std::string> getRegisteredServiceNames() const = 0;
    //virtual std::shared_ptr<arya::IRPCServer::IRPCService> getServiceByName(const std::string& service_name) const = 0;

public: //internal rpc server... needs to override it
    // virtual fep3::Result registerService(const std::string& service_name,
    //                                      const std::shared_ptr<IRPCService>& service) = 0;
    // virtual fep3::Result unregisterService(const std::string& service_name) = 0;
    // virtual std::string getUrl() const = 0;

public: //rpc service pre-implementation
    /**
     * retrieve the name of the participant
     * @return the name of the participant
     */
    std::string getParticipantName() const
    {
        return _name;
    }
    /**
     * retrieve the system name the participant belongs to
     * @return  the system name the participant belongs to
     */
    std::string getSystemName() const
    {
        return _system_name;
    }

public: //internal rpc server pre implementation
    /**
     * @copydoc arya::IRPCServer::getName
     */
    std::string getName() const override
    {
        return _name;
    }
    /**
     * @copydoc arya::IRPCServer::setName
     */
    void setName(const std::string& name) override
    {
        _name = name;
    }

private:
    //the name
    std::string _name;
    //the system name
    std::string _system_name;

};

}
using arya::ServiceRegistryBase;
}
}
