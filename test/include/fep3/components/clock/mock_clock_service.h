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

#include <fep3/components/base/component.h>
#include <fep3/components/clock/clock_service_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

namespace detail {

/**
 * @brief Mock class for @ref fep3::arya::IClockService
 *
 * @tparam component_base_type The component base class
 */
template <template <typename...> class component_base_type>
struct ClockService : public component_base_type<fep3::arya::IClockService> {
    MOCK_METHOD(fep3::arya::Timestamp, getTime, (), (const, override));
    MOCK_METHOD(Optional<fep3::arya::Timestamp>, getTime, (const std::string&), (const, override));

    MOCK_METHOD(fep3::arya::IClock::ClockType, getType, (), (const, override));
    MOCK_METHOD(Optional<fep3::arya::IClock::ClockType>,
                getType,
                (const std::string&),
                (const, override));

    MOCK_METHOD(std::string, getMainClockName, (), (const, override));

    MOCK_METHOD(fep3::Result,
                registerEventSink,
                (const std::weak_ptr<fep3::arya::IClock::IEventSink>&),
                (override));
    MOCK_METHOD(fep3::Result,
                unregisterEventSink,
                (const std::weak_ptr<fep3::arya::IClock::IEventSink>&),
                (override));

    MOCK_METHOD(fep3::Result,
                registerClock,
                (const std::shared_ptr<fep3::arya::IClock>&),
                (override));
    MOCK_METHOD(fep3::Result, unregisterClock, (const std::string&), (override));

    MOCK_METHOD(std::list<std::string>, getClockNames, (), (const, override));
    MOCK_METHOD(std::shared_ptr<fep3::arya::IClock>,
                findClock,
                (const std::string&),
                (const, override));

    MOCK_METHOD(fep3::Result, start, (), (override));
    MOCK_METHOD(fep3::Result, stop, (), (override));
};

} // namespace detail

/**
 * @brief Mock class for @ref fep3::arya::IClockService
 */
using ClockService = detail::ClockService<fep3::base::arya::Component>;

} // namespace arya

namespace experimental {

namespace detail {

/**
 * @brief Mock class for @ref fep3::arya::IClockService
 *
 * @tparam component_base_type The component base class
 */
template <template <typename...> class component_base_type>
struct ClockService
    : public component_base_type<fep3::experimental::IClockService, fep3::arya::IClockService> {
    MOCK_METHOD(fep3::arya::Timestamp, getTime, (), (const, override));
    MOCK_METHOD(Optional<fep3::arya::Timestamp>, getTime, (const std::string&), (const, override));

    MOCK_METHOD(fep3::arya::IClock::ClockType, getType, (), (const, override));
    MOCK_METHOD(Optional<fep3::arya::IClock::ClockType>,
                getType,
                (const std::string&),
                (const, override));

    MOCK_METHOD(std::string, getMainClockName, (), (const, override));

    MOCK_METHOD(fep3::Result,
                registerEventSink,
                (const std::weak_ptr<fep3::arya::IClock::IEventSink>&),
                (override));
    MOCK_METHOD(fep3::Result,
                unregisterEventSink,
                (const std::weak_ptr<fep3::arya::IClock::IEventSink>&),
                (override));

    MOCK_METHOD(fep3::Result,
                registerEventSink,
                (const std::weak_ptr<fep3::experimental::IClock::IEventSink>&),
                (override));
    MOCK_METHOD(fep3::Result,
                unregisterEventSink,
                (const std::weak_ptr<fep3::experimental::IClock::IEventSink>&),
                (override));

    MOCK_METHOD(fep3::Result,
                registerClock,
                (const std::shared_ptr<fep3::arya::IClock>&),
                (override));
    MOCK_METHOD(fep3::Result,
                registerClock,
                (const std::shared_ptr<fep3::experimental::IClock>&),
                (override));
    MOCK_METHOD(fep3::Result, unregisterClock, (const std::string&), (override));

    MOCK_METHOD(std::list<std::string>, getClockNames, (), (const, override));
    MOCK_METHOD(std::shared_ptr<fep3::arya::IClock>,
                findClock,
                (const std::string&),
                (const, override));

    MOCK_METHOD(std::shared_ptr<fep3::experimental::IClock>,
                findClockCatelyn,
                (const std::string&),
                (const, override));

    MOCK_METHOD(fep3::Result, start, (), (override));
    MOCK_METHOD(fep3::Result, stop, (), (override));
};

} // namespace detail

/**
 * @brief Mock class for @ref fep3::arya::IClockService
 */
using ClockService = detail::ClockService<fep3::base::arya::Component>;

} // namespace experimental

using experimental::ClockService;
} // namespace mock
} // namespace fep3

///@endcond nodoc