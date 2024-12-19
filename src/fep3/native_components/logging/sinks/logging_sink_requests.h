/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <mutex>
#include <queue>
#include <string>
#include <variant>

namespace fep3::native {

struct RegisterSinkRequest {
    std::string address;
    std::string logger_name_filter;
    int severity;
};
struct UnRegisterSinkRequest {
    std::string address;
};

using SinkRequest = std::variant<RegisterSinkRequest, UnRegisterSinkRequest>;

class SinkRequestQueue {
public:
    void addRegisterSinkRequest(const std::string& address,
                                const std::string& logger_name_filter,
                                int severity)
    {
        std::scoped_lock lock(_mutex);
        _requests.emplace(RegisterSinkRequest{address, logger_name_filter, severity});
    }

    void addUnRegisterSinkRequest(const std::string& address)
    {
        std::scoped_lock lock(_mutex);
        _requests.emplace(UnRegisterSinkRequest{address});
    }

    std::queue<SinkRequest> popAll()
    {
        std::scoped_lock lock(_mutex);
        std::queue<SinkRequest> ret;
        std::swap(ret, _requests);
        return ret;
    }

private:
    std::mutex _mutex;
    std::queue<SinkRequest> _requests;
};

} // namespace fep3::native
