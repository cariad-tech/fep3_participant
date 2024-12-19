/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "http_server.h"

#include "http_systemaccess.h"

#include <a_util/strings/strings_convert.h>

#include <cxx_url.h>

using namespace fep3::arya;

namespace fep3 {
namespace native {

constexpr const char* const HttpServer::_default_url;
constexpr const char* const HttpServer::_discovery_search_target;

/*******************************************************************************************
 *
 *******************************************************************************************/
struct RPCResponseToFEPResponse : public arya::IRPCRequester::IRPCResponse {
    ::rpc::IResponse& _bounded_response;
    RPCResponseToFEPResponse(::rpc::IResponse& response_to_bind)
        : _bounded_response(response_to_bind)
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

HttpServer::RPCObjectToRPCServerWrapper::RPCObjectToRPCServerWrapper(
    const std::shared_ptr<IRPCService>& service)
    : _service(service)
{
}

a_util::result::Result HttpServer::RPCObjectToRPCServerWrapper::HandleCall(
    const char* strRequest, size_t, ::rpc::IResponse& oResponse)
{
    RPCResponseToFEPResponse response_convert(oResponse);
    return _service->handleRequest("json", strRequest, response_convert);
}

std::shared_ptr<arya::IRPCServer::IRPCService> HttpServer::RPCObjectToRPCServerWrapper::getService()
    const
{
    return _service;
}

/*******************************************************************************************
 *
 *******************************************************************************************/

HttpServer::HttpServer(const std::string& name,
                       const std::string& url,
                       const std::string& system_name,
                       const std::string& system_url,
                       std::shared_ptr<ILogger> logger,
                       std::shared_ptr<IServiceDiscoveryFactory> service_discovery_factory,
                       bool discovery_active,
                       std::unique_ptr<IThreadPoolExecutor> thread_executor)
    : _url(url),
      _system_url(system_url),
      base::arya::ServiceRegistryBase(name, system_name),
      _heartbeat_interval(std::chrono::milliseconds(5000)),
      _logger_proxy(std::make_shared<LoggerProxy>(logger)),
      _service_discovery_factory(std::move(service_discovery_factory)),
      _thread_executor(std::move(thread_executor))
{
    _default_server_url_used = _url == _use_default_url;
    if (_default_server_url_used) {
        _url = _default_url;
    }
    fep3::helper::Url url_to_parse = _url;
    int port_number = a_util::strings::toInt32(url_to_parse.port());
    a_util::result::Result res;

    // If port is given as non-zero, we only try to allocate the given port.
    if (port_number != 0) {
        _port_begin = port_number;
        _port_end = _port_begin;
    }

    for (int port = _port_begin; port <= _port_end; ++port) {
        _url = url_to_parse.scheme() + "://" + url_to_parse.host() + ":" + std::to_string(port);
        res = _http_server.StartListening(_url.c_str(), 0);
        if (res) {
            _logger_proxy->logDebug(
                a_util::strings::format("HttpServer StartListening on %s", _url.c_str()));
            break;
        }
    }

    if (!res) {
        std::string throw_msg =
            std::string("start listening on ") + _url + " failed, errno = " + res.getDescription();
        throw std::runtime_error(throw_msg);
    }

    if (discovery_active) {
        if (!system_url.empty()) {
            startDiscovery();
        }
        else {
            throw std::runtime_error(
                "discovery_active flag set to true, but used system url is empty. set "
                "discovery_active to false or use a valid system url");
        }
    }

    _thread_executor->start();

    _is_started = true;
}

fep3::Result HttpServer::initialize()
{
    auto res = base::ServiceRegistryBase::initialize();
    if (res) {
        auto rpc_http_server = std::make_shared<RPCHttpServer>(*this);
        return registerService(rpc::catelyn::IRPCHttpServerDef::DEFAULT_NAME, rpc_http_server);
    }
    else {
        return res;
    }
}

void HttpServer::startDiscovery()
{
    const std::string system_location = getName() + "@" + getSystemName();

    _logger_proxy->logDebug(
        a_util::strings::format("HttpServer creating system discovery and "
                                "heartbeat threads for system  %s with server url %s",
                                system_location.c_str(),
                                _url.c_str()));

    _discovery_service = _service_discovery_factory->getServiceDiscovery(
        _system_url,
        HttpSystemAccess::getNetworkInterface(),
        std::chrono::seconds(60),
        std::make_pair(_url, _default_server_url_used),
        // TODO: create a Type for this discovery service name
        system_location,
        HttpServer::_discovery_search_target,
        FEP3_PARTICIPANT_LIBRARY_VERSION_ID,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR);

    _logger_proxy->logDebug("HttpServer starting discovery and heartbeat threads");
    startDiscoveryThread();
    startHeartbeatThread();
}

void HttpServer::startDiscoveryThread()
{
    if (_discovery_service) {
        _discovery_stop_handle = _thread_executor->postPeriodic(std::chrono::seconds(1), [this] {
            try {
                _discovery_service->checkForMSearchAndSendResponse(std::chrono::seconds(1));
            }
            catch (const std::exception& ex) {
                _logger_proxy->logError(
                    a_util::strings::format("Exception while executing the discovery thread, "
                                            "exception: %s, thread will stop",
                                            ex.what()));
                return false;
            }
            return true;
        });
    }
}

void HttpServer::stopDiscoveryThread()
{
    _thread_executor->cancel(_discovery_stop_handle);
}

void HttpServer::startHeartbeatThread()
{
    if (_discovery_service) {
        _heartbeat_stop_handle = _thread_executor->postPeriodic(_heartbeat_interval, [this] {
            try {
                _logger_proxy->logDebug("HttpServer sending notify alive");
                if (!_discovery_service->sendNotifyAlive()) {
                    _logger_proxy->logError(
                        a_util::strings::format("Error while sending sendNotifyAlive, error: %s",
                                                _discovery_service->getLastSendErrors().c_str()));
                }
            }
            catch (const std::exception& ex) {
                _logger_proxy->logError(
                    a_util::strings::format("Exception while executing the heartbeat thread, "
                                            "exception: %s, thread will stop",
                                            ex.what()));
                return false;
            }
            return true;
        });
    }
}

void HttpServer::stopHeartbeatThread()
{
    _thread_executor->cancel(_heartbeat_stop_handle);
}

void HttpServer::stopDiscovery()
{
    _logger_proxy->logDebug("HttpServer stopping discovery and heartbeat threads");
    stopHeartbeatThread();
    stopDiscoveryThread();
    if (_discovery_service) {
        _discovery_service->sendNotifyByeBye();
    }
}

void HttpServer::setHeartbeatInterval(std::chrono::milliseconds interval)
{
    stopHeartbeatThread();
    _heartbeat_interval = interval;
    startHeartbeatThread();
}

std::chrono::milliseconds HttpServer::getHeartbeatInterval() const
{
    return _heartbeat_interval;
}

HttpServer::~HttpServer()
{
    _is_started = false;
    _logger_proxy->logDebug("HttpServer StopListening");
    _http_server.StopListening();

    if (!_service_wrappers.empty()) {
        std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);
        for (const auto& service_key_value: _service_wrappers) {
            _http_server.UnregisterRPCObject(service_key_value.first.c_str());
        }

        _service_wrappers.clear();
    }

    stopDiscovery();
    // wait for pending tasks and join threads in pool
    _thread_executor.reset();
}

fep3::Result HttpServer::registerService(const std::string& service_name,
                                         const std::shared_ptr<IRPCService>& service)
{
    if (_logger_proxy->isDebugEnabled()) {
        _logger_proxy->logDebug(a_util::strings::format(
            "HttpServer registering service %s, with IID %s and interface definition %s",
            service_name.c_str(),
            service->getRPCServiceIIDs().c_str(),
            service->getRPCInterfaceDefinition().c_str()));
    }

    std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);

    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found != _service_wrappers.cend()) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                                 "ServiceDiscovery with the name %s already exists",
                                 service_name.c_str());
    }
    else {
        auto wrapper = std::make_shared<HttpServer::RPCObjectToRPCServerWrapper>(service);
        auto res = _http_server.RegisterRPCObject(service_name.c_str(), wrapper.get());
        if (res) {
            _service_wrappers[service_name] = wrapper;
            return {};
        }
        else {
            return res;
        }
    }
}

fep3::Result HttpServer::unregisterService(const std::string& service_name)
{
    _logger_proxy->logDebug(
        a_util::strings::format("HttpServer unregistering service %s", service_name.c_str()));

    std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);

    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found == _service_wrappers.end()) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                                 "ServiceDiscovery with the name %s does not exists",
                                 service_name.c_str());
    }
    else {
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
    _logger_proxy->logDebug("HttpServer will lock wrapper for getRegisteredServiceNames");
    std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);
    std::vector<std::string> names;
    for (const auto& value: _service_wrappers) {
        names.push_back(value.first);
    }
    return names;
}

std::shared_ptr<arya::IRPCServer::IRPCService> HttpServer::getServiceByName(
    const std::string& service_name) const
{
    std::lock_guard<std::recursive_mutex> _lock(_sync_wrappers);

    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found != _service_wrappers.end()) {
        return service_found->second->getService();
    }
    else {
        return {};
    }
}

Json::Value RPCHttpServer::setHeartbeatInterval(int interval_ms)
{
    Json::Value json_value;
    _http_server.setHeartbeatInterval(std::chrono::milliseconds(interval_ms));
    json_value["interval_ms"] = _http_server.getHeartbeatInterval().count();
    return json_value;
}

Json::Value RPCHttpServer::getHeartbeatInterval()
{
    Json::Value json_value;
    json_value["interval_ms"] = _http_server.getHeartbeatInterval().count();
    return json_value;
}
} // namespace native
} // namespace fep3
