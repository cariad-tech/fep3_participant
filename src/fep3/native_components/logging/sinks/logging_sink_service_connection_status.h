/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/range/adaptors.hpp>

#include <thread>

namespace fep3::native {

class LoggingSinkConnectionStatus {
public:
    LoggingSinkConnectionStatus();

    ~LoggingSinkConnectionStatus();

    template <typename UrlContainer>
    auto getUnreachable(const UrlContainer& urls)
    {
        return boost::adaptors::filter(urls, [&](const auto& x) { return isUnreachable(x); });
    }

private:
    bool isUnreachable(const std::string& url_string);
    template <typename U, typename T>
    bool endpointReachable(const T& endpoint);
    boost::asio::io_context _ioc;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> _work;
    std::thread _context_thread;
    boost::beast::tcp_stream _stream;
    boost::asio::ip::tcp::resolver _resolver;
};
} // namespace fep3::native
