/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "logging_sink_requests.h"

#include <boost/range/adaptors.hpp>

#include <map>
#include <queue>
#include <string>

namespace fep3::native {
template <typename ConnectionStatus>
class LogginkSinkConnectionHandleStrategy {
public:
    LogginkSinkConnectionHandleStrategy(ConnectionStatus& connection_status)
        : _connection_status(connection_status)
    {
    }

    template <typename ClientFilterType>
    std::queue<SinkRequest> checkConnectivity(
        const std::map<std::string, ClientFilterType>& client_filters)
    {
        auto unreachable_urls_range =
            _connection_status.getUnreachable(boost::adaptors::keys(client_filters));

        return handleFailedTransmissions(unreachable_urls_range);
    }

    template <typename UrlsIterable>
    std::queue<SinkRequest> handleFailedTransmissions(const UrlsIterable& failed_connections)
    {
        std::queue<SinkRequest> ret;
        for (const auto& unreachable_url: failed_connections) {
            ret.emplace(UnRegisterSinkRequest{unreachable_url});
        }

        return ret;
    }

private:
    ConnectionStatus& _connection_status;
};
} // namespace fep3::native
