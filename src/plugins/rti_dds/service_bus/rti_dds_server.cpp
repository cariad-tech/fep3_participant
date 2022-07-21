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

#include "rti_dds_server.h"
#include <a_util/result.h>

using namespace fep3::arya;

namespace fep3
{
namespace rti_dds
{

/*
struct RPCResponseToFEPResponse : public IRPCRequester::IRPCResponse
{
    rpc::IResponse& _bounded_response;
    RPCResponseToFEPResponse(rpc::IResponse& response_to_bind) : _bounded_response(response_to_bind)
    {
    }
    fep3::Result set(const std::string& response)
    {
        _bounded_response.Set(response.c_str(), response.size());
        return {};
    }
};*/

DDSServer::DDSReceiverToRPCServiceWrapper::DDSReceiverToRPCServiceWrapper(const std::shared_ptr<IRPCService>& service)
    : _service(service)
{

}

a_util::result::Result DDSServer::DDSReceiverToRPCServiceWrapper::HandleCall(const char* ,
    size_t )
{
    return {};
}

std::shared_ptr<IServiceBus::IParticipantServer::IRPCService> DDSServer::DDSReceiverToRPCServiceWrapper::getService() const
{
    return _service;
}

DDSServer::DDSServer(const std::string& name,
                     const std::string& url,
                     const std::string& system_name)
    : fep3::base::arya::ServiceRegistryBase(name, system_name),
      _url(url)
{

}
DDSServer::~DDSServer()
{

}

std::vector<std::string> DDSServer::getRegisteredServiceNames() const
{
    std::vector<std::string> services_return_value;
    std::lock_guard<std::recursive_mutex> lock(_sync_list);
    for (const auto& service : _service_wrappers)
    {
        services_return_value.push_back(service.first);
    }
    return services_return_value;
}
std::shared_ptr<arya::IRPCServer::IRPCService> DDSServer::getServiceByName(const std::string& service_name) const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_list);
    const auto iterator = _service_wrappers.find(service_name);
    if (iterator != _service_wrappers.cbegin())
    {
        return iterator->second->getService();
    }
    return {};
}

fep3::Result DDSServer::registerService(const std::string& service_name,
    const std::shared_ptr<IRPCService>& )
{
    std::lock_guard<std::recursive_mutex> lock(_sync_list);
    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found != _service_wrappers.cend())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            "Service with the name %s already exists",
            service_name.c_str());
    }
    else
    {
     //  auto wrapper = std::make_shared<DDSServer::RPCObjectToRPCServerWrapper>(service);
      //  auto res = _http_server.RegisterRPCObject(service_name.c_str(), wrapper.get());
     //   if (fep3::isOk())
     //   {
     //       _service_wrappers[service_name] = wrapper;
            return {};
     //   }
     //   else
     //   {
     //       return res;
     //   }
    }
}


fep3::Result DDSServer::unregisterService(const std::string& service_name)
{
    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found == _service_wrappers.end())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            "Service with the name %s does not exists",
            service_name.c_str());
    }
    else
    {
       // DDSServer.UnregisterRPCObject(service_name.c_str());
        _service_wrappers.erase(service_name);
        return {};
    }
}

std::string DDSServer::getUrl() const
{
    return _url;
}
}
}

