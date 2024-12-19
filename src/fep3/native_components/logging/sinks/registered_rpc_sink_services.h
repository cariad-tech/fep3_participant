/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "logging_sink_requests.h"

#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/native_components/logging/sinks/logging_formater_common.hpp>
#include <fep3/rpc_services/logging/logging_rpc_sink_client_client_stub.h>
#include <fep3/rpc_services/logging/logging_service_rpc_intf_def.h>

#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <thread>

namespace fep3::native {
template <class>
inline constexpr bool always_false_v = false;

inline void logHelper(const std::string& log)
{
    std::cout << "[time:" << fep3::native::getLocalTime() << "]"
              << "[thread:" << std::this_thread::get_id() << "]" << log << "\n";
}

using RPCSinkClientClient =
    fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingRPCSinkClientClientStub,
                                fep3::rpc::IRPCLoggingSinkClientDef>;
struct ClientFilter {
    std::string _name_filter;
    fep3::LoggerSeverity _severity_filter;
    std::unique_ptr<RPCSinkClientClient> _client;
};

class RegisteredRpcSinkServices {
public:
    template <typename RequesterFactoryFunction>
    void processRequests(std::queue<SinkRequest> requests, const RequesterFactoryFunction& factory)
    {
        while (!requests.empty()) {
            std::visit(
                [this, &factory](const auto& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, RegisterSinkRequest>)
                        addClient(arg, factory);
                    else if constexpr (std::is_same_v<T, UnRegisterSinkRequest>)
                        removeClient(arg.address);
                    else
                        static_assert(always_false_v<T>, "non-exhaustive visitor!");
                },
                requests.front());
            requests.pop();
        }
    }

    const std::map<std::string, ClientFilter>& getRegisteredSinkClients() const
    {
        return _client_filters;
    }

    void clear()
    {
        _client_filters.clear();
    }

private:
    template <typename RequesterFactoryFunction>
    void addClient(const RegisterSinkRequest& register_request,
                   const RequesterFactoryFunction& factory)
    {
        const auto& address = register_request.address;

        auto new_client = std::make_unique<RPCSinkClientClient>(
            rpc::IRPCLoggingSinkClientDef::getRPCDefaultName(), factory(address));

        auto& new_filter = _client_filters[address];
        new_filter._name_filter = register_request.logger_name_filter;
        new_filter._severity_filter = static_cast<fep3::LoggerSeverity>(register_request.severity);
        new_filter._client.reset(new_client.release());
    }

    void removeClient(const std::string& address)
    {
        _client_filters.erase(address);
        logHelper(std::string("RPC Sink with url: ") + address +
                  " is unreachable and will be deleted");
    }

private:
    std::map<std::string, ClientFilter> _client_filters;
};
} // namespace fep3::native
