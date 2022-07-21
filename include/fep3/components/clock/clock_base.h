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


#pragma once

#include <mutex>
#include <atomic>

#include <fep3/components/clock/clock_service_intf.h>

namespace fep3
{
namespace base
{
namespace arya
{
/**
* @brief Base implementation of a Clock
*
*/
class ClockBase : public fep3::arya::IClock
{
public:
    /**
    * @brief CTOR.
    * A @ref ClockBase is initialized with a current time of 0.
    *
    * @param[in] clock_name Name of the clock
    */
    explicit ClockBase(const std::string& clock_name)
        : _clock_name(clock_name)
        , _event_sink_and_time(fep3::arya::Timestamp(0))
        , _updated(false)
        , _started(false)
    {
    }

    /**
    * @brief Delete copy CTOR
    *
    * @param[in] other ClockBase to copy from
    */
    ClockBase(const ClockBase& other) = delete;

    /**
    * @brief Delete move CTOR
    *
    * @param[in] other ClockBase to move from
    */
    ClockBase(ClockBase&& other) = delete;

    /**
    * @brief Delete copy assignment operator
    *
    * @param[in] other ClockBase to copy from
    * @return ClockBase
    */
    ClockBase& operator=(const ClockBase& other) = delete;

    /**
    * @brief Delete move assignment operator
    *
    * @param[in] other ClockBase to move from
    * @return ClockBase
    */
    ClockBase& operator=(ClockBase&& other) = delete;

    /**
    * @brief DTOR
    */
    virtual ~ClockBase() = default;

public:
   /**
    * @copydoc IClock::getName
    */
    std::string getName() const override
    {
        return _clock_name;
    }

    /**
    * @copydoc IClock::start
    */
    void start(const std::weak_ptr<fep3::arya::IClock::IEventSink>& event_sink) override
    {
        _updated = false;
        {
            std::lock_guard<std::recursive_mutex> lock_guard(_event_sink_and_time._mutex_sink_and_time);
            _event_sink_and_time._event_sink = event_sink;
        }
        _started = true;
        reset(fep3::arya::Timestamp(0));
    }

    /**
    * @copydoc IClock::stop
    */
    void stop() override
    {
        _started = false;
        {
            std::lock_guard<std::recursive_mutex> lock_guard(_event_sink_and_time._mutex_sink_and_time);
            _event_sink_and_time._event_sink.reset();
        }
        _updated = false;
    }

protected:
    /**
    * @brief Helper struct containing the current clock time, event sink and
    *        the corresponding mutex.
    */
    struct EventSinkAndTime
    {
        /**
        * @brief CTOR
        *
        * @param[in] timestamp Starting times of the clock
        */
        EventSinkAndTime(fep3::arya::Timestamp timestamp)
            : _current_time(timestamp)
        {}
        ///event sink given on start call which receives time events
        std::weak_ptr<fep3::arya::IClock::IEventSink> _event_sink;
        ///the current time of this clock
        fep3::arya::Timestamp _current_time;
        ///recursive mutex
        std::recursive_mutex _mutex_sink_and_time;
    };

    ///members have to be secured versus multi threaded access in derived classes
    ///the name of this clock
    std::string _clock_name;
    ///holds the event sink and current time with the mutex.
    mutable EventSinkAndTime _event_sink_and_time;
    ///determine if the clock was set by any setNewTime call
    mutable std::atomic_bool _updated;
    ///determine wether the clock is started or not
    mutable std::atomic_bool _started;
};

/**
* @brief Base implementation for a continuous clock which will automatically
*     call the @ref arya::IClock::IEventSink for you.
* You have to implement following functionality:
* @li CTOR
* @li @ref ContinuousClock::getNewTime
* @li @ref ContinuousClock::resetTime
*/
class ContinuousClock : public arya::ClockBase
{
public:
    /**
    * @brief CTOR
    *
    * @param[in] name Name of the clock
    */
    ContinuousClock(const std::string& name)
        : arya::ClockBase(name)
        , _time_reset_in_progress{}
    {
    }

protected:
    /**
    * @brief Receive a new Timestamp from the continuous clock
    *
    * @remark @p Override this function to implement a custom continuous clock
    *
    * @return The new time
    */
    virtual fep3::arya::Timestamp getNewTime() const = 0;

    /**
    * @brief Reset the clock
    *
    * @remark @p Override this function to implement a custom continuous clock
    *
    * @param[in] new_time The new time of the clock
    * @return The timestamp to which the clock has been reset to
    */
    virtual fep3::arya::Timestamp resetTime(fep3::arya::Timestamp new_time) = 0;

protected:
    /**
    * @brief Get the current clock time
    *
    * @return The current time of the clock
    */
    fep3::arya::Timestamp getTime() const override
    {
        setNewTime(getNewTime());

        std::lock_guard<std::recursive_mutex> lock_guard(_event_sink_and_time._mutex_sink_and_time);
        return _event_sink_and_time._current_time;
    }

    /**
    * @brief Get the clock type
    *
    * @return The clock type
    */
    fep3::arya::IClock::ClockType getType() const override
    {
        return fep3::arya::IClock::ClockType::continuous;
    }

    /**
    * @brief Reset the clock time
    *
    * @param[in] new_time The new time of the clock
    */
    void reset(fep3::arya::Timestamp new_time) override
    {
         setResetTime(resetTime(new_time));
    }

private:
    /**
    * @brief Helper struct containing the flag for the time reset and
    *        the corresponding mutex.
    */
    struct TimeResetFlag
    {
        ///flag indicating if a clock reset is currently ongoing.
        bool _flag = false;
        ///flag mutex.
        std::recursive_mutex _mutex;
    };

    /**
    * @brief Set a new time for the clock.
    * Reset the clock if 'setNewTime' has been called for the first time
    * or if @p new_time is smaller than the old time
    *
    * @param[in] new_time The new time of the clock
    */
    void setNewTime(fep3::arya::Timestamp new_time) const
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_event_sink_and_time._mutex_sink_and_time);
        const auto old_time = _event_sink_and_time._current_time;
        lock_guard.unlock();

        if (!_updated)
        {
            _updated = true;
            setResetTime(new_time);
        }
        _updated = true;

        lock_guard.lock();
        _event_sink_and_time._current_time = new_time;
        lock_guard.unlock();
    }

    /**
    * @brief Set a new time for the clock and emit time reset events via the event sink
    *
    * @param[in] new_time The new time of the clock
    */
    void setResetTime(const fep3::arya::Timestamp new_time) const
    {
        // avoid recursive calls in case event_sink_pointer calls again getTime
        // that results in a new setResetTime calls
        {
            std::unique_lock<std::recursive_mutex> lock_guard_reset(_time_reset_in_progress._mutex);
            if (_time_reset_in_progress._flag)
            {
                return;
            }
            else
            {
                _time_reset_in_progress._flag = true;
            }
        }

        std::unique_lock<std::recursive_mutex> lock_guard(_event_sink_and_time._mutex_sink_and_time);
        const auto old_time = _event_sink_and_time._current_time;
        auto event_sink_pointer = _event_sink_and_time._event_sink.lock();
        lock_guard.unlock();

        if (event_sink_pointer)
        {
            event_sink_pointer->timeResetBegin(old_time, new_time);
        }

        _updated = true;

        lock_guard.lock();
        _event_sink_and_time._current_time = new_time;
        lock_guard.unlock();

        if (event_sink_pointer)
        {
            event_sink_pointer->timeResetEnd(new_time);
        }

        {
            std::unique_lock<std::recursive_mutex> lock_guard_reset(_time_reset_in_progress._mutex);
            _time_reset_in_progress._flag = false;
        }
    }

    mutable TimeResetFlag _time_reset_in_progress;
};

/**
* @brief Base implementation for a discrete clock which will automatically
*        call the IClock::IEventSink for you.
* You have to implement following functionality:
* @li CTOR
*
* While using you only call DiscreteClock::setNewTime and DiscreteClock::setResetTime
*/
class DiscreteClock : public arya::ClockBase
{
public:
    /**
    * @brief CTOR
    *
    * @param[in] name Name of the clock
    */
    DiscreteClock(const std::string& name) : arya::ClockBase(name)
    {
    }

protected:
    /**
    * @brief Get the current clock time
    *
    * @return The current clock time
    */
    fep3::arya::Timestamp getTime() const override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_event_sink_and_time._mutex_sink_and_time);
        return _event_sink_and_time._current_time;
    }

    /**
    * @brief Reset the clock time
    *
    * @param[in] new_time The new time of the clock
    */
    void reset(fep3::arya::Timestamp new_time) override
    {
        _updated = true;

        setResetTime(new_time);
    }

    /**
    * @brief Get the clock type
    *
    * @return The clock type
    */
    fep3::arya::IClock::ClockType getType() const override
    {
        return fep3::arya::IClock::ClockType::discrete;
    }

public:
    /**
    * @brief Set a new time for the clock.
    * Emit time update events via the event sink.
    * Reset the clock if @ref setNewTime has been called for the first time
    * or if @p new_time is smaller than the current time
    *
    * @param[in] new_time The new time of the clock
    * @param[in] send_update_before_after Flag indicates whether @ref IClock::IEventSink::timeUpdateBegin
    *                                 and @ref IClock::IEventSink::timeUpdateEnd events should be emitted.
    *                                 The event @ref IClock::IEventSink::timeUpdating is always emitted.
    */
    void setNewTime(const fep3::arya::Timestamp new_time, const bool send_update_before_after) const
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_event_sink_and_time._mutex_sink_and_time);

        const auto old_time = _event_sink_and_time._current_time;
        lock_guard.unlock();

        if (!_updated)
        {
            _updated = true;
            setResetTime(new_time);
        }
        else if (new_time < old_time)
        {
            setResetTime(new_time);
        }
        else
        {
            lock_guard.lock();
            auto event_sink_pointer =_event_sink_and_time._event_sink.lock();
            lock_guard.unlock();

            if (event_sink_pointer && send_update_before_after)
            {
                event_sink_pointer->timeUpdateBegin(old_time, new_time);
            }

            lock_guard.lock();
            _event_sink_and_time._current_time = new_time;
            lock_guard.unlock();

            if (event_sink_pointer)
            {
                event_sink_pointer->timeUpdating(new_time);
            }
            if (event_sink_pointer && send_update_before_after)
            {
                event_sink_pointer->timeUpdateEnd(new_time);
            }
        }
    }

    /**
    * @brief Set a new time for the clock and emit time reset events via the event sink.
    *
    * @param[in] new_time The new time of the clock
    */
    void setResetTime(const fep3::arya::Timestamp new_time) const
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_event_sink_and_time._mutex_sink_and_time);

        const auto old_time = _event_sink_and_time._current_time;
        auto _event_sink_pointer = _event_sink_and_time._event_sink.lock();
        lock_guard.unlock();

        if (_event_sink_pointer)
        {
            _event_sink_pointer->timeResetBegin(old_time, new_time);
        }

        _updated = true;

        lock_guard.lock();
        _event_sink_and_time._current_time = new_time;
        lock_guard.unlock();

        if (_event_sink_pointer)
        {
            _event_sink_pointer->timeResetEnd(new_time);
        }
    }
};
} // namespace arya
using arya::ClockBase;
using arya::ContinuousClock;
using arya::DiscreteClock;
} // namespace base
} // namespace fep3

