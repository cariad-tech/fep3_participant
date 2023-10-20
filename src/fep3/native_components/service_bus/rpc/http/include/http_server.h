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

#include "service_discovery_factory_intf.h"

#include <fep3/components/service_bus/service_registry_base.hpp>
#include <fep3/rpc_services/service_bus/http_server_rpc_intf_def.h>
#include <fep3/rpc_services/service_bus/http_server_service_stub.h>

#include <threaded_executor.h>

#pragma warning(push)
#pragma warning(disable : 4290)
#include <rpc/rpc.h>
#pragma warning(pop)

#include <condition_variable>
#include <mutex>
#include <thread>

namespace fep3 {
namespace native {

class RPCHttpServer;
class LoggerProxy;

class HttpServer : public fep3::base::arya::ServiceRegistryBase {
public:
    struct RPCObjectToRPCServerWrapper : public ::rpc::IRPCObject {
    public:
        explicit RPCObjectToRPCServerWrapper(const std::shared_ptr<IRPCService>& service);
        virtual ~RPCObjectToRPCServerWrapper() = default;
        a_util::result::Result HandleCall(const char* strRequest,
                                          size_t nRequestSize,
                                          ::rpc::IResponse& oResponse);
        std::shared_ptr<IRPCServer::IRPCService> getService() const;

    private:
        std::shared_ptr<IRPCServer::IRPCService> _service;
    };

public:
    HttpServer(const std::string& name,
               const std::string& url,
               const std::string& system_name,
               const std::string& system_url,
               std::shared_ptr<ILogger> logger,
               std::shared_ptr<IServiceDiscoveryFactory> service_discovery_factory,
               bool discovery_active,
               std::unique_ptr<IThreadPoolExecutor> thread_executor =
                   std::make_unique<ThreadPoolExecutor>(2));
    virtual ~HttpServer();

public: // implementation of ServiceRegistryBase
    fep3::Result initialize() override;
    fep3::Result registerService(const std::string& service_name,
                                 const std::shared_ptr<IRPCService>& service) override;
    fep3::Result unregisterService(const std::string& service_name) override;
    std::string getUrl() const override;

    std::vector<std::string> getRegisteredServiceNames() const override;
    std::shared_ptr<arya::IRPCServer::IRPCService> getServiceByName(
        const std::string& service_name) const override;

    void setHeartbeatInterval(std::chrono::milliseconds interval);
    std::chrono::milliseconds getHeartbeatInterval() const;

public: // default url of this implementation
    static constexpr const char* const _default_url = "http://0.0.0.0:0";
    static constexpr const char* const _discovery_search_target =
        "fep3:servicebus:http:participant";

private:
    ::rpc::http::cJSONRPCServer _http_server;
    std::map<std::string, std::shared_ptr<RPCObjectToRPCServerWrapper>> _service_wrappers;
    mutable std::recursive_mutex _sync_wrappers;
    bool _is_started = false;

    std::string _url;
    std::string _system_url;
    bool _default_server_url_used = true;
    int _port_begin = 9090;
    int _port_end = 10090;
    std::unique_ptr<fep3::native::IServiceDiscovery> _discovery_service;

    void startDiscovery();
    void stopDiscovery();
    void startDiscoveryThread();
    void stopDiscoveryThread();
    void startHeartbeatThread();
    void stopHeartbeatThread();
    std::chrono::milliseconds _heartbeat_interval;

    std::shared_ptr<LoggerProxy> _logger_proxy;
    std::shared_ptr<IServiceDiscoveryFactory> _service_discovery_factory;

    std::unique_ptr<IThreadPoolExecutor> _thread_executor;
    uintptr_t _discovery_stop_handle;
    uintptr_t _heartbeat_stop_handle;
};

class RPCHttpServer
    : public rpc::RPCService<rpc_stubs::RPCHttpServerServiceStub, rpc::IRPCHttpServerDef> {
public:
    explicit RPCHttpServer(HttpServer& http_server) : _http_server(http_server)
    {
    }

protected:
    Json::Value setHeartbeatInterval(int interval_ms) override;
    Json::Value getHeartbeatInterval() override;

private:
    HttpServer& _http_server;
};

} // namespace native
} // namespace fep3
