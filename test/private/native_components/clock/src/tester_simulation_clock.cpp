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

#include <fep3/components/clock/mock/mock_clock_service_addons.h>
#include <fep3/native_components/clock/clock_event_sink_registry.h>
#include <fep3/native_components/clock/simulation_clock.h>

#include <boost/thread/barrier.hpp>

#include <optional>

using namespace ::testing;
using namespace fep3::native;
using namespace fep3::arya;
using namespace std::chrono;
using namespace fep3;

using EventSinkTimeEventValues = NiceMock<fep3::mock::EventSinkTimeEventValues>;
using EventSinkTimeEventFrequency = NiceMock<fep3::mock::EventSinkTimeEventFrequency>;

class ExternalClockSimulationMock : public IExternalClock {
public:
    MOCK_METHOD(TimePoint, now, (), ());
    MOCK_METHOD(void, waitUntil, (std::function<void()>), ());
    MOCK_METHOD(void, notify, (std::function<void()>), ());
};

using ExternalClockMock = NiceMock<ExternalClockSimulationMock>;
/**
 * @detail Fixture
 */
struct SimulationClockTest : public Test {
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    SimulationClock simulation_clock;
};

struct SimulationClockMockTest : public Test {
    void SetUp() override
    {
        external_clock_mock = std::make_unique<ExternalClockMock>();

        ON_CALL(*external_clock_mock, waitUntil(_)).WillByDefault(InvokeWithoutArgs([&]() {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]() { return stop_waiting == true; });
            stop_waiting = false;
        }));

        ON_CALL(*external_clock_mock, notify(_)).WillByDefault(InvokeWithoutArgs([&]() {
            stepForward();
        }));

        // Given a dummy time, so the waitUntil always is triggered.
        // Use EXPECT_CALL to modify it for more specified unit tests.
        ON_CALL(*external_clock_mock, now())
            .WillByDefault(Return(SimulationClock::TimePoint(Timestamp(0))));
    }

    void TearDown() override
    {
    }

    void stepForward()
    {
        {
            std::lock_guard lk(mtx);
            stop_waiting = true;
        }
        cv.notify_one();
    }

    std::unique_ptr<ExternalClockMock> external_clock_mock;
    std::unique_ptr<SimulationClock> simulation_clock_mock;

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic_bool stop_waiting{false};
};

TEST(SimulationClock, CTOR__withDefaultClock)
{
    auto clock = SimulationClock();
    ASSERT_EQ(clock.getName(), FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME);
    ASSERT_EQ(clock.getStepSize(), Duration{FEP3_CLOCK_SIM_TIME_STEP_SIZE_DEFAULT_VALUE});
    ASSERT_EQ(clock.getTimeFactor(), FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE);
}

TEST_F(SimulationClockTest, updateConfiguration__successful)
{
    const size_t step_size = 500;
    const double time_factor = 2.0;
    simulation_clock.updateConfiguration(Duration{step_size}, time_factor);
    ASSERT_EQ(simulation_clock.getStepSize(), Duration{step_size});
    ASSERT_EQ(simulation_clock.getTimeFactor(), time_factor);
}

TEST_F(SimulationClockTest, getStepSize__successful)
{
    ASSERT_EQ(simulation_clock.getStepSize(),
              Duration{FEP3_CLOCK_SIM_TIME_STEP_SIZE_DEFAULT_VALUE});
}

TEST_F(SimulationClockTest, getTimeFactor__successful)
{
    ASSERT_EQ(simulation_clock.getTimeFactor(), FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE);
}

TEST_F(SimulationClockTest, getName__successful)
{
    ASSERT_EQ(simulation_clock.getName(), FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME);
}

TEST_F(SimulationClockTest, getType__successful)
{
    ASSERT_EQ(simulation_clock.getType(), fep3::arya::IClock::ClockType::discrete);
}

TEST_F(SimulationClockTest, getTime__successful)
{
    ASSERT_EQ(simulation_clock.getTime(), Timestamp{0});
}

TEST_F(SimulationClockTest, reset__eventSinkTriggersResetEvent)
{
    auto event_sink_mock = std::make_shared<EventSinkTimeEventValues>(1, 1);

    EXPECT_CALL(*event_sink_mock, timeResetBegin(Timestamp{0}, Timestamp{0}));
    EXPECT_CALL(*event_sink_mock, timeResetEnd(Timestamp{0}));
    simulation_clock.start(event_sink_mock);
    event_sink_mock->waitForReset(1s);
    event_sink_mock->waitForUpdate(1s);

    EXPECT_CALL(*event_sink_mock, timeResetBegin(Timestamp{0}, Timestamp{1}));
    EXPECT_CALL(*event_sink_mock, timeResetEnd(Timestamp{1}));
    simulation_clock.reset(Timestamp{1});

    simulation_clock.stop();
}

TEST_F(SimulationClockTest, start__providesDiscreteTimeSteps)
{
    const size_t step_size = 100, clock_cycles = 5;
    const double time_factor = 1.0;
    // we expect 1 reset and 5 time updates
    auto event_sink_mock = std::make_shared<EventSinkTimeEventValues>(1, clock_cycles);

    // First timestamps originating from a reset event once the clock is started
    // and the first update
    std::vector<Timestamp> expected_timestamps{Timestamp{0}};

    for (size_t i = 0; i < clock_cycles; i++) {
        // Timestamps originating from time update events
        expected_timestamps.emplace_back(Timestamp(i * step_size));
    }

    simulation_clock.updateConfiguration(Duration{step_size}, time_factor);

    // actual test
    {
        simulation_clock.start(event_sink_mock);
        event_sink_mock->waitForReset(1s);
        event_sink_mock->waitForUpdate(1s);
        simulation_clock.stop();

        EXPECT_EQ(expected_timestamps, event_sink_mock->_calls);
    }
}

/**
 * @detail Test whether the clock can reset its time
 */
TEST_F(SimulationClockTest, start__testClockReset)
{
    const size_t step_size = 100;
    const double time_factor = 1.0;
    // we expect 1 reset and 2 update
    auto event_sink_mock = std::make_shared<EventSinkTimeEventValues>(1, 2);

    std::vector<Timestamp> expected_timestamps{Timestamp{0},
                                               Timestamp{0},
                                               Timestamp{100}, // first run
                                               Timestamp{0},
                                               Timestamp{0},
                                               Timestamp{100}}; // second run after reset

    // Update the clock configuraiton to reduce test duration
    simulation_clock.updateConfiguration(Duration{step_size}, time_factor);

    // actual test
    {
        simulation_clock.start(event_sink_mock);
        ASSERT_TRUE(event_sink_mock->waitForUpdate(1s));
        simulation_clock.stop();

        // we wait for another reset and 2 time update events
        event_sink_mock->_expected_reset_calls_count = 2;
        event_sink_mock->_expected_update_calls_count = 4;

        // start the clock again to trigger a time reset event
        simulation_clock.start(event_sink_mock);
        ASSERT_TRUE(event_sink_mock->waitForUpdate(100s));
        simulation_clock.stop();

        EXPECT_EQ(expected_timestamps, event_sink_mock->_calls);
    }
}

/**
 * @detail Test whether the frequency of a discrete clock's time update events
 * meets the configured expectations
 *
 * @testType performanceTest
 * This test may fail if the system is under heavy load as the test depends on
 * the system performance.
 */
TEST_F(SimulationClockTest, start__testClockTimeEventFrequency)
{
    const Duration step_size{50ms};
    const Duration expected_event_duration{50ms};
    const Duration allowed_deviation{
        200ms}; // wait_for wakeup can be delayed because of system performance

    const size_t clock_cycles = 3;
    const double time_factor = 1;

    auto event_sink_mock = std::make_shared<EventSinkTimeEventFrequency>(clock_cycles);
    simulation_clock.updateConfiguration(step_size, time_factor);

    // actual test
    {
        simulation_clock.start(event_sink_mock);
        event_sink_mock->waitFor(1s);
        simulation_clock.stop();

        EXPECT_EQ(clock_cycles, event_sink_mock->_call_durations.size());
        event_sink_mock->assertTimeEventDeviation(expected_event_duration, allowed_deviation);
    }
}

/**
 * @detail Test whether a small time factor decreases the frequency of a
 * discrete clock's time update events
 * @req_id FEPSDK-2111
 *
 * @testType performanceTest
 * This test may fail if the system is under heavy load as the test depends on
 * the system performance.
 */
TEST_F(SimulationClockTest, start__testClockTimeEventFrequencySmallTimeFactor)
{
    const Duration step_size{50ms}, expected_event_duration{100ms}, allowed_deviation{200ms};

    const size_t clock_cycles = 3;
    const double time_factor = 0.5;
    auto event_sink_mock = std::make_shared<EventSinkTimeEventFrequency>(clock_cycles);

    simulation_clock.updateConfiguration(step_size, time_factor);

    // actual test
    {
        simulation_clock.start(event_sink_mock);
        event_sink_mock->waitFor(1s);
        simulation_clock.stop();

        EXPECT_EQ(clock_cycles, event_sink_mock->_call_durations.size());
        event_sink_mock->assertTimeEventDeviation(expected_event_duration, allowed_deviation);
    }
}

/**
 * @detail Test whether a big time factor increases the frequency of a discrete
 * clock's time update events
 * @req_id FEPSDK-2111
 *
 * @testType performanceTest
 * This test may fail if the system is under heavy load as the test depends on
 * the system performance.
 */
TEST_F(SimulationClockTest, start__testClockTimeEventFrequencyBigTimeFactor)
{
    const Duration step_size{50ms}, expected_event_duration{25ms}, allowed_deviation{200ms};
    const size_t clock_cycles = 3;
    const double time_factor = 2;
    auto event_sink_mock = std::make_shared<EventSinkTimeEventFrequency>(clock_cycles);

    simulation_clock.updateConfiguration(step_size, time_factor);

    // actual test
    {
        simulation_clock.start(event_sink_mock);
        event_sink_mock->waitFor(1s);
        simulation_clock.stop();

        EXPECT_EQ(clock_cycles, event_sink_mock->_call_durations.size());
        event_sink_mock->assertTimeEventDeviation(expected_event_duration, allowed_deviation);
    }
}

/**
 * @detail Test whether a time factor of 0 (which means AFAP mode) increases the
 * frequency of a discrete clock's time update events
 * @req_id FEPSDK-2122
 *
 * @testType performanceTest
 * This test may fail if the system is under heavy load as the test depends on
 * the system performance.
 */
TEST_F(SimulationClockTest, start__testClockTimeEventAFAP)
{
    // Step size value does not matter in case of this test due to time factor
    // being 0. This test succeeds if event durations are below 2 ms as this
    // durations can not be reached when using a time factor != 0.
    const Duration step_size{20ms}, expected_event_duration{20ms}, allowed_deviation{20ms};
    const size_t clock_cycles = 3;
    const double time_factor = 0;

    auto event_sink_mock = std::make_shared<EventSinkTimeEventFrequency>(clock_cycles);
    simulation_clock.updateConfiguration(step_size, time_factor);

    // actual test
    {
        simulation_clock.start(event_sink_mock);
        event_sink_mock->waitFor(1s);
        simulation_clock.stop();

        EXPECT_EQ(clock_cycles, event_sink_mock->_call_durations.size());
        event_sink_mock->assertTimeEventDeviation(expected_event_duration, allowed_deviation);
    }
}

TEST_F(SimulationClockMockTest, stop__successful)
{
    // expect 1 reset and 1 update
    auto event_sink_mock = std::make_shared<EventSinkTimeEventValues>(1, 1);
    simulation_clock_mock =
        std::make_unique<SimulationClock>("name", std::move(external_clock_mock));

    EXPECT_CALL(*event_sink_mock, timeUpdateBegin(_, _)).Times(1);
    EXPECT_CALL(*event_sink_mock, timeUpdating(_, _)).Times(1);
    EXPECT_CALL(*event_sink_mock, timeUpdateEnd(_)).Times(1);

    simulation_clock_mock->start(event_sink_mock);
    event_sink_mock->waitForUpdate(1s);
    simulation_clock_mock->stop();
}

TEST_F(SimulationClockMockTest, start__testMultipleSteps)
{
    using ::testing::Sequence;

    auto event_sink_mock = std::make_shared<EventSinkTimeEventValues>(1, 1);
    simulation_clock_mock =
        std::make_unique<SimulationClock>("name", std::move(external_clock_mock));

    {
        InSequence s;

        // Expectation
        EXPECT_CALL(*event_sink_mock, timeUpdateBegin(_, _)).Times(1);
        EXPECT_CALL(*event_sink_mock, timeUpdating(_, _)).Times(1);
        EXPECT_CALL(*event_sink_mock, timeUpdateEnd(_)).Times(1);

        EXPECT_CALL(*event_sink_mock, timeUpdateBegin(_, _)).Times(1);
        EXPECT_CALL(*event_sink_mock, timeUpdating(_, _)).Times(1);
        EXPECT_CALL(*event_sink_mock, timeUpdateEnd(_)).Times(1);

        EXPECT_CALL(*event_sink_mock, timeUpdateBegin(_, _)).Times(1);
        EXPECT_CALL(*event_sink_mock, timeUpdating(_, _)).Times(1);
        EXPECT_CALL(*event_sink_mock, timeUpdateEnd(_)).Times(1);

        // Act
        simulation_clock_mock->start(event_sink_mock);
        event_sink_mock->waitForUpdate(1s);

        event_sink_mock->_expected_update_calls_count++;
        stepForward();
        event_sink_mock->waitForUpdate(1s);

        event_sink_mock->_expected_update_calls_count++;
        stepForward();
        event_sink_mock->waitForUpdate(1s);

        simulation_clock_mock->stop();
    }
}

TEST_F(SimulationClockTest, start__testClockEventSinkRegistryParallelEvents)
{
    auto clock_event_sink_registry = std::make_shared<fep3::native::ClockEventSinkRegistry>();
    int update_cycles_expected = 10;
    int event_sink_total = 5;

    // check the event sink will be handled on an dedicated thread
    std::vector<std::optional<std::thread::id>> thread_ids;
    std::vector<std::shared_ptr<EventSinkTimeEventValues>> event_sinks;
    std::set<std::thread::id> thread_id_counter;
    boost::barrier bar(event_sink_total);
    std::mutex mtx;

    auto check_thread_id = [&](std::optional<std::thread::id>& id) {
        if (!id) {
            id = std::this_thread::get_id();
            {
                std::lock_guard<std::mutex> lk(mtx);
                thread_id_counter.insert(*id);
            }
        }
        else {
            ASSERT_EQ(*id, std::this_thread::get_id());
        }
    };
    // block all event sink with barrier
    for (int i = 0; i < event_sink_total; ++i) {
        thread_ids.emplace_back(std::optional<std::thread::id>());
        event_sinks.emplace_back(
            std::make_shared<EventSinkTimeEventValues>(1, update_cycles_expected, [&, i]() {
                bar.wait();
                check_thread_id(thread_ids[i]);
            }));
        EXPECT_CALL(*(event_sinks.back()), timeUpdating(_, _)).Times(update_cycles_expected);
        clock_event_sink_registry->registerSink<fep3::experimental::IClock::IEventSink>(
            event_sinks.back());
    }

    simulation_clock.start(clock_event_sink_registry);

    for (int i = 0; i < event_sink_total; ++i) {
        event_sinks[i]->waitForUpdate(2s);
    }

    ASSERT_EQ(thread_id_counter.size(), event_sink_total);

    simulation_clock.stop();
}
