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


#include <thread>
#include <chrono>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/components/clock/clock_base.h>
#include <fep3/native_components/clock/local_system_clock.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/native_components/clock/local_clock_service.h>

using namespace ::testing;
using namespace fep3::arya;
using namespace fep3::native;
using namespace std::literals;

using EventSinkTimeEventValues = NiceMock<fep3::mock::EventSinkTimeEventValues>;

struct ContinuousClockTest : public Test
{
    ContinuousClockTest()
        : _event_sink_mock(std::make_shared<EventSinkTimeEventValues>(10))
    {

    }

    std::weak_ptr<EventSinkTimeEventValues> _event_sink_mock;
};

/**
 * @detail Test whether the clock provides a steadily rising time
 * @req_id FEPSDK-2108
 */
TEST_F(ContinuousClockTest, ClockProvidesSteadyTime)
{
    LocalSystemRealClock local_system_real_clock;
    Timestamp last_time(0), current_time(0);

    ASSERT_EQ(local_system_real_clock.getNewTime(), Timestamp(0));

    local_system_real_clock.resetTime(last_time);

    local_system_real_clock.start(_event_sink_mock);

    auto testidx = 0;
    //check if the clock is steady
    while (testidx < 10)
    {
        std::this_thread::sleep_for(1ms);
        current_time = local_system_real_clock.getNewTime();
        ASSERT_GT(current_time, last_time);
        last_time = current_time;
        testidx++;
    }

    local_system_real_clock.stop();
}

/**
 * @detail Test whether the clock can reset it's time
 *
 */
TEST_F(ContinuousClockTest, ClockReset)
{
    LocalSystemRealClock local_system_real_clock;
    Timestamp reset_time(0), reference_time(0);
    const Timestamp sync_time(250ms);

    local_system_real_clock.start(_event_sink_mock);
    local_system_real_clock.resetTime(sync_time);

    auto testidx = 0;
    //check if the clock correctly resets it's time
    while (testidx < 10)
    {
        std::this_thread::sleep_for(1ms);
        reference_time = local_system_real_clock.getNewTime();
        reset_time = local_system_real_clock.resetTime(sync_time);
        testidx++;
        ASSERT_GT(reference_time, reset_time);
    }

    local_system_real_clock.stop();
}

class TestContinuousClock : public fep3::base::ContinuousClock
{
public:
    TestContinuousClock()
        : ContinuousClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME)
    {
    }
    Timestamp getNewTime() const override
    {
        return std::chrono::nanoseconds(100)* (_count++ + 1) + _m_timestamp_offs;
    }
    Timestamp resetTime(Timestamp) override
    {
        return _m_timestamp_offs;
    }
    Timestamp _m_timestamp_offs = std::chrono::nanoseconds(100000000);
    mutable uint16_t _count = 0;
};

class TestEventSink : public fep3::arya::IClock::IEventSink
{
    void timeUpdateBegin(fep3::arya::Timestamp, fep3::arya::Timestamp) override
    {
    }
    void timeUpdating(fep3::arya::Timestamp) override
    {
    }

    void timeUpdateEnd(fep3::arya::Timestamp) override
    {
    }

    void timeResetBegin(fep3::arya::Timestamp, fep3::arya::Timestamp) override
    {
        _clock->getTime();
    }

    void timeResetEnd(fep3::arya::Timestamp) override
    {
    }
public:
    fep3::arya::IClock* _clock;
};

/**
 * @detail Test whether a call of getTime causes a recursion crash.
 * @req_id FEPSDK-3212
 */
TEST_F(ContinuousClockTest, RecursiveSetResetTimeCall)
{
    std::shared_ptr<TestEventSink> test_event_sink = std::make_shared<TestEventSink>();
    TestContinuousClock test_clock;
    fep3::arya::IClock* clock_interface = &test_clock;
    test_event_sink->_clock = clock_interface;

    // first tick
    clock_interface->start(test_event_sink);
    // second tick
    ASSERT_EQ(clock_interface->getTime(), std::chrono::nanoseconds(100000200))
        << "Clock time is not the expected ";
    // thirds tick
    ASSERT_EQ(clock_interface->getTime(), std::chrono::nanoseconds(100000300))
        << "Clock time is not the expected ";
    /*now the clocked is forced to go back causing a reset*/
    test_clock._m_timestamp_offs = std::chrono::nanoseconds(100);
    test_clock._count = 0;
    /*two ticks, one from this call and one from  TestEventSink::timeResetBegin*/
    ASSERT_EQ(clock_interface->getTime(), std::chrono::nanoseconds(200))
        << "Clock time is not the expected ";
}

class TestNonMonotonicClock : public fep3::base::ContinuousClock
{
public:
    TestNonMonotonicClock()
        : ContinuousClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME)
    {
    }
    Timestamp getNewTime() const override
    {
        return  _m_timestamp_offs - std::chrono::nanoseconds(100) * (_count++ + 1);
    }
    Timestamp resetTime(Timestamp) override
    {
        return _m_timestamp_offs;
    }
    Timestamp _m_timestamp_offs = std::chrono::nanoseconds(100000000);
    mutable int16_t _count = 0;
};

TEST_F(ContinuousClockTest, NoResetCallsWithNonMonotonicClock)
{
    auto mock_event_sink = std::make_shared<::testing::StrictMock<::fep3::mock::EventSink>>();
    TestNonMonotonicClock test_clock;
    fep3::arya::IClock* clock_interface = &test_clock;
    
    EXPECT_CALL(*mock_event_sink, timeResetBegin(_, { std::chrono::nanoseconds(100000000) }));
    EXPECT_CALL(*mock_event_sink, timeResetEnd({ std::chrono::nanoseconds(100000000) }));
    test_clock.start(mock_event_sink);

    // even if clock is non monotonic there should be no resets
    ASSERT_EQ(clock_interface->getTime(), std::chrono::nanoseconds(99999900))
        << "Clock time is not the expected ";
    ASSERT_EQ(clock_interface->getTime(), std::chrono::nanoseconds(99999800))
        << "Clock time is not the expected ";
}