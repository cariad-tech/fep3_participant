/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once
#include <boost/asio.hpp>

#include <functional>
#include <thread>

namespace fep3::base {

class SingleThreadWorker {
public:
    SingleThreadWorker() : _work(std::make_shared<boost::asio::io_service::work>(_io_service))
    {
        _thr = std::thread([&] { _io_service.run(); });
    }

    ~SingleThreadWorker()
    {
        _work.reset();
        _thr.join();
    }

    void dispatch(std::function<void()> f)
    {
        boost::asio::post(_io_service, std::move(f));
    }

private:
    std::thread _thr;
    boost::asio::io_service _io_service;
    std::shared_ptr<boost::asio::io_service::work> _work;
};

} // namespace fep3::base
