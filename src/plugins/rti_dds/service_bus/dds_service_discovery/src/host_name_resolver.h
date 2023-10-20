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

#include "../../../../../fep3/native_components/service_bus/rpc/service_discovery/service_discovery_intf/logger_proxy.h"
#include "../include/host_name_resolver_intf.h"

#include <boost/asio.hpp>

#include <map>

namespace fep3 {
namespace native {

class HostNameResolver : public IHostNameResolver {
public:
    explicit HostNameResolver(std::shared_ptr<ILogger> logger);
    std::string findIp(const std::string& address, const std::string& port) override;

private:
    std::string getCachedIp(const std::string& address) const;
    void disconnect();
    void await_operation(std::chrono::milliseconds const& deadline_or_duration);

    boost::asio::io_service ioservice{};
    boost::asio::ip::tcp::socket socket{ioservice};
    const uint8_t _tries_max = 5;
    std::string _error_message;
    bool _socket_found = false;
    std::shared_ptr<ILogger> _logger;
    std::map<std::string, std::string> _cached_hosts;
};

} // namespace native
} // namespace fep3
