/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <fep3/native_components/logging/sinks/logging_sink_service_connection_status.h>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thread>

class TestLoggingSinkConnectionStatus : public ::testing::Test {
public:
    TestLoggingSinkConnectionStatus()
        : _acceptor(_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0))
    {
    }

protected:
    void SetUp() override
    {
        _port = _acceptor.local_endpoint().port();
    }
    boost::asio::io_service _service;
    boost::asio::ip::tcp::acceptor _acceptor;
    unsigned short _port;
};

TEST_F(TestLoggingSinkConnectionStatus, getUnreachable__successfull)
{
    fep3::native::LoggingSinkConnectionStatus connection_status;
    auto dummy_url_1 = "http://badUrl:1";
    auto reachable_url = "http://127.0.0.1:" + std::to_string(_port);
    auto dummy_url_2 = "http://1.1.1.1:3";
    auto reachable_url_2 = "http://localhost:" + std::to_string(_port);

    std::vector<std::string> urls = {dummy_url_1, reachable_url, dummy_url_2, reachable_url_2};
    auto res = connection_status.getUnreachable(urls);
    ASSERT_THAT(res, ::testing::UnorderedElementsAreArray({dummy_url_1, dummy_url_2}));
}
