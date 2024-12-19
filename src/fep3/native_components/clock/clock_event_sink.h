/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/clock/clock_service_intf.h>

#include <atomic>
#include <mutex>

namespace fep3 {
namespace native {
/**
 * @brief Helper struct containing the current clock time, event sink and
 *        the corresponding mutex.
 */
struct ClockEventSink {
    /// the current time of this clock
    fep3::arya::Timestamp _current_time;
    /// event sink given on start call which receives time events
    std::weak_ptr<fep3::experimental::IClock::IEventSink> _event_sink;
    /// recursive mutex
    mutable std::recursive_mutex _mutex;

    /**
     * @brief set current time
     *
     * @param[in] time
     */
    void setCurrentTime(fep3::arya::Timestamp time)
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_mutex);
        _current_time = time;
    }

    /**
     * @brief get current time
     *
     * @return Timestamp
     */
    const fep3::arya::Timestamp getCurrentTime() const
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_mutex);
        return _current_time;
    }

    /**
     * @brief set event sink
     *
     * @param[in] event_sink
     */
    void setEventSink(std::weak_ptr<fep3::experimental::IClock::IEventSink> event_sink)
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_mutex);
        _event_sink = event_sink;
    }

    /**
     * @brief set event sink
     *
     * @return EventSink
     */
    const std::shared_ptr<fep3::experimental::IClock::IEventSink> getEventSink()
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_mutex);
        return _event_sink.lock();
    }
};

} // namespace native
} // namespace fep3
