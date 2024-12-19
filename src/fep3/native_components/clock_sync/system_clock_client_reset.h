/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/logging/logger_intf.h>
#include <fep3/fep3_timestamp.h>

#include <functional>
#include <memory>
#include <mutex>

namespace fep3::native {
/// @brief Class for caching the reset event from local_system_realtime clock
/// If start is not called:
///     Caching the event reset with the timestamp new_time
///     At the start call clock is reset and a warning is logged
/// If start was called
///     Clock is reset with the timestamp new_time.
class SystemClockClientReset {
public:
    using ResetFunction = std::function<void(fep3::arya::Timestamp)>;

    void reset(fep3::arya::Timestamp new_time, ResetFunction reset_function);

    void start(ResetFunction reset_function, fep3::ILogger* _logger);

    void stop();

private:
    std::optional<fep3::arya::Timestamp> _reset_time;
    bool _started = false;
    std::mutex _timestamp_mutex;
};

} // namespace fep3::native
