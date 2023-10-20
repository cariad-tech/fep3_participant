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

#include "clock_event_sink.h"
#include "external_clock.h"

#include <fep3/components/clock/clock_base.h>

#include <mutex>

namespace fep3 {
namespace native {

/**
 * @brief Native implementation of a system clock.
 */
class SystemClock : public fep3::experimental::IClock {
public:
    using SteadyClock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<SteadyClock>;
    using Timestamp = fep3::arya::Timestamp;

    /**
     * CTOR
     */
    SystemClock(const std::string& name = FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME,
                std::unique_ptr<IExternalClock> external_clock = std::make_unique<ExternalClock>());

    /**
     * DTOR
     */
    ~SystemClock() = default;

    std::string getName() const override
    {
        return _name;
    }

    fep3::arya::IClock::ClockType getType() const override
    {
        return fep3::arya::IClock::ClockType::continuous;
    }

    void reset(fep3::arya::Timestamp new_time) override
    {
        setResetTime(resetTime(new_time));
    }

    fep3::arya::Timestamp getTime() const override;

    void start(const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink) override;

    void stop() override;

public:
    /**
     * @brief Return the passed time since the last reset.
     *
     * @return Passed time
     */
    Timestamp getNewTime() const;

    /**
     * @brief Reset the clock and return the reset time.
     *
     * @param[in] new_time The new time of the clock
     * @return Passed time
     */
    Timestamp resetTime(Timestamp new_time);

private:
    /**
     * @brief Set a new time for the clock.
     * Reset the clock if 'setNewTime' has been called for the first time
     * or if @p new_time is smaller than the old time
     *
     * @param[in] new_time The new time of the clock
     */

    void setNewTime(fep3::arya::Timestamp new_time) const;
    /**
     * @brief Set a new time for the clock and emit time reset events via the event sink
     *
     * @param[in] new_time The new time of the clock
     */
    void setResetTime(const fep3::arya::Timestamp new_time) const;

private:
    /// Clock offset which is set during reset calls.
    const std::string _name;
    /// External clock for timing
    std::unique_ptr<IExternalClock> _external_clock;
    mutable TimePoint _current_offset;
    mutable std::atomic_bool _updated;
    /// determine wether the clock is started or not
    mutable std::atomic_bool _started;
    const Timestamp _initial_time;
    mutable ClockEventSink _clock_event_sink;
    mutable std::atomic_bool _time_resetting;

    mutable std::recursive_mutex _mutex;
};

} // namespace native
} // namespace fep3
