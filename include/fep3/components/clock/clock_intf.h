/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/fep3_timestamp.h>

#include <memory>
#include <optional>
#include <string>

namespace fep3 {
namespace arya {

/**
 * @brief Interface of a clock
 */
class IClock {
public:
    /**
     * @brief Event sink to react synchronously on time reset and time update events.
     */
    class IEventSink {
    protected:
        /// DTOR
        ~IEventSink() = default;

    public:
        /**
         * @brief This event is emitted before the time is updated.
         * The @ref IClock::getTime value is still @p old_time.
         * @remark This event is only emitted by discrete clocks (@ref
         * fep3::arya::IClock::ClockType::discrete)
         *
         * @param[in] old_time The time before updating
         * @param[in] new_time The future time after updating
         */
        virtual void timeUpdateBegin(arya::Timestamp old_time, arya::Timestamp new_time) = 0;

        /**
         * @brief This event is emitted while the time is being updated.
         * @remark This event is only emitted by discrete clocks (@ref
         * fep3::arya::IClock::ClockType::discrete)
         *
         * @param[in] new_time The future time after updating
         */
        virtual void timeUpdating(arya::Timestamp new_time) = 0;

        /**
         * @brief This event is emitted after the time was updated.
         * The @ref IClock::getTime value was set to the @p new_time.
         * @remark This event is only emitted by discrete clocks (@ref
         * fep3::arya::IClock::ClockType::discrete)
         *
         * @param[in] new_time The current time after the update
         */
        virtual void timeUpdateEnd(arya::Timestamp new_time) = 0;

        /**
         * @brief This event is emitted before the time will be reset.
         * It is used to inform about time jumps to the future or the past!
         * The IClock::getTime value is still @p old_time.
         *
         * @param[in] old_time The time before resetting
         * @param[in] new_time The future time after resetting
         */
        virtual void timeResetBegin(arya::Timestamp old_time, arya::Timestamp new_time) = 0;

        /**
         * @brief This event is emitted after the time was reset.
         * The @ref IClock::getTime value was already set to @p new_time.
         *
         * @param[in] new_time The current time after the reset
         */
        virtual void timeResetEnd(arya::Timestamp new_time) = 0;
    };

    /**
     * @brief Type of a clock
     */
    enum class ClockType
    {
        /**
         * @brief A continuous clock will steadily raise the time value
         */
        continuous = 0,
        /**
         * @brief A discrete clock will jump configured or calculated time steps.
         * @remark It depends on the implementation which logical time steps are used.
         */
        discrete = 1
    };

protected:
    /// DTOR
    ~IClock() = default;

public:
    /**
     * @brief Get the Name of the clock.
     *
     * @return Name of the clock
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get the Type of the clock.
     *
     * @return The type of the clock
     * @retval ClockType::continuous The clock implementation is a continuous clock
     * @retval ClockType::discrete   The clock implementation is a discrete clock
     */
    virtual ClockType getType() const = 0;

    /**
     * @brief Get the current time of the clock.
     *
     * @return Current time of the clock
     */
    virtual arya::Timestamp getTime() const = 0;

    /**
     * @brief Reset the clock.
     * Depending on the implementation the offset will be reset.
     * @remark The @ref IEventSink::timeResetBegin and the @ref IEventSink::timeResetEnd will be
     * emitted
     *
     * @param[in] new_time The new time of the clock
     */
    virtual void reset(fep3::arya::Timestamp new_time) = 0;

    /**
     * @brief Start the clock.
     * @remark The reset events @ref IEventSink::timeResetBegin and @ref IEventSink::timeResetEnd
     *         will be emitted to @p event_sink.
     * @remark If @p event_sink is a @ref fep3::arya::IClock::ClockType::discrete also the events
     *         @ref IEventSink::timeUpdateBegin, @ref fep3::arya::IClock::IEventSink::timeUpdating,
     *         @ref IEventSink::timeUpdateEnd will be emitted to @p event_sink.
     *
     * @param[in] event_sink The event sink to emit time reset and time updating events to.
     */
    virtual void start(const std::weak_ptr<IEventSink>& event_sink) = 0;

    /**
     * @brief Stop the clock.
     * Usually the clock will reset immediately (still depending on the implementation).
     */
    virtual void stop() = 0;
};
} // namespace arya

namespace experimental {
/**
 * @brief Interface of a clock
 */
class IClock {
protected:
    /// DTOR
    ~IClock() = default;

public:
    /**
     * @brief Event sink to react synchronously on time reset and time update events.
     */
    class IEventSink {
    public:
        /// DTOR
        ~IEventSink() = default;
        /**
         * @brief This event is emitted before the time is updated.
         * The @ref IClock::getTime value is still @p old_time.
         * @remark This event is only emitted by discrete clocks (@ref
         * fep3::arya::IClock::ClockType::discrete)
         *
         * @param[in] old_time The time before updating
         * @param[in] new_time The future time after updating
         */
        virtual void timeUpdateBegin(arya::Timestamp old_time, arya::Timestamp new_time) = 0;

        /**
         * @brief This event is emitted while the time is being updated.
         * @remark This event is only emitted by discrete clocks (@ref
         * fep3::arya::IClock::ClockType::discrete)
         *
         * @param[in] new_time The future time after updating
         * @param[in] estimated_next_time The future next time after updating
         */
        virtual void timeUpdating(arya::Timestamp new_time,
                                  std::optional<arya::Timestamp> estimated_next_time) = 0;

        /**
         * @brief This event is emitted after the time was updated.
         * The @ref IClock::getTime value was set to the @p new_time.
         * @remark This event is only emitted by discrete clocks (@ref
         * fep3::arya::IClock::ClockType::discrete)
         *
         * @param[in] new_time The current time after the update
         */
        virtual void timeUpdateEnd(arya::Timestamp new_time) = 0;

        /**
         * @brief This event is emitted before the time will be reset.
         * It is used to inform about time jumps to the future or the past!
         * The IClock::getTime value is still @p old_time.
         *
         * @param[in] old_time The time before resetting
         * @param[in] new_time The future time after resetting
         */
        virtual void timeResetBegin(arya::Timestamp old_time, arya::Timestamp new_time) = 0;

        /**
         * @brief This event is emitted after the time was reset.
         * The @ref fep3::experimental::IClock::getTime value was already set to @p new_time.
         *
         * @param[in] new_time The current time after the reset
         */
        virtual void timeResetEnd(arya::Timestamp new_time) = 0;
    };

    /**
     * @brief Start the clock.
     * @remark The reset events @ref IEventSink::timeResetBegin and @ref IEventSink::timeResetEnd
     *         will be emitted to @p event_sink.
     * @remark If @p event_sink is a @ref fep3::arya::IClock::ClockType::discrete also the events
     *         @ref IEventSink::timeUpdateBegin, @ref IEventSink::timeUpdating,
     *         @ref IEventSink::timeUpdateEnd will be emitted to @p event_sink.
     *
     * @param[in] event_sink The event sink to emit time reset and time updating events to.
     */
    virtual void start(const std::weak_ptr<experimental::IClock::IEventSink>& event_sink) = 0;
    /**
     * @brief Stop the clock.
     * Usually the clock will reset immediately (still depending on the implementation).
     */
    virtual void stop() = 0;
    /**
     * @brief Get the Name of the clock.
     *
     * @return Name of the clock
     */
    virtual std::string getName() const = 0;
    /**
     * @brief Get the Type of the clock.
     *
     * @return The type of the clock
     * @retval fep3::arya::IClock::ClockType::continuous The clock implementation is a continuous
     * clock
     * @retval fep3::arya::IClock::ClockType::discrete   The clock implementation is a discrete
     * clock
     */
    virtual fep3::arya::IClock::ClockType getType() const = 0;
    /**
     * @brief Get the current time of the clock.
     *
     * @return Current time of the clock
     */
    virtual arya::Timestamp getTime() const = 0;
    /**
     * @brief Reset the clock.
     * Depending on the implementation the offset will be reset.
     * @remark The @ref fep3::experimental::IClock::IEventSink::timeResetBegin and the @ref
     * fep3::experimental::IClock::IEventSink::timeResetEnd will be emitted
     *
     * @param[in] new_time The new time of the clock
     */
    virtual void reset(fep3::arya::Timestamp new_time) = 0;
};

} // namespace experimental

using arya::IClock;

} // namespace fep3
