/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "clock_event_sink.h"
#include "external_clock.h"

#include <fep3/components/clock/clock_base.h>
#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/fep3_duration.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace fep3 {
namespace native {
class SimulationClock : public fep3::experimental::IClock {
public:
    using SteadyClock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<SteadyClock>;
    /**
     * @brief CTOR.
     * A @ref ClockBase is initialized with a current time of 0.
     *
     * @param[in] clock_name Name of the clock
     */

    SimulationClock(
        const std::string& name = FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME,
        std::unique_ptr<IExternalClock> external_clock = std::make_unique<ExternalClock>());
    /**
     * @brief DTOR.
     */
    ~SimulationClock();

    /**
     * @brief Update the clock configuration.
     *
     * @param[in] step_size new clock step size in nanoseconds
     * @param[in] time_factor new clock time factor
     */

public:
    // Inherited methods from IClock

    void start(const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink) override;

    void stop() override;

    std::string getName() const override
    {
        return _name;
    }

    fep3::arya::IClock::ClockType getType() const override
    {
        return fep3::arya::IClock::ClockType::discrete;
    }

    fep3::arya::Timestamp getTime() const override
    {
        return _clock_event_sink.getCurrentTime();
    }

    void reset(fep3::arya::Timestamp new_time) override;

public:
    // Its own public methods

    void updateConfiguration(Duration step_size, double time_factor);

    /**
     * @brief Get step size.
     */
    Duration getStepSize()
    {
        return _step_size;
    }

    /**
     * @brief Get time factor.
     */
    double getTimeFactor()
    {
        return _time_factor;
    }

private:
    /**
     * @brief Cyclically wait for the configured step size time interval and
     * update the clock time.
     */
    void work();

    /**
     * @brief Set a new time for the clock.
     * Emit time update events via the event sink.
     * Reset the clock if @ref setNewTime has been called for the first time
     * or if @p new_time is smaller than the current time
     *
     * @param[in] new_time The new time of the clock
     * @param[in] next_time The new next time of the clock
     */
    void setNewTime(const fep3::arya::Timestamp new_time, const fep3::arya::Timestamp next_time);

    /**
     * @brief Set a new time for the clock and emit time reset events via the
     * event sink.
     *
     * @param[in] new_time The new time of the clock
     */
    void setResetTime(const fep3::arya::Timestamp new_time);

private:
    /// Clock name
    const std::string _name;

    /// holds the event sink and current time with the mutex.
    mutable ClockEventSink _clock_event_sink;

    /// Current simulation time in nanoseconds
    Timestamp _simulation_time;
    /// Duration of a single discrete time step in nanoseconds
    Duration _step_size;
    /// Factor to control the relation between simulated time and system time
    double _time_factor;
    /// Thread to update the clock time
    std::thread _worker;
    /// Flag to mark if the clock has been reset
    std::atomic_bool _time_reset;
    /// Stop flag for condition variable
    std::atomic_bool _stop;
    /// Mutex for updating clock
    std::mutex _mutex;
    /// Condition variable
    std::condition_variable _cycle_wait_cv;
    /// External clock for timing
    std::unique_ptr<IExternalClock> _external_clock;

    const fep3::arya::Timestamp _initial_time = fep3::arya::Timestamp{0};
};

} // namespace native
} // namespace fep3
