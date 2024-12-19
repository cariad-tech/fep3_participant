/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "clock_registry.h"

#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/fep3_errors.h>

#include <a_util/strings/strings_format.h>

#include <algorithm>
#include <iterator>

namespace fep3 {
namespace native {

fep3::Result ClockRegistry::registerNativeClocks(
    const std::vector<std::shared_ptr<fep3::experimental::IClock>>& native_clocks)
{
    if (!_native_clocks.empty()) {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG,
                                 "Registering native clocks can only be done once");
    }

    for (const auto& clock: native_clocks) {
        _native_clocks.push_back(clock->getName());
        auto result = registerClock(clock);
        if (!result) {
            return result;
        }
    }

    return {};
}

fep3::Result ClockRegistry::unregisterClock(const std::string& name)
{
    if (std::find(_native_clocks.begin(), _native_clocks.end(), name) != _native_clocks.end()) {
        auto result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            a_util::strings::format(
                "Unregistering clock failed. The native clock %s can not be unregistered.",
                name.c_str())
                .c_str());

        FEP3_LOG_RESULT(result);
        return result;
    }

    std::unique_lock<std::mutex> lock_guard(_clock_registry_mutex);

    if (_clocks.count(name) == 0) {
        auto result = CREATE_ERROR_DESCRIPTION(
            fep3::ERR_INVALID_ARG,
            a_util::strings::format(
                "Unregistering clock failed. A clock with the name '%s' is not registered.",
                name.c_str())
                .c_str());

        lock_guard.unlock();
        FEP3_LOG_RESULT(result);
        return result;
    }

    _clocks.erase(name);

    lock_guard.unlock();
    FEP3_LOG_DEBUG(
        a_util::strings::format("Clock '%s' unregistered from clock service.", name.c_str()));

    return {};
}

std::list<std::string> ClockRegistry::getClockNames() const
{
    std::list<std::string> clock_list;
    std::lock_guard<std::mutex> lock_guard(_clock_registry_mutex);
    std::transform(_clocks.begin(),
                   _clocks.end(),
                   std::back_inserter(clock_list),
                   [&](auto map_entry) { return map_entry.second.getName(); });

    return clock_list;
}

} // namespace native
} // namespace fep3
