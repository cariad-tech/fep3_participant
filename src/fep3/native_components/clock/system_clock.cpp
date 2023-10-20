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

#include "system_clock.h"

#include <fep3/components/clock/clock_service_intf.h>

#include <a_util/system/system.h>

namespace fep3 {
namespace native {
SystemClock::SystemClock(const std::string& name, std::unique_ptr<IExternalClock> external_clock)
    : _name(name),
      _external_clock(std::move(external_clock)),
      _current_offset(_external_clock->now()),
      _initial_time{Timestamp{0}},
      _updated(false),
      _started(false),
      _clock_event_sink{fep3::arya::Timestamp(0), {}, {}},
      _time_resetting(false)
{
}

Timestamp SystemClock::getTime() const
{
    setNewTime(getNewTime());
    return _clock_event_sink.getCurrentTime();
}

Timestamp SystemClock::getNewTime() const
{
    if (_started) {
        return Timestamp{_external_clock->now() - _current_offset};
    }
    else {
        return Timestamp{0};
    }
}

Timestamp SystemClock::resetTime(const fep3::arya::Timestamp new_time)
{
    _current_offset = _external_clock->now() - new_time;
    return getNewTime();
}

void SystemClock::setNewTime(const fep3::arya::Timestamp new_time) const
{
    if (!_updated) {
        _updated = true;
        setResetTime(new_time);
    }
    _updated = true;

    _clock_event_sink.setCurrentTime(new_time);
}

void SystemClock::setResetTime(const fep3::arya::Timestamp new_time) const
{
    // avoid recursive calls in case event_sink_pointer calls again getTime
    // that results in a new setResetTime calls
    {
        std::unique_lock<std::recursive_mutex> lk(_mutex);
        if (_time_resetting) {
            return;
        }
        else {
            _time_resetting = true;
        }
    }

    const auto old_time = _clock_event_sink.getCurrentTime();
    auto event_sink_pointer = _clock_event_sink.getEventSink();

    if (event_sink_pointer) {
        event_sink_pointer->timeResetBegin(old_time, new_time);
    }

    _updated = true;

    _clock_event_sink.setCurrentTime(new_time);

    if (event_sink_pointer) {
        event_sink_pointer->timeResetEnd(new_time);
    }

    {
        std::unique_lock<std::recursive_mutex> lk(_mutex);
        _time_resetting = false;
    }
}

void SystemClock::start(const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink)
{
    _updated = false;
    {
        _clock_event_sink.setEventSink(event_sink);
    }
    _started = true;
    reset(_initial_time);
}

void SystemClock::stop()
{
    _started = false;
    {
        _clock_event_sink.setEventSink(std::weak_ptr<fep3::experimental::IClock::IEventSink>());
    }
    _updated = false;
}

} // namespace native
} // namespace fep3
