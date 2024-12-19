/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "simulation_clock.h"

#include <cmath>
#include <iostream>

const constexpr fep3::Timestamp simulation_time_start_value{0};

namespace fep3 {
namespace native {
SimulationClock::SimulationClock(const std::string& name,
                                 std::unique_ptr<IExternalClock> external_clock)
    : _name(name),
      _clock_event_sink{Timestamp{0}, {}, {}},
      _simulation_time(simulation_time_start_value),
      _step_size(FEP3_CLOCK_SIM_TIME_STEP_SIZE_DEFAULT_VALUE),
      _time_factor(FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE),
      _external_clock(std::move(external_clock))
{
}

SimulationClock::~SimulationClock()
{
    stop();
}

void SimulationClock::work()
{
    while (!_stop) {
        auto start_t = _external_clock->now();

        try {
            setNewTime(_simulation_time, _simulation_time + _step_size);
        }
        catch (std::exception& exception) {
            std::cout << "Caught an exception during update of simulation time: "
                      << exception.what() << std::endl;
        }

        // If time factor is configured to be '0,0', we do not wait between time
        // steps
        if (FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE != _time_factor) {
            auto actual_step_size = fep3::Duration{std::lround(_step_size.count() / _time_factor)};
            TimePoint next_time_point = start_t + actual_step_size;

            if (next_time_point > _external_clock->now()) {
                _external_clock->waitUntil([&]() {
                    // Wait if the system timestamp for the next discrete time
                    // step is not reached yet
                    std::unique_lock lk(_mutex);
                    _cycle_wait_cv.wait_until(lk, next_time_point, [&]() { return _stop == true; });
                });
            }
        }

        {
            std::lock_guard lk(_mutex);
            _simulation_time += _step_size;
        }
    }
}

void SimulationClock::updateConfiguration(const Duration step_size, const double time_factor)
{
    std::lock_guard lk(_mutex);
    _step_size = step_size;
    _time_factor = time_factor;
}

void SimulationClock::start(const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink)
{
    {
        std::lock_guard<std::mutex> lk(_mutex);
        _time_reset = false;
    }

    _clock_event_sink.setEventSink(event_sink);

    reset(_initial_time);

    {
        std::lock_guard<std::mutex> lk(_mutex);
        _simulation_time = Timestamp{0};
        _stop = false;
    }
    _worker = std::thread([this] { work(); });
}

void SimulationClock::reset(fep3::arya::Timestamp new_time)
{
    {
        std::lock_guard<std::mutex> lk(_mutex);
        _time_reset = false;
    }
    setResetTime(new_time);
}

void SimulationClock::stop()
{
    {
        std::lock_guard<std::mutex> lk(_mutex);
        if (_stop) {
            return;
        }
        _stop = true;
    }

    // cancel the waiting
    _external_clock->notify([&]() { _cycle_wait_cv.notify_one(); });

    if (_worker.joinable()) {
        _worker.join();
    }

    _clock_event_sink.setEventSink(std::weak_ptr<fep3::experimental::IClock::IEventSink>());

    {
        std::lock_guard<std::mutex> lk(_mutex);
        _time_reset = false;
    }
}

void SimulationClock::setNewTime(const fep3::arya::Timestamp new_time,
                                 const fep3::arya::Timestamp next_time)
{
    const auto old_time = _clock_event_sink.getCurrentTime();

    if (!_time_reset) {
        {
            std::lock_guard<std::mutex> lk(_mutex);
            _time_reset = true;
        }
        setResetTime(new_time);
    }
    else if (new_time < old_time) {
        setResetTime(new_time);
    }
    else {
        auto event_sink_pointer = _clock_event_sink.getEventSink();

        if (event_sink_pointer) {
            event_sink_pointer->timeUpdateBegin(old_time, new_time);
        }

        _clock_event_sink.setCurrentTime(new_time);

        if (event_sink_pointer) {
            event_sink_pointer->timeUpdating(new_time, next_time);
        }
        if (event_sink_pointer) {
            event_sink_pointer->timeUpdateEnd(new_time);
        }
    }
}

void SimulationClock::setResetTime(const fep3::arya::Timestamp new_time)
{
    const auto old_time = _clock_event_sink.getCurrentTime();
    auto _event_sink_pointer = _clock_event_sink.getEventSink();

    if (_event_sink_pointer) {
        _event_sink_pointer->timeResetBegin(old_time, new_time);
    }

    {
        std::lock_guard<std::mutex> lk(_mutex);
        _time_reset = true;
    }

    _clock_event_sink.setCurrentTime(new_time);

    if (_event_sink_pointer) {
        _event_sink_pointer->timeResetEnd(new_time);
    }
}

} // namespace native
} // namespace fep3
