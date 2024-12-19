/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "rpc_clock_service.h"

#include "clock_service.h"

namespace fep3 {
namespace rpc {

RPCClockService::RPCClockService(fep3::experimental::IClockService& service) : _service(service)
{
}

std::string RPCClockService::getClockNames()
{
    auto return_list = _service.getClockNames();
    auto first = true;
    std::string return_string;
    for (auto& clockname: return_list) {
        if (first) {
            return_string = clockname;
            first = false;
        }
        else {
            return_string += "," + clockname;
        }
    }
    return return_string;
}

std::string RPCClockService::getMainClockName()
{
    return _service.getMainClockName();
}

std::string RPCClockService::getTime(const std::string& clock_name)
{
    if (clock_name.empty()) {
        return std::to_string(_service.getTime().count());
    }
    else {
        auto current_time = _service.getTime(clock_name);
        if (current_time.has_value()) {
            return std::to_string(current_time.value().count());
        }
        else {
            return "-1";
        }
    }
}

int RPCClockService::getType(const std::string& clock_name)
{
    if (clock_name.empty()) {
        return static_cast<int>(_service.getType());
    }
    else {
        auto clock_type = _service.getType(clock_name);
        if (clock_type.has_value()) {
            return static_cast<int>(clock_type.value());
        }
        else {
            return -1;
        }
    }
}

} // namespace rpc
} // namespace fep3
