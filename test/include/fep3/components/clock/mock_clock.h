/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/clock/clock_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

struct Clock : public fep3::arya::IClock {
    using ClockInterfaceType = fep3::arya::IClock;
    struct EventSink : fep3::arya::IClock::IEventSink {
        using ClockEventSinkInterfaceType = fep3::arya::IClock::IEventSink;
        MOCK_METHOD(void,
                    timeUpdateBegin,
                    (fep3::arya::Timestamp, fep3::arya::Timestamp),
                    (override));
        MOCK_METHOD(void, timeUpdating, (fep3::arya::Timestamp), (override));
        MOCK_METHOD(void, timeUpdateEnd, (fep3::arya::Timestamp), (override));
        MOCK_METHOD(void,
                    timeResetBegin,
                    (fep3::arya::Timestamp, fep3::arya::Timestamp),
                    (override));
        MOCK_METHOD(void, timeResetEnd, (fep3::arya::Timestamp), (override));
    };

    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(ClockType, getType, (), (const, override));
    MOCK_METHOD(fep3::arya::Timestamp, getTime, (), (const, override));
    MOCK_METHOD(void, reset, (fep3::arya::Timestamp), (override));
    MOCK_METHOD(void, start, (const std::weak_ptr<IEventSink>&), (override));
    MOCK_METHOD(void, stop, (), (override));
};

} // namespace arya

namespace experimental {
struct Clock : public fep3::experimental::IClock {
    using ClockInterfaceType = fep3::experimental::IClock;
    struct EventSink : fep3::experimental::IClock::IEventSink {
        using ClockEventSinkInterfaceType = fep3::experimental::IClock::IEventSink;
        MOCK_METHOD(void,
                    timeUpdateBegin,
                    (fep3::arya::Timestamp, fep3::arya::Timestamp),
                    (override));
        MOCK_METHOD(void,
                    timeUpdating,
                    (fep3::arya::Timestamp, std::optional<fep3::arya::Timestamp> next_tick),
                    (override));
        MOCK_METHOD(void, timeUpdateEnd, (fep3::arya::Timestamp), (override));
        MOCK_METHOD(void,
                    timeResetBegin,
                    (fep3::arya::Timestamp, fep3::arya::Timestamp),
                    (override));
        MOCK_METHOD(void, timeResetEnd, (fep3::arya::Timestamp), (override));
    };

    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(fep3::arya::IClock::ClockType, getType, (), (const, override));
    MOCK_METHOD(fep3::arya::Timestamp, getTime, (), (const, override));
    MOCK_METHOD(void, reset, (fep3::arya::Timestamp), (override));
    MOCK_METHOD(void, start, (const std::weak_ptr<IEventSink>&), (override));
    MOCK_METHOD(void, stop, (), (override));
};
} // namespace experimental

using experimental::Clock;
} // namespace mock

namespace stub::arya {
struct Clock : public fep3::arya::IClock {
    using ClockInterfaceType = fep3::arya::IClock;

    Clock()
    {
        using namespace ::testing;
        ON_CALL(*this, getName).WillByDefault(Return("StubClock"));
        ON_CALL(*this, getType).WillByDefault(Return(fep3::arya::IClock::ClockType::discrete));
        ON_CALL(*this, getTime).WillByDefault(Return(fep3::Timestamp{0}));
    }

    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(ClockType, getType, (), (const, override));
    MOCK_METHOD(fep3::arya::Timestamp, getTime, (), (const, override));
    MOCK_METHOD(void, reset, (fep3::arya::Timestamp), (override));
    MOCK_METHOD(void, start, (const std::weak_ptr<IEventSink>&), (override));
    MOCK_METHOD(void, stop, (), (override));
};
} // namespace stub::arya

namespace stub::experimental {
struct Clock : public fep3::experimental::IClock {
    using ClockInterfaceType = fep3::experimental::IClock;

    Clock()
    {
        using namespace ::testing;
        ON_CALL(*this, getName).WillByDefault(Return("StubClock"));
        ON_CALL(*this, getType).WillByDefault(Return(fep3::arya::IClock::ClockType::discrete));
        ON_CALL(*this, getTime).WillByDefault(Return(fep3::Timestamp{0}));
    }

    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(fep3::arya::IClock::ClockType, getType, (), (const, override));
    MOCK_METHOD(fep3::arya::Timestamp, getTime, (), (const, override));
    MOCK_METHOD(void, reset, (fep3::arya::Timestamp), (override));
    MOCK_METHOD(void,
                start,
                (const std::weak_ptr<fep3::experimental::IClock::IEventSink>&),
                (override));
    MOCK_METHOD(void, stop, (), (override));
};
} // namespace stub::experimental

} // namespace fep3

///@endcond nodoc