/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "variant_handling/clock_variant_handling.h"

#include <fep3/components/clock/clock_registry_intf.h>
#include <fep3/components/logging/easy_logger.h>

#include <mutex>
#include <optional>
#include <vector>

namespace fep3 {

namespace arya {
class ILogger;
class ILoggingService;
class IComponents;
} // namespace arya

namespace native {

/**
 * @brief Native implementation of a clock registry.
 */
class ClockRegistry : public base::EasyLogging {
public:
    fep3::Result registerNativeClocks(
        const std::vector<std::shared_ptr<fep3::experimental::IClock>>& native_clocks);

public:
    template <typename T>
    fep3::Result registerClock(std::shared_ptr<T> clock)
    {
        if (!clock) {
            auto result = CREATE_ERROR_DESCRIPTION(
                ERR_POINTER,
                a_util::strings::format(
                    "Registering clock failed. The clock to be registered is invalid.")
                    .c_str());

            FEP3_LOG_RESULT(result);
            return result;
        }

        const auto clock_name = clock->getName();

        std::unique_lock<std::mutex> lock_guard(_clock_registry_mutex);

        if (0 != _clocks.count(clock_name)) {
            auto result = CREATE_ERROR_DESCRIPTION(
                fep3::ERR_INVALID_ARG,
                a_util::strings::format(
                    "Registering clock failed. A clock with the name %s is already registered.",
                    clock_name.c_str())
                    .c_str());

            lock_guard.unlock();
            FEP3_LOG_RESULT(result);
            return result;
        }

        _clocks.emplace(clock_name, clock);

        lock_guard.unlock();
        FEP3_LOG_DEBUG(a_util::strings::format("Clock '%s' registered at the clock service.",
                                               clock->getName().c_str()));

        return {};
    }

    fep3::Result unregisterClock(const std::string& clock_name);
    std::list<std::string> getClockNames() const;

    template <typename T>
    std::shared_ptr<T> findClock(const std::string& clock_name) const
    {
        std::lock_guard<std::mutex> lock_guard(_clock_registry_mutex);
        const auto clocks_iterator = _clocks.find(clock_name);

        if (clocks_iterator != _clocks.end()) {
            return clocks_iterator->second.getClockPointer<T>();
        }

        return nullptr;
    }

    std::optional<GenericClockAdapter> getClockAdapter(const std::string& clock_name)
    {
        std::lock_guard<std::mutex> lock_guard(_clock_registry_mutex);
        const auto clocks_iterator = _clocks.find(clock_name);

        if (clocks_iterator != _clocks.end()) {
            return clocks_iterator->second;
        }

        return {};
    }

private:
    mutable std::mutex _clock_registry_mutex;
    std::map<std::string, GenericClockAdapter> _clocks;

    std::vector<std::string> _native_clocks;
};

} // namespace native
} // namespace fep3
