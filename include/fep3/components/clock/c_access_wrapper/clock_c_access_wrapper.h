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
// @note All methods are defined inline to provide the functionality as header only.

#pragma once

#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/clock/c_intf/clock_c_intf.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

/**
 * Access class for @ref fep3::arya::IClock.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IClock
 * that resides in another binary (e. g. a shared library).
 */
class Clock
    : public ::fep3::arya::IClock
    , private c::arya::DestructionManager
    , protected arya::Helper
{
public:
    /**
     * Access class for @ref fep3::arya::IClock::IEventSink.
     * Use this class to access a remote object of a type derived from @ref fep3::arya::IClock::IEventSink
     * that resides in another binary (e. g. a shared library).
     */
    class EventSink
        : public fep3::arya::IClock::IEventSink
        , private c::arya::DestructionManager
    {
    public:
        /// Type of access structure
        using Access = fep3_arya_IClock_SIEventSink;

        /**
         * @brief CTOR
         * @param[in] access Access to the remote object
         * @param[in] destructors List of destructors to be called upon destruction of this
         */
        inline EventSink
            (const Access& access
            , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
            );
        inline ~EventSink() override = default;

        // methods implementing fep3::arya::IClock::IEventSink
        /// @cond no_documentation
        inline void timeUpdateBegin(::fep3::arya::Timestamp old_time, ::fep3::arya::Timestamp new_time) override;
        inline void timeUpdating(::fep3::arya::Timestamp new_time) override;
        inline void timeUpdateEnd(::fep3::arya::Timestamp new_time) override;
        inline void timeResetBegin(::fep3::arya::Timestamp old_time, ::fep3::arya::Timestamp new_time) override;
        inline void timeResetEnd(::fep3::arya::Timestamp new_time) override;
        /// @endcond no_documentation

    private:
        Access _access;
    };

    /// Type of access structure
    using Access = fep3_arya_SIClock;

    /**
     * @brief CTOR
     * @param[in] access Access to the remote object
     * @param[in] destructors List of destructors to be called upon destruction of this
     */
    inline Clock
        (const Access& access
        , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
        );
    inline ~Clock() override = default;

    // methods implementing fep3::arya::IClock
    /// @cond no_documentation
    inline std::string getName() const override;
    inline fep3::arya::IClock::ClockType getType() const override;
    inline fep3::arya::Timestamp getTime() const override;
    inline void reset(::fep3::arya::Timestamp new_time) override;
    inline void start(const std::weak_ptr<fep3::arya::IClock::IEventSink>& event_sink) override;
    inline void stop() override;
    /// @endcond no_documentation

private:
    Access _access;
    std::deque
        <std::pair<std::weak_ptr<::fep3::arya::IClock::IEventSink>
        , ::fep3::plugin::c::access::arya::Destructor<fep3_plugin_c_arya_SDestructionManager>>
        > _remote_clock_object_destructors;
};

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface @ref fep3::arya::IClock
 */
class Clock : private arya::Helper<fep3::arya::IClock>
{
public:
    /**
     * Wrapper class for interface @ref fep3::arya::IClock::IEventSink
     */
    class EventSink : private arya::Helper<fep3::arya::IClock::IEventSink>
    {
    public:
        /**
         * Functor creating an access structure for @ref ::fep3::arya::IClock::IEventSink
         */
        struct AccessCreator
        {
            /**
             * Creates an access structure to the event sink as pointed to by \p pointer_to_event_sink
             *
             * @param[in] pointer_to_event_sink Pointer to the event sink to create an access structure for
             * @return Access structure to the event sink
             */
            fep3_arya_IClock_SIEventSink operator()(fep3::arya::IClock::IEventSink* pointer_to_event_sink) const noexcept
            {
                return fep3_arya_IClock_SIEventSink
                    {reinterpret_cast<fep3_arya_IClock_HIEventSink>(pointer_to_event_sink)
                    , timeUpdateBegin
                    , timeUpdating
                    , timeUpdateEnd
                    , timeResetBegin
                    , timeResetEnd
                    };
            }
        };

        /// Alias for the helper
        using Helper = arya::Helper<fep3::arya::IClock::IEventSink>;
        /// Alias for the type of the handle to a wrapped object of type @ref fep3::arya::IClock::IEventSink
        using Handle = fep3_arya_IClock_HIEventSink;

        // static methods transferring calls from the C interface to an object of fep3::arya::IClock::IEventSink
        /// @cond no_documentation
        static inline fep3_plugin_c_InterfaceError timeUpdateBegin
            (Handle handle
            , int64_t old_time
            , int64_t new_time
            ) noexcept
        {
            return Helper::call
                (handle
                , &fep3::arya::IClock::IEventSink::timeUpdateBegin
                , fep3::arya::Timestamp(old_time)
                , fep3::arya::Timestamp(new_time)
                );
        }
        static inline fep3_plugin_c_InterfaceError timeUpdating
            (Handle handle
            , int64_t new_time
            ) noexcept
        {
            return Helper::call
                (handle
                , &fep3::arya::IClock::IEventSink::timeUpdating
                , fep3::arya::Timestamp(new_time)
                );
        }
        static inline fep3_plugin_c_InterfaceError timeUpdateEnd
            (Handle handle
            , int64_t new_time
            ) noexcept
        {
            return Helper::call
                (handle
                , &fep3::arya::IClock::IEventSink::timeUpdateEnd
                , fep3::arya::Timestamp(new_time)
                );
        }
        static inline fep3_plugin_c_InterfaceError timeResetBegin
            (Handle handle
            , int64_t old_time
            , int64_t new_time
            ) noexcept
        {
            return Helper::call
                (handle
                , &fep3::arya::IClock::IEventSink::timeResetBegin
                , fep3::arya::Timestamp(old_time)
                , fep3::arya::Timestamp(new_time)
                );
        }
        static inline fep3_plugin_c_InterfaceError timeResetEnd
            (Handle handle
            , int64_t new_time
            ) noexcept
        {
            return Helper::call
                (handle
                , &fep3::arya::IClock::IEventSink::timeResetEnd
                , fep3::arya::Timestamp(new_time)
                );
        }
        /// @endcond no_documentation
    };

    /**
     * Functor creating an access structure for @ref ::fep3::arya::IClock
     */
    struct AccessCreator
    {
        /**
         * Creates an access structure to the clock as pointed to by @p pointer_to_clock
         *
         * @param[in] pointer_to_clock Pointer to the clock to create an access structure for
         * @return Access structure to the clock
         */
        fep3_arya_SIClock operator()(::fep3::arya::IClock* pointer_to_clock) const noexcept
        {
            return fep3_arya_SIClock
                {reinterpret_cast<::fep3::plugin::c::wrapper::arya::Clock::Handle>(pointer_to_clock)
                , ::fep3::plugin::c::wrapper::arya::Clock::getName
                , ::fep3::plugin::c::wrapper::arya::Clock::getType
                , ::fep3::plugin::c::wrapper::arya::Clock::getTime
                , ::fep3::plugin::c::wrapper::arya::Clock::reset
                , ::fep3::plugin::c::wrapper::arya::Clock::start
                , ::fep3::plugin::c::wrapper::arya::Clock::stop
                };
        }
    };

    /// Alias for the helper
    using Helper = arya::Helper<fep3::arya::IClock>;
    /// Alias for the type of the handle to a wrapped object of type @ref fep3::arya::IClock
    using Handle = fep3_arya_HIClock;

    // static methods transferring calls from the C interface to an object of fep3::arya::IClock
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError getName
        (Handle handle
        , void(*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::IClock::getName
            , callback
            , destination
            , [](const std::string& name)
                {
                    return name.c_str();
                }
            );
    }
    static inline fep3_plugin_c_InterfaceError getType
        (Handle handle
        , int32_t* result
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IClock::getType
            , [](const fep3::arya::IClock::ClockType& type)
                {
                    return static_cast<int32_t>(type);
                }
            , result
            );
    }
    static inline fep3_plugin_c_InterfaceError getTime
        (Handle handle
        , int64_t* result
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IClock::getTime
            , [](fep3::arya::Timestamp timestamp)
                {
                    return timestamp.count();
                }
            , result
            );
    }
    static inline fep3_plugin_c_InterfaceError reset
        (Handle handle
        , int64_t new_time
        ) noexcept
    {
        return Helper::call
            (handle
            , &fep3::arya::IClock::reset
            , fep3::arya::Timestamp(new_time)
            );
    }
    static inline fep3_plugin_c_InterfaceError start
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_IClock_SIEventSink event_sink_access
        ) noexcept
    {
        return Helper::transferWeakPtr<access::arya::Clock::EventSink>
            (handle
            , [](auto&& clock, auto&& event_sink)
                {
                    return clock->start(std::forward<decltype(event_sink)>(event_sink));
                }
            , destruction_manager_access_result
            , event_sink_access
            );
    }
    static inline fep3_plugin_c_InterfaceError stop
        (Handle handle
        ) noexcept
    {
        return Helper::call
            (handle
            , &fep3::arya::IClock::stop
            );
    }
    /// @endcond no_documentation
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

Clock::EventSink::EventSink
    (const Access& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
    : _access(access)
{
    addDestructors(std::move(destructors));
}

/// @cond no_documentation
void Clock::EventSink::timeUpdateBegin(::fep3::arya::Timestamp old_time, ::fep3::arya::Timestamp new_time)
{
    return arya::Helper::call
        (_access._handle
        , _access.timeUpdateBegin
        , old_time.count()
        , new_time.count()
        );
}

void Clock::EventSink::timeUpdating(::fep3::arya::Timestamp new_time)
{
    return arya::Helper::call
        (_access._handle
        , _access.timeUpdating
        , new_time.count()
        );
}

void Clock::EventSink::timeUpdateEnd(::fep3::arya::Timestamp new_time)
{
    return arya::Helper::call
        (_access._handle
        , _access.timeUpdateEnd
        , new_time.count()
        );
}

void Clock::EventSink::timeResetBegin(::fep3::arya::Timestamp old_time, ::fep3::arya::Timestamp new_time)
{
    return arya::Helper::call
        (_access._handle
        , _access.timeResetBegin
        , old_time.count()
        , new_time.count()
        );
}

void Clock::EventSink::timeResetEnd(Timestamp new_time)
{
    return arya::Helper::call
        (_access._handle
        , _access.timeResetEnd
        , new_time.count()
        );
}

Clock::Clock
    (const Access& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
    : _access(access)
{
    addDestructors(std::move(destructors));
}

std::string Clock::getName() const
{
    return arya::Helper::callWithResultCallback<std::string>
        (_access._handle
        , _access.getName
        , [](auto result)
            {
                return result;
            }
        );
}

fep3::arya::IClock::ClockType Clock::getType() const
{
    return static_cast<fep3::arya::IClock::ClockType>(arya::Helper::callWithResultParameter
        (_access._handle
        , _access.getType
        ));
}

fep3::arya::Timestamp Clock::getTime() const
{
    return fep3::arya::Timestamp(arya::Helper::callWithResultParameter
        (_access._handle
        , _access.getTime
        ));
}

void Clock::reset(fep3::arya::Timestamp new_time)
{
    return arya::Helper::call
        (_access._handle
        , _access.reset
        , new_time.count()
        );
}

void Clock::start(const std::weak_ptr<fep3::arya::IClock::IEventSink>& event_sink)
{
    return arya::Helper::transferWeakPtr
        (event_sink
        , _remote_clock_object_destructors
        , _access._handle
        , _access.start
        , [](const auto& pointer_to_event_sink)
            {
                return ::fep3::plugin::c::wrapper::arya::Clock::EventSink::AccessCreator()
                    (pointer_to_event_sink);
            }
        );
}

void Clock::stop()
{
    return arya::Helper::call
        (_access._handle
        , _access.stop
        );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
