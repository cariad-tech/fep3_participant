/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include <fep3/components/clock/mock/mock_clock_service_addons.h>
#include <fep3/native_components/clock/clock_event_sink_registry.h>

#include <gtest/gtest.h>

#include <common/gtest_asserts.h>

using ExperimentalEventSinkMock = NiceMock<fep3::mock::experimental::Clock::EventSink>;
using AryaEventSinkMock = NiceMock<fep3::mock::arya::Clock::EventSink>;

namespace fep3 {

namespace test {

template <typename T>
struct ClockEventSinkRegistryTest : public ::testing::Test {
    using MockEventSinkType = T;
    using ClockEventSinkInterfaceType = typename T::ClockEventSinkInterfaceType;

    ClockEventSinkRegistryTest()
        : _clock_event_sink_registry(std::make_shared<native::ClockEventSinkRegistry>())
    {
    }
    std::shared_ptr<native::ClockEventSinkRegistry> _clock_event_sink_registry;
};

using ClockEventSinkTypes = ::testing::Types<ExperimentalEventSinkMock, AryaEventSinkMock>;
TYPED_TEST_SUITE(ClockEventSinkRegistryTest, ClockEventSinkTypes);

TYPED_TEST(ClockEventSinkRegistryTest, registerSink__successful)
{
    using ThisType = std::remove_pointer_t<std::remove_cv_t<decltype(this)>>;
    using ClockEventSinkInterfaceType = typename ThisType::ClockEventSinkInterfaceType;
    using MockEventSinkType = typename ThisType::MockEventSinkType;

    auto sink = std::make_shared<MockEventSinkType>();
    ASSERT_FEP3_NOERROR(
        this->_clock_event_sink_registry->template registerSink<ClockEventSinkInterfaceType>(sink));
}

TYPED_TEST(ClockEventSinkRegistryTest, registerSink__failed)
{
    using ThisType = std::remove_pointer_t<std::remove_cv_t<decltype(this)>>;
    using ClockEventSinkInterfaceType = typename ThisType::ClockEventSinkInterfaceType;
    using MockEventSinkType = typename ThisType::MockEventSinkType;

    auto sink1 = std::make_shared<MockEventSinkType>();
    auto sink2 = std::make_shared<MockEventSinkType>();
    std::weak_ptr<MockEventSinkType> sink3 = sink1;

    ASSERT_FEP3_NOERROR(
        this->_clock_event_sink_registry->template registerSink<ClockEventSinkInterfaceType>(
            std::weak_ptr<ClockEventSinkInterfaceType>(sink1)));
    ASSERT_FEP3_NOERROR(
        this->_clock_event_sink_registry->template registerSink<ClockEventSinkInterfaceType>(
            std::weak_ptr<ClockEventSinkInterfaceType>(sink2)));
    ASSERT_FEP3_RESULT(
        this->_clock_event_sink_registry->template registerSink<ClockEventSinkInterfaceType>(
            std::weak_ptr<ClockEventSinkInterfaceType>(sink3)),
        fep3::ERR_FAILED);
}

TYPED_TEST(ClockEventSinkRegistryTest, registerSink__invalidEventSink)
{
    using ThisType = std::remove_pointer_t<std::remove_cv_t<decltype(this)>>;
    using ClockEventSinkInterfaceType = typename ThisType::ClockEventSinkInterfaceType;
    using MockEventSinkType = typename ThisType::MockEventSinkType;

    auto sink1 = std::make_shared<MockEventSinkType>();
    std::weak_ptr<MockEventSinkType> sink2 = sink1;
    sink1.reset();

    ASSERT_FEP3_RESULT(
        this->_clock_event_sink_registry->template registerSink<ClockEventSinkInterfaceType>(sink2),
        fep3::ERR_INVALID_ARG);
}

TYPED_TEST(ClockEventSinkRegistryTest, unregisterSink__successful)
{
    using ThisType = std::remove_pointer_t<std::remove_cv_t<decltype(this)>>;
    using ClockEventSinkInterfaceType = typename ThisType::ClockEventSinkInterfaceType;
    using MockEventSinkType = typename ThisType::MockEventSinkType;

    auto sink = std::make_shared<MockEventSinkType>();
    ASSERT_FEP3_NOERROR(
        this->_clock_event_sink_registry->template registerSink<ClockEventSinkInterfaceType>(sink));
    ASSERT_FEP3_NOERROR(
        this->_clock_event_sink_registry->template unregisterSink<ClockEventSinkInterfaceType>(
            sink));
}

TYPED_TEST(ClockEventSinkRegistryTest, unregisterSink__failed)
{
    using ThisType = std::remove_pointer_t<std::remove_cv_t<decltype(this)>>;
    using ClockEventSinkInterfaceType = typename ThisType::ClockEventSinkInterfaceType;
    using MockEventSinkType = typename ThisType::MockEventSinkType;

    auto sink = std::make_shared<MockEventSinkType>();
    ASSERT_FEP3_RESULT(
        this->_clock_event_sink_registry->template unregisterSink<ClockEventSinkInterfaceType>(
            sink),
        fep3::ERR_FAILED);
}

TYPED_TEST(ClockEventSinkRegistryTest, unregisterSink__invalidEventSink)
{
    using ThisType = std::remove_pointer_t<std::remove_cv_t<decltype(this)>>;
    using MockEventSinkType = typename ThisType::MockEventSinkType;

    auto sink1 = std::make_shared<MockEventSinkType>();
    std::weak_ptr<MockEventSinkType> sink2 = sink1;
    sink1.reset();
    ASSERT_FEP3_RESULT(this->_clock_event_sink_registry->unregisterSink(sink2),
                       fep3::ERR_INVALID_ARG);
}

TYPED_TEST(ClockEventSinkRegistryTest, registerAryaSink__successful)
{
    auto sink = std::make_shared<AryaEventSinkMock>();
    ASSERT_FEP3_NOERROR(
        this->_clock_event_sink_registry->template registerSink<fep3::arya::IClock::IEventSink>(
            sink));
}

TYPED_TEST(ClockEventSinkRegistryTest, unregisterAryaSink__successful)
{
    auto sink = std::make_shared<AryaEventSinkMock>();
    ASSERT_FEP3_NOERROR(
        this->_clock_event_sink_registry->template registerSink<fep3::arya::IClock::IEventSink>(
            sink));
    ASSERT_FEP3_NOERROR(
        this->_clock_event_sink_registry->template unregisterSink<fep3::arya::IClock::IEventSink>(
            sink));
}

template <typename EventSinkType>
class SinkTriggeringTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        if constexpr (std::is_same_v<EventSinkType, ExperimentalEventSinkMock>)
            this->_clock_event_sink_registry->registerSink<EventSinkType>(_sink);
        else
            this->_clock_event_sink_registry->registerSink<fep3::arya::IClock::IEventSink>(_sink);
    }
    void setUpTimeUpdateExpectation(Timestamp new_time, std::optional<Timestamp> next_tick)
    {
        if constexpr (std::is_same_v<EventSinkType, ExperimentalEventSinkMock>)
            EXPECT_CALL(*_sink, timeUpdating(new_time, next_tick)).Times(1);
        else {
            (void)next_tick;
            EXPECT_CALL(*_sink, timeUpdating(new_time)).Times(1);
        }
    }

    std::shared_ptr<EventSinkType> _sink = std::make_shared<EventSinkType>();
    std::shared_ptr<native::ClockEventSinkRegistry> _clock_event_sink_registry =
        std::make_shared<native::ClockEventSinkRegistry>();
};

using SinkTypes = ::testing::Types<AryaEventSinkMock, ExperimentalEventSinkMock>;
TYPED_TEST_SUITE(SinkTriggeringTest, SinkTypes);

TYPED_TEST(SinkTriggeringTest, eventsForwarded__successful)
{
    fep3::experimental::IClock::IEventSink* sink = this->_clock_event_sink_registry.get();
    using namespace std::chrono_literals;

    EXPECT_CALL(*(this->_sink), timeUpdateBegin(0ns, 1ns)).Times(1);
    this->setUpTimeUpdateExpectation(1ns, 2ns);
    EXPECT_CALL(*(this->_sink), timeUpdateEnd(2ns)).Times(1);
    EXPECT_CALL(*(this->_sink), timeResetBegin(3ns, 2ns)).Times(1);
    EXPECT_CALL(*(this->_sink), timeResetEnd(4ns)).Times(1);

    sink->timeUpdateBegin(0ns, 1ns);
    sink->timeUpdating(1ns, 2ns);
    sink->timeUpdateEnd(2ns);
    sink->timeResetBegin(3ns, 2ns);
    sink->timeResetEnd(4ns);
}

} // namespace test
} // namespace fep3
