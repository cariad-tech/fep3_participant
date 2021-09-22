/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#include "local_clock_registry.h"

#include <algorithm>
#include <iterator>

#include <a_util/strings/strings_format.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/clock/clock_service_intf.h>
#include "fep3/components/logging/easy_logger.h"

namespace fep3
{
namespace native
{

fep3::Result ClocksSynchronized::registerClock(const std::shared_ptr<IClock>& clock)
{
    const auto clock_name = clock->getName();

    std::lock_guard<std::mutex> lock_guard(_mutex);

    if (0 != _clocks.count(clock_name))
    {
        return CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                "Registering clock failed. A clock with the name %s is already registered.",
                clock_name.c_str())
            .c_str());
    }

    _clocks.emplace(clock_name, clock);

    return {};
}

fep3::Result ClocksSynchronized::unregisterClock(const std::string& name)
{
    std::lock_guard<std::mutex> lock_guard(_mutex);

    if (_clocks.count(name) == 0)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                "Unregistering clock failed. A clock with the name '%s' is not registered.",
                name.c_str())
            .c_str());

        return result;
    }

    _clocks.erase(name);

    return {};
}

std::list<std::string> ClocksSynchronized::getClockNames() const
{
    std::list<std::string> clock_list;

    std::lock_guard<std::mutex> lock_guard(_mutex);

    std::transform(
        _clocks.begin(),
        _clocks.end(),
        std::back_inserter(clock_list),
        [](auto map_entry)
    {
        return map_entry.second->getName();
    });

    return clock_list;
}

std::shared_ptr<IClock> ClocksSynchronized::findClock(const std::string& name) const
{
    std::lock_guard<std::mutex> lock_guard(_mutex);

    const auto clocks_iterator = _clocks.find(name);
    if (clocks_iterator != _clocks.end())
    {
        return clocks_iterator->second;
    }

    return nullptr;
}

fep3::Result LocalClockRegistry::registerClock(const std::shared_ptr<IClock>& clock)
{
    if (!clock)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_POINTER,
            a_util::strings::format(
                "Registering clock failed. The clock to be registered is invalid.")
            .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    const auto result = _clocks_synchronized.registerClock(clock);

    if (isFailed(result))
    {
        FEP3_LOG_RESULT(result);

        return result;
    }

    FEP3_LOG_DEBUG(a_util::strings::format(
        "Clock '%s' registered at the clock service.",
        clock->getName().c_str()));

    return {};
}

fep3::Result LocalClockRegistry::unregisterClock(const std::string& name)
{
    if ((FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME == name) ||
        (FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME == name))
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                "Unregistering clock failed. The clocks %s or %s can not be unregistered.",
                FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME,
                FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME)
            .c_str());

        FEP3_LOG_RESULT(result);

        return result;
    }

    const auto result = _clocks_synchronized.unregisterClock(name);
    if (isFailed(result))
    {
        FEP3_LOG_RESULT(result);

        return result;
    }

    FEP3_LOG_DEBUG(a_util::strings::format(
        "Clock '%s' unregistered from clock service.", name.c_str()));

    return {};
}

std::list<std::string> LocalClockRegistry::getClockNames() const
{
    return _clocks_synchronized.getClockNames();
}

std::shared_ptr<IClock> LocalClockRegistry::findClock(const std::string& name) const
{
    return _clocks_synchronized.findClock(name);
}

} // namespace native
} // namespace fep3
