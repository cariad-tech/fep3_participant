/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "logging_sink_requests.h"
#include "logging_sink_service_connection_status.h"
#include "logging_sink_service_connection_strategy.h"
#include "registered_rpc_sink_services.h"
#include "send_log_to_rpc_sink.h"

#include <fep3/base/properties/properties.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/rpc_services/logging/logging_rpc_sink_service_service_stub.h>
#include <fep3/rpc_services/logging/logging_service_rpc_intf_def.h>

#include <mutex>

namespace fep3 {
namespace native {

// this is the class to send messages to a registered sink far away on another process
using RPCSinkClientService = rpc::RPCService<fep3::rpc_stubs::RPCLoggingRPCSinkServiceServiceStub,
                                             fep3::rpc::IRPCLoggingSinkServiceDef>;

class LoggingSinkRPC;

class RPCSinkClientServiceImpl : public RPCSinkClientService {
public:
    explicit RPCSinkClientServiceImpl(LoggingSinkRPC& logging_sink);
    int registerRPCLoggingSinkClient(const std::string& address,
                                     const std::string& logger_name_filter,
                                     int severity) override;
    int unregisterRPCLoggingSinkClient(const std::string& address) override;

private:
    LoggingSinkRPC& _logging_sink;
};

/**
 * @brief Implementation of the rpc logging. Can be used as a base class for a custom sink.
 *        Logs will be send to the system library.
 */
class LoggingSinkRPC : public base::Properties<ILoggingService::ILoggingSink> {
public:
    explicit LoggingSinkRPC(IServiceBus& service_bus) : _service_bus(&service_bus)
    {
        _service_bus->getServer()->registerService(
            fep3::rpc::IRPCLoggingSinkServiceDef::getRPCDefaultName(),
            std::make_shared<RPCSinkClientServiceImpl>(*this));
    }

    void releaseServiceBus()
    {
        std::scoped_lock lock(_mutex_registered_rpc_sink_services);
        _registered_rpc_sink_services.clear();
        _service_bus->getServer()->unregisterService(
            fep3::rpc::IRPCLoggingSinkServiceDef::getRPCDefaultName());
        _service_bus = nullptr;
    }

    fep3::Result log(LogMessage log) const override final
    {
        std::queue<SinkRequest> sink_requests = _sink_registration_request_queue.popAll();
        // from here lock free from the RPC calls
        auto requester_factory = [this](const std::string& address) {
            return getRequester(address);
        };

        // we guard here since releaseServiceBus can be called from LoggingService::Destroy
        // the thread in LoggingQueue however can still trigger us until the destructor of
        // LoggingService
        std::scoped_lock lock(_mutex_registered_rpc_sink_services);

        // first we process the (un)registrations received from rpc calls
        _registered_rpc_sink_services.processRequests(sink_requests, requester_factory);
        // Then we check if all the registered sinks are alive
        LogginkSinkConnectionHandleStrategy strategy(_sink_connection_status);
        auto strategy_result =
            strategy.checkConnectivity(_registered_rpc_sink_services.getRegisteredSinkClients());
        // remove the dead sinks
        _registered_rpc_sink_services.processRequests(strategy_result, requester_factory);
        // send the log message to the sinks
        auto [result, failed_url] =
            sendLogToSinks(log, _registered_rpc_sink_services.getRegisteredSinkClients());
        // remove again the sinks that had an error during transmission
        auto failed_transmission_strategy_result = strategy.handleFailedTransmissions(failed_url);
        _registered_rpc_sink_services.processRequests(failed_transmission_strategy_result,
                                                      requester_factory);

        return result;
    }

public:
    int registerRPCLoggingSinkClient(const std::string& address,
                                     const std::string& logger_name_filter,
                                     int severity)
    {
        _sink_registration_request_queue.addRegisterSinkRequest(
            address, logger_name_filter, severity);
        return 0;
    }

    int unregisterRPCLoggingSinkClient(const std::string& address)
    {
        _sink_registration_request_queue.addUnRegisterSinkRequest(address);

        return 0;
    }

private:
    std::shared_ptr<fep3::arya::IServiceBus::IParticipantRequester> getRequester(
        const std::string& address) const
    {
        if (_service_bus) {
            return _service_bus->getRequester(address, true);
        }
        else
            return nullptr;
    }

    IServiceBus* _service_bus;

    mutable SinkRequestQueue _sink_registration_request_queue;
    mutable RegisteredRpcSinkServices _registered_rpc_sink_services;
    mutable LoggingSinkConnectionStatus _sink_connection_status;
    mutable std::mutex _mutex_registered_rpc_sink_services;
};

inline RPCSinkClientServiceImpl::RPCSinkClientServiceImpl(LoggingSinkRPC& logging_sink)
    : _logging_sink(logging_sink)
{
}

inline int RPCSinkClientServiceImpl::registerRPCLoggingSinkClient(
    const std::string& address, const std::string& logger_name_filter, int severity)
{
    return _logging_sink.registerRPCLoggingSinkClient(address, logger_name_filter, severity);
}

inline int RPCSinkClientServiceImpl::unregisterRPCLoggingSinkClient(const std::string& address)
{
    return _logging_sink.unregisterRPCLoggingSinkClient(address);
}

} // namespace native
} // namespace fep3
