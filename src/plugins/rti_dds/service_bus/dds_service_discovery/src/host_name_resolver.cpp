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
#include "host_name_resolver.h"

namespace fep3 {
namespace native {

HostNameResolver::HostNameResolver(std::shared_ptr<ILogger> logger) : _logger(std::move(logger))
{
}

std::string HostNameResolver::findIp(const std::string& address, const std::string& port)
{
    const std::string cached_ip = getCachedIp(address);
    if (!cached_ip.empty()) {
        return cached_ip;
    }

    boost::asio::ip::tcp::resolver resolver(ioservice);
    boost::asio::ip::tcp::resolver::query query(
        boost::asio::ip::tcp::v4(),
        address,
        port,
        boost::asio::ip::resolver_query_base::numeric_service);
    auto const query_res = resolver.resolve(query); // url, port
    auto wait_time = std::chrono::milliseconds(100);
    _socket_found = false;

    for (auto try_count = 0; !_socket_found && (try_count < _tries_max); ++try_count) {
        for (boost::asio::ip::tcp::resolver::iterator i = query_res.begin(); i != query_res.end();
             ++i) {
            _socket_found = true;
            async_connect(
                socket,
                i,
                [&](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator) {
                    if (ec)
                        _error_message = (ec.message());
                });
            await_operation(wait_time);
            if (_socket_found)
                break;
        }

        wait_time = wait_time * 2;
    }

    std::string found_socket;

    if (_socket_found) {
        found_socket = socket.remote_endpoint().address().to_string();
        _logger->logDebug(std::string("found ip ") + found_socket);
        _cached_hosts[address] = found_socket;
    }
    else {
        _logger->logError(std::string("ip not found"));
    }

    disconnect();
    return found_socket;
}

void HostNameResolver::disconnect()
{
    using namespace boost::asio;

    if (socket.is_open()) {
        try {
            socket.shutdown(ip::tcp::socket::shutdown_both);
            socket.close();
        }
        catch (const boost::system::system_error& e) {
            // ignore
            using namespace std::literals::string_literals;
            _logger->logWarning("ignored error during host name resolving "s + e.what());
        }
    }
}

void HostNameResolver::await_operation(std::chrono::milliseconds const& deadline_or_duration)
{
    using namespace boost::asio;

    ioservice.reset();
    {
        high_resolution_timer tm(ioservice, deadline_or_duration);
        tm.async_wait([this](boost::system::error_code ec) {
            if (ec != error::operation_aborted) {
                socket.cancel();
                socket.close();
                _socket_found = false;
            }
        });
        ioservice.run_one();
    }
    ioservice.run();
}

std::string HostNameResolver::getCachedIp(const std::string& address) const
{
    // check also if it is the same machine
    auto it = _cached_hosts.find(address);
    if (it == _cached_hosts.end()) {
        return {};
    }
    else {
        return it->second;
    }
}

} // namespace native
} // namespace fep3
