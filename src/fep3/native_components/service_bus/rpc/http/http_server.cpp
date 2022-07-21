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
#include "http_server.h"
#include "http_systemaccess.h"
#include "find_free_port.h"
#include <../3rdparty/lssdp-cpp/src/url/cxx_url.h>
#include "../../service_bus_logger.hpp"
#include <a_util/strings/strings_convert.h>
#include <fep3/base/environment_variable/environment_variable.h>

using namespace fep3::arya;

namespace fep3
{
namespace native
{

constexpr const char* const HttpServer::_default_url;
constexpr const char* const HttpServer::_discovery_search_target;

/*******************************************************************************************
 *
 *******************************************************************************************/
struct RPCResponseToFEPResponse : public arya::IRPCRequester::IRPCResponse
{
    ::rpc::IResponse& _bounded_response;
    RPCResponseToFEPResponse(::rpc::IResponse& response_to_bind) : _bounded_response(response_to_bind)
    {
    }
    fep3::Result set(const std::string& response)
    {
        _bounded_response.Set(response.c_str(), response.size());
        return {};
    }
};

/*******************************************************************************************
 *
 *******************************************************************************************/

HttpServer::RPCObjectToRPCServerWrapper::RPCObjectToRPCServerWrapper(const std::shared_ptr<IRPCService>& service)
    : _service(service)
{
}

a_util::result::Result HttpServer::RPCObjectToRPCServerWrapper::HandleCall(
    const char* strRequest,
    size_t,
    ::rpc::IResponse& oResponse)
{
    RPCResponseToFEPResponse response_convert(oResponse);
    return _service->handleRequest(
        "json",
        strRequest,
        response_convert);
}

std::shared_ptr<arya::IRPCServer::IRPCService> HttpServer::RPCObjectToRPCServerWrapper::getService() const
{
    return _service;
}

/*******************************************************************************************
 *
 *******************************************************************************************/

HttpServer::HttpServer(const std::string& name,
                       const std::string& url,
                       const std::string& system_name,
                       const std::string& system_url)
    : _url(url), _system_url(system_url), base::arya::ServiceRegistryBase(name, system_name)
{
    if (_url == _use_default_url)
    {
        _url = _default_url;
    }
    fep3::helper::Url url_to_parse = _url;
    int port_number = a_util::strings::toInt32(url_to_parse.port());
    a_util::result::Result res;

	// If port is given as non-zero, we only try to allocate the given port.
    if (port_number != 0)
    {
        _port_begin = port_number;
        _port_end = _port_begin;
    }

    for (int port = _port_begin; port <= _port_end; ++port)
    {
        _url = url_to_parse.scheme() + "://" + url_to_parse.host() + ":" + a_util::strings::toString(port);
        res = _http_server.StartListening(_url.c_str(), 0);
        if (fep3::isOk(res))
        {
            break;
        }
    }

    if (fep3::isFailed(res))
    {
        std::string throw_msg = std::string("start listening on ") + _url + " failed, errno = "
            + res.getDescription();
        throw std::runtime_error(throw_msg);
    }

    if (!system_url.empty())
    {
        //TODO: the interval must be parsed from the system url in future
        startDiscovery(std::chrono::seconds(5));
    }

    _is_started = true;
}

void HttpServer::startDiscovery(std::chrono::seconds interval)
{
    _lssdp_service = std::make_unique<lssdp::Service>(
         _system_url,
        HttpSystemAccess::getNetworkInterface(),
        std::chrono::seconds(60),
        _url,
        //TODO: create a Type for this discovery service name
        getName() + "@" + getSystemName(),
        HttpServer::_discovery_search_target,
        FEP3_PARTICIPANT_LIBRARY_VERSION_ID,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR);

    _stop_loop = false;

    _loop = std::move(std::thread(
        [this, send_alive_interval = interval]
        {
            try
            {
                auto last_time = std::chrono::system_clock::now();

                // send notify first
                _lssdp_service->sendNotifyAlive();

                do
                {
                    auto now = std::chrono::system_clock::now();
                    if ((now - last_time) >= send_alive_interval)
                    {
                        last_time = now;
                        if (!_lssdp_service->sendNotifyAlive())
                        {
                            service_bus_helper::Logger::get().internalLog(_lssdp_service->getLastSendErrors());
                        }
                    }
                    _lssdp_service->checkForMSearchAndSendResponse(std::chrono::seconds(1));

                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                } while (!_stop_loop);

                // send notify to say good bye
                _lssdp_service->sendNotifyByeBye();
            }
            catch (const std::exception& ex)
            {
                service_bus_helper::Logger::get().internalLog(ex.what());
            }
        }
        )
    );
}

void HttpServer::stopDiscovery()
{
    if (_lssdp_service)
    {
        _stop_loop = true;
        if (_loop.joinable())
        {
            _loop.join();
        }
    }
}

HttpServer::~HttpServer()
{
    _is_started = false;
    _http_server.StopListening();

    if (!_service_wrappers.empty())
    {
        std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);
        for (const auto& service_key_value : _service_wrappers)
        {
            _http_server.UnregisterRPCObject(service_key_value.first.c_str());
        }
        
        _service_wrappers.clear();
    }

    stopDiscovery();
}

fep3::Result HttpServer::registerService(const std::string& service_name,
    const std::shared_ptr<IRPCService>& service)
{
    std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);

    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found != _service_wrappers.cend())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            "Service with the name %s already exists",
            service_name.c_str());
    }
    else
    {
        auto wrapper = std::make_shared<HttpServer::RPCObjectToRPCServerWrapper>(service);
        auto res = _http_server.RegisterRPCObject(service_name.c_str(), wrapper.get());
        if (fep3::isOk(res))
        {
            _service_wrappers[service_name] = wrapper;
            return {};
        }
        else
        {
            return res;
        }
    }
}


fep3::Result HttpServer::unregisterService(const std::string& service_name)
{
    std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);

    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found == _service_wrappers.end())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            "Service with the name %s does not exists",
            service_name.c_str());
    }
    else
    {
        _http_server.UnregisterRPCObject(service_name.c_str());
        _service_wrappers.erase(service_name);
        return {};
    }
}

std::string HttpServer::getUrl() const
{
    return _url;
}

std::vector<std::string> HttpServer::getRegisteredServiceNames() const
{
    std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);
    std::vector<std::string> names;
    for (const auto& value : _service_wrappers)
    {
        names.push_back(value.first);
    }
    return names;
}

std::shared_ptr<arya::IRPCServer::IRPCService> HttpServer::getServiceByName(const std::string& service_name) const
{
    std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);

    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found != _service_wrappers.end())
    {
        return service_found->second->getService();
    }
    else
    {
        return {};
    }
}

}
}

