/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "data_registry_rpc_client.h"

#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/rpc_services/data_registry/data_registry_rpc_intf_def.h>

#include <data_registry_client_stub.h>

namespace fep3::base {

DataRegistryRpcClient::DataRegistryRpcClient(const char* server_object_name,
                                             std::shared_ptr<fep3::IRPCRequester> rpc)
    : DataRegistryRpcClient::base_type(server_object_name, rpc)
{
}

std::vector<SignalInfo> DataRegistryRpcClient::getRegisteredSignals()
{
    std::vector<SignalInfo> ret;

    fillSignalInfo(getSignalInNames(), ret, SignalFlow::input);
    fillSignalInfo(getSignalOutNames(), ret, SignalFlow::output);

    return ret;
}

void DataRegistryRpcClient::fillSignalInfo(const std::vector<std::string>& signal_names,
                                           std::vector<SignalInfo>& info,
                                           SignalFlow direction)
{
    for (const auto& signal_name: signal_names) {
        info.push_back(getSignalInfo(signal_name, direction));
    }
}

std::vector<std::string> DataRegistryRpcClient::getSignalInNames() const
{
    try {
        return a_util::strings::split(GetStub().getSignalInNames(), ",");
    }
    catch (jsonrpc::JsonRpcException&) {
    }
    return std::vector<std::string>{};
}

std::vector<std::string> DataRegistryRpcClient::getSignalOutNames() const
{
    try {
        return a_util::strings::split(GetStub().getSignalOutNames(), ",");
    }
    catch (jsonrpc::JsonRpcException&) {
    }
    return std::vector<std::string>{};
}

SignalInfo DataRegistryRpcClient::getSignalInfo(const std::string& signal_name,
                                                SignalFlow direction) const
{
    SignalInfo signal_info;
    try {
        signal_info._name = signal_name;
        signal_info._direction = direction;
        const auto json_value = GetStub().getStreamType(signal_name);
        signal_info._stream_meta_type = json_value["meta_type"].asString();

        const auto properties = json_value["properties"];
        auto current_property = properties.begin();

        while (current_property != properties.end()) {
            signal_info._properties.push_back(
                StreamTypeProperty{(*current_property)["name"].asString(),
                                   (*current_property)["value"].asString(),
                                   (*current_property)["type"].asString()});

            current_property++;
        }
    }
    catch (jsonrpc::JsonRpcException&) {
        // TODO Error Handling
        signal_info._name = "Error retrieving";
    }

    return signal_info;
}

} // namespace fep3::base
