/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "logging_sink_service_connection_status.h"

#include <fep3/native_components/logging/sinks/logging_formater_common.hpp>

#include <boost/asio/strand.hpp>
#include <boost/asio/use_future.hpp>
#include <cxx_url.h>

#include <iostream>
#include <thread>

namespace fep3::native {

constexpr std::chrono::milliseconds socket_connection_timeout = std::chrono::milliseconds{100};

LoggingSinkConnectionStatus::LoggingSinkConnectionStatus()
    : _work(boost::asio::make_work_guard(_ioc)),
      _stream(boost::asio::make_strand(_ioc)),
      _resolver(boost::asio::make_strand(_ioc))
{
    _context_thread = std::thread([&]() { _ioc.run(); });
}

LoggingSinkConnectionStatus::~LoggingSinkConnectionStatus()
{
    _resolver.cancel();
    _stream.close();
    // https://www.boost.org/doc/libs/1_78_0/doc/html/boost_asio/reference/io_context.html#:~:text=the%20application%20will%20then%20need%20to%20call%20the%20io_context%20object%27s%20stop()%20member%20function
    // To effect a shutdown, the application will then need to call the io_context object's
    // stop() member function. This will cause the io_context run() call to return as soon as
    // possible, abandoning unfinished operations and without permitting ready handlers to be
    // dispatched.
    try {
        _ioc.stop();
    }
    catch (const std::exception&) {
    }

    _work.reset();

    if (_context_thread.joinable()) {
        _context_thread.join();
    }
}

template <typename U, typename T>
bool LoggingSinkConnectionStatus::endpointReachable(const T& endpoint)
{
    auto single_endpoint = U::create(endpoint, endpoint.host_name(), endpoint.service_name());
    auto connect_fut = _stream.async_connect(single_endpoint, boost::asio::use_future);
    try {
        connect_fut.get();
    }
    catch ([[maybe_unused]] std::exception& e) {
        return false;
    }

    boost::beast::error_code ec;
    if (_stream.socket().is_open()) {
        _stream.socket().shutdown(boost::asio::ip::tcp ::socket::shutdown_both, ec);
    }

    if (ec) {
        return false;
    }
    else {
        return true;
    }
}

bool LoggingSinkConnectionStatus::isUnreachable(const std::string& url_string)
{
    fep3::helper::Url url(url_string);
    using namespace std::chrono_literals;
    boost::asio::ip::tcp::resolver::results_type resolve_results;
    // According to the discussion on boost - users mailing list,
    //  https://www.google.de/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwibwIip88mFAxUU3gIHHVEADsIQFnoECBcQAQ&url=https%3A%2F%2Fgroups.google.com%2Fg%2Fboost-developers-archive%2Fc%2F9r0MVggfuzs&usg=AOvVaw1WDZPFQdU0VJr3CQe5fGgZ&opi=89978449
    //  resolver::cancel() is only able to cancel pending, queued resolve requests,
    //  not the one that's currently executing.
    auto resolve_fut = _resolver.async_resolve(url.host(), url.port(), boost::asio::use_future);

    try {
        resolve_results = resolve_fut.get();
    }
    catch ([[maybe_unused]] const std::exception& e) {
        return true;
    }

    bool endpoint_reachable = false;

    for (const auto res: resolve_results) {
        if (res.endpoint().protocol() == boost::asio::ip::tcp::v4()) {
            _stream.expires_after(socket_connection_timeout);
            endpoint_reachable = endpointReachable<decltype(resolve_results)>(res);
            if (endpoint_reachable) {
                break;
            }
        }
    }
    return !endpoint_reachable;
}

} // namespace fep3::native
