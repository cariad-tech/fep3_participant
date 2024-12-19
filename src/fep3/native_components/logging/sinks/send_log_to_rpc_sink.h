/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "registered_rpc_sink_services.h"

#include <fep3/base/logging/logging_types.h>

namespace fep3::native {

template <typename ClientFilterType>
std::pair<fep3::Result, std::vector<std::string>> sendLogToSinks(
    const fep3::LogMessage& log, const std::map<std::string, ClientFilterType>& _client_filters)
{
    fep3::Result result;
    std::vector<std::string> failed_connections;

    for (const auto& current_client: _client_filters) {
        try {
            auto tmp_result = current_client.second._client->onLog(log._message,
                                                                   log._logger_name,
                                                                   log._participant_name,
                                                                   static_cast<int>(log._severity),
                                                                   log._timestamp);
            if (tmp_result != 0) {
                failed_connections.push_back(current_client.first);
                result = tmp_result;
            }
        }
        catch (const jsonrpc::JsonRpcException& ex) {
            result = CREATE_ERROR_DESCRIPTION(ERR_EXCEPTION_RAISED, ex.what());
            failed_connections.push_back(current_client.first);
        }
        catch (const std::exception& ex) {
            result = CREATE_ERROR_DESCRIPTION(ERR_EXCEPTION_RAISED, ex.what());
            failed_connections.push_back(current_client.first);
        }
    }

    return {result, failed_connections};
}

} // namespace fep3::native
