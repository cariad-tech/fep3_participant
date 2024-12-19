/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "system_clock_client_reset.h"

#include <fep3/components/clock_sync/clock_sync_service_intf.h>
#include <fep3/components/logging/easy_logger.h>

namespace fep3::native {

void SystemClockClientReset::reset(fep3::arya::Timestamp new_time, ResetFunction reset_function)
{
    std::scoped_lock lock(_timestamp_mutex);
    _reset_time = new_time;
    if (_started) {
        reset_function(new_time);
    }
}

void SystemClockClientReset::start(ResetFunction reset_function, fep3::ILogger* logger)
{
    std::scoped_lock lock(_timestamp_mutex);
    if (_reset_time) {
        FEP3_LOGGER_LOG_WARNING(
            logger,
            std::string(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND) +
                " clock received reset event before the clock start,"
                "set the start priority of the timing_master lower as the client clocks, "
                "timing master should be started last");
        reset_function(_reset_time.value());
    }
    _started = true;
}

void SystemClockClientReset::stop()
{
    std::scoped_lock lock(_timestamp_mutex);
    _reset_time.reset();
    _started = false;
}

} // namespace fep3::native
