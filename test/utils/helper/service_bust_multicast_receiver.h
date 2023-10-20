
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

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

class MulticastReceiver {
public:
    MulticastReceiver(const boost::asio::ip::address& listen_address,
                      const boost::asio::ip::address& multicast_address,
                      std::function<void(const std::vector<char>&)> receive_callback,
                      unsigned short port)
        : socket_(io_service), _receive_callback(receive_callback)
    {
        // Create the socket so that multiple may be bound to the same address.
        boost::asio::ip::udp::endpoint listen_endpoint(listen_address, port);

        socket_.open(listen_endpoint.protocol());
        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket_.bind(listen_endpoint);
        _port = socket_.local_endpoint().port();

        // Join the multicast group.
        socket_.set_option(boost::asio::ip::multicast::join_group(multicast_address));

        socket_.async_receive_from(boost::asio::buffer(data_, max_length),
                                   sender_endpoint_,
                                   boost::bind(&MulticastReceiver::handle_receive_from,
                                               this,
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
    {
        if (!error) {
            std::vector<char> rec_vector(&(data_[0]), std::next(data_, bytes_recvd));
            _receive_callback(rec_vector);
            socket_.async_receive_from(boost::asio::buffer(data_, max_length),
                                       sender_endpoint_,
                                       boost::bind(&MulticastReceiver::handle_receive_from,
                                                   this,
                                                   boost::asio::placeholders::error,
                                                   boost::asio::placeholders::bytes_transferred));
        }
    }

    void run()
    {
        run_thread = std::thread([&]() { io_service.run(); });
    }
    void stop()
    {
        socket_.close();
        io_service.stop();
        run_thread.join();
    }
    unsigned short get_port()
    {
        return _port;
    };

private:
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    std::thread run_thread;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
    std::function<void(const std::vector<char>&)> _receive_callback;
    unsigned short _port;
};