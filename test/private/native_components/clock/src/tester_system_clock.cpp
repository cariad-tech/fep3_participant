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

#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/clock/mock/mock_clock_service_addons.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/native_components/clock/clock_service.h>
#include <fep3/native_components/clock/system_clock.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

using namespace ::testing;
using namespace fep3::arya;
using namespace fep3::native;
using namespace std::literals;

using EventSinkTimeEventValues = NiceMock<fep3::mock::EventSinkTimeEventValues>;

struct SystemClockTest : public Test {
    SystemClockTest() : _event_sink_mock(std::make_shared<EventSinkTimeEventValues>(10))
    {
    }

    std::shared_ptr<EventSinkTimeEventValues> _event_sink_mock;
    SystemClock sys_clock;
};

TEST_F(SystemClockTest, getName__isSystemTimeClock)
{
    ASSERT_EQ(sys_clock.getName(), FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);
}

TEST_F(SystemClockTest, getType__isContinousClockType)
{
    ASSERT_EQ(sys_clock.getType(), fep3::arya::IClock::ClockType::continuous);
}

/**
 * @detail Test whether the clock can reset it's time
 */
TEST_F(SystemClockTest, resetTime__successful)
{
    Timestamp reset_time(0), reference_time(0);
    const Timestamp sync_time(250ms);

    sys_clock.start(_event_sink_mock);
    sys_clock.resetTime(sync_time);

    auto testidx = 0;
    // check if the clock correctly resets it's time
    while (testidx < 10) {
        std::this_thread::sleep_for(1ms);
        reference_time = sys_clock.getNewTime();
        reset_time = sys_clock.resetTime(sync_time);
        testidx++;
        ASSERT_GT(reference_time, reset_time);
    }

    sys_clock.stop();
}

/**
 * @detail Test whether the clock provides a steadily rising time
 * @req_id FEPSDK-2108
 */
TEST_F(SystemClockTest, start__clockProvidesSteadyTime)
{
    Timestamp last_time(0), current_time(0);

    ASSERT_EQ(sys_clock.getNewTime(), Timestamp(0));

    sys_clock.resetTime(last_time);

    sys_clock.start(_event_sink_mock);

    auto testidx = 0;
    // check if the clock is steady
    while (testidx < 10) {
        std::this_thread::sleep_for(1ms);
        current_time = sys_clock.getNewTime();
        ASSERT_GT(current_time, last_time);
        last_time = current_time;
        testidx++;
    }

    sys_clock.stop();
}

/**
 * @detail Test whether a call of getTime causes a recursion crash.
 * @req_id FEPSDK-3212
 */
TEST_F(SystemClockTest, getTime__recursiveSetResetTimeCall)
{
    EXPECT_CALL(*_event_sink_mock, timeResetBegin(_, _))
        .WillRepeatedly(
            Invoke([&](fep3::arya::Timestamp, fep3::arya::Timestamp) { sys_clock.getTime(); }));
    sys_clock.start(_event_sink_mock);
    _event_sink_mock->waitForReset(1s);
    sys_clock.stop();
}
