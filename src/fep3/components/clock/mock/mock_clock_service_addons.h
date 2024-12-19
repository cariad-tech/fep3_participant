/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/base/component.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/clock/mock_clock.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/fep3_duration.h>

#include <chrono>
#include <condition_variable>
#include <functional>

using namespace ::testing;

namespace fep3 {
namespace mock {
// Event sink which stores values of time update and reset events
//

struct EventSinkTimeEventValues : public fep3::mock::experimental::Clock::EventSink {
    EventSinkTimeEventValues(
        const size_t expected_reset_calls_count,
        const size_t expected_update_calls_count = 0,
        std::function<void()> update_func = []() {})
        : _expected_reset_calls_count(expected_reset_calls_count),
          _expected_update_calls_count(expected_update_calls_count),
          _update_func(update_func)
    {
        ON_CALL(*this, timeUpdating(_, _)).WillByDefault(WithArg<0>([this](Timestamp new_time) {
            std::lock_guard<std::mutex> lk(_mutex);

            if (_update_calls.size() < _expected_update_calls_count) {
                _update_calls.push_back(new_time);
                _calls.push_back(new_time);
            }

            if (_update_calls.size() >= _expected_update_calls_count) {
                _expected_update_calls_reached.notify_all();
            }

            _update_func();
        }));

        ON_CALL(*this, timeResetEnd(_)).WillByDefault(Invoke([this](Timestamp new_time) {
            std::lock_guard<std::mutex> lk(_mutex);

            if (_reset_calls.size() < _expected_reset_calls_count) {
                _reset_calls.push_back(new_time);
                _calls.push_back(new_time);
            }

            if (_reset_calls.size() >= _expected_reset_calls_count) {
                _expected_reset_calls_reached.notify_all();
            }
        }));
    }

    bool waitForReset(std::chrono::milliseconds duration_ms)
    {
        std::unique_lock<std::mutex> lk(_mutex);
        bool res = _expected_reset_calls_reached.wait_for(
            lk, duration_ms, [&] { return _reset_calls.size() >= _expected_reset_calls_count; });
        return res;
    }

    bool waitForUpdate(std::chrono::milliseconds duration_ms)
    {
        std::unique_lock<std::mutex> lk(_mutex);
        bool res = _expected_update_calls_reached.wait_for(
            lk, duration_ms, [&] { return _update_calls.size() >= _expected_update_calls_count; });
        return res;
    }

    size_t _expected_reset_calls_count;
    size_t _expected_update_calls_count;

    std::condition_variable _expected_reset_calls_reached;
    std::condition_variable _expected_update_calls_reached;

    std::vector<Timestamp> _reset_calls{};
    std::vector<Timestamp> _update_calls{};
    std::vector<Timestamp> _calls{};
    std::function<void()> _update_func;

    std::mutex _mutex;
};

// Event sink which stores frequency of (the duration between) time update
// events
struct EventSinkTimeEventFrequency : fep3::mock::experimental::Clock::EventSink {
    EventSinkTimeEventFrequency(const size_t expected_calls_count)
        : _expected_calls_count(expected_calls_count)
    {
        using namespace ::testing;
        using namespace std::chrono;

        _call_durations.reserve(expected_calls_count);

        ON_CALL(*this, timeUpdating(_, _)).WillByDefault(InvokeWithoutArgs([this]() {
            if (_call_durations.size() == 0) {
                _last_time_event_occured = steady_clock::now();
                _call_durations.push_back(Timestamp{0});
            }
            else if (_call_durations.size() < _expected_calls_count) {
                _call_durations.push_back(steady_clock::now() - _last_time_event_occured);
                _last_time_event_occured = steady_clock::now();
            }
            else if (_call_durations.size() >= _expected_calls_count) {
                {
                    std::lock_guard<std::mutex> lk(_mutex);
                    _stop = true;
                }
                _expected_calls_reached.notify_all();
            }
        }));
    }

    void assertTimeEventDeviation(Timestamp expected_event_duration,
                                  Timestamp allowed_deviation = Timestamp{0})
    {
        for (const auto& time_actual: _call_durations) {
            if (0 != time_actual.count()) {
                EXPECT_NEAR(static_cast<double>(time_actual.count()),
                            static_cast<double>(expected_event_duration.count()),
                            static_cast<double>(allowed_deviation.count()));
            }
        }
    }

    bool waitFor(std::chrono::milliseconds duration_ms)
    {
        {
            std::unique_lock<std::mutex> lk(_mutex);
            _stop = false;
            return _expected_calls_reached.wait_for(lk, duration_ms, [&] { return _stop == true; });
        }
    }

    std::chrono::time_point<std::chrono::steady_clock> _last_time_event_occured;
    size_t _expected_calls_count;
    std::condition_variable _expected_calls_reached;
    std::vector<Timestamp> _call_durations{};

    std::atomic_bool _stop{false};
    std::mutex _mutex;
};

struct ClockServiceComponentWithDefaultBehaviour : public ClockService {
    ClockServiceComponentWithDefaultBehaviour()
    {
        using namespace ::testing;

        ON_CALL(*this, getTime()).WillByDefault(Invoke([]() { return Timestamp(0); }));
        ON_CALL(*this, getTime(_)).WillByDefault(Invoke([](std::string /*clock_name*/) {
            return Optional<Timestamp>{Timestamp(0)};
        }));

        ON_CALL(*this,
                registerEventSink(
                    ::testing::Matcher<const std::weak_ptr<fep3::IClock::IEventSink>&>(_)))
            .WillByDefault(
                Invoke([this](std::weak_ptr<fep3::IClock::IEventSink>) { return fep3::Result{}; }));

        ON_CALL(*this,
                unregisterEventSink(
                    ::testing::Matcher<const std::weak_ptr<fep3::IClock::IEventSink>&>(_)))
            .WillByDefault(
                Invoke([this](std::weak_ptr<fep3::IClock::IEventSink>) { return fep3::Result{}; }));
    }
};

struct DiscreteSteppingClockService
    : public fep3::base::Component<fep3::arya::IClockService, fep3::experimental::IClockService> {
    MOCK_CONST_METHOD0(getTime, Timestamp());
    MOCK_CONST_METHOD1(getTime, Optional<Timestamp>(const std::string&));

    MOCK_CONST_METHOD0(getType, fep3::arya::IClock::ClockType());
    MOCK_CONST_METHOD1(getType, Optional<fep3::arya::IClock::ClockType>(const std::string&));

    MOCK_CONST_METHOD0(getMainClockName, std::string());

    MOCK_METHOD1(registerEventSink,
                 fep3::Result(const std::weak_ptr<fep3::arya::IClock::IEventSink>&));
    MOCK_METHOD1(registerEventSink,
                 fep3::Result(const std::weak_ptr<fep3::experimental::IClock::IEventSink>&));
    MOCK_METHOD1(unregisterEventSink,
                 fep3::Result(const std::weak_ptr<fep3::arya::IClock::IEventSink>&));

    MOCK_METHOD1(unregisterEventSink,
                 fep3::Result(const std::weak_ptr<fep3::experimental::IClock::IEventSink>&));

    MOCK_METHOD1(registerClock, fep3::Result(const std::shared_ptr<fep3::experimental::IClock>&));
    MOCK_METHOD1(registerClock, fep3::Result(const std::shared_ptr<fep3::arya::IClock>&));
    MOCK_METHOD1(unregisterClock, fep3::Result(const std::string&));

    MOCK_CONST_METHOD0(getClockNames, std::list<std::string>());
    MOCK_CONST_METHOD1(findClock, std::shared_ptr<fep3::arya::IClock>(const std::string&));
    MOCK_CONST_METHOD1(findClockCatelyn,
                       std::shared_ptr<fep3::experimental::IClock>(const std::string&));
    inline DiscreteSteppingClockService()
    {
        using namespace std::chrono;
        using namespace ::testing;

        ON_CALL(*this, getTime()).WillByDefault(Invoke([this]() {
            return Timestamp(_current_time);
        }));

        ON_CALL(*this, getTime(_)).WillByDefault(Invoke([this](std::string /*dont_care_for_name*/) {
            return Optional<Timestamp>{Timestamp(_current_time)};
        }));

        ON_CALL(
            *this,
            registerEventSink(
                ::testing::Matcher<const std::weak_ptr<fep3::experimental::IClock::IEventSink>&>(
                    _)))
            .WillByDefault(Return(fep3::Result{}));

        ON_CALL(
            *this,
            unregisterEventSink(
                ::testing::Matcher<const std::weak_ptr<fep3::experimental::IClock::IEventSink>&>(
                    _)))
            .WillByDefault(Return(fep3::Result{}));

        ON_CALL(*this, getType()).WillByDefault(Return(fep3::arya::IClock::ClockType::continuous));
    }

    void inline setCurrentTime(Timestamp current_time)
    {
        std::lock_guard<std::mutex> lock(_time_mutex);
        _current_time = current_time;
    }

    void inline incrementTime(Timestamp time_increment)
    {
        std::lock_guard<std::mutex> lock(_time_mutex);
        _current_time += time_increment;
    }

private:
    std::mutex _time_mutex;
    Timestamp _current_time{0};
};

struct ChronoDrivenClockService : public fep3::base::Component<fep3::experimental::IClockService> {
    MOCK_CONST_METHOD0(getTime, Timestamp());
    MOCK_CONST_METHOD1(getTime, Optional<Timestamp>(const std::string&));

    MOCK_CONST_METHOD0(getType, fep3::arya::IClock::ClockType());
    MOCK_CONST_METHOD1(getType, Optional<fep3::arya::IClock::ClockType>(const std::string&));

    MOCK_CONST_METHOD0(getMainClockName, std::string());

    MOCK_METHOD1(registerEventSink,
                 fep3::Result(const std::weak_ptr<fep3::experimental::IClock::IEventSink>&));
    MOCK_METHOD1(registerEventSink,
                 fep3::Result(const std::weak_ptr<fep3::arya::IClock::IEventSink>&));
    MOCK_METHOD1(unregisterEventSink,
                 fep3::Result(const std::weak_ptr<fep3::experimental::IClock::IEventSink>&));
    MOCK_METHOD1(unregisterEventSink,
                 fep3::Result(const std::weak_ptr<fep3::arya::IClock::IEventSink>&));

    MOCK_METHOD1(registerClock, fep3::Result(const std::shared_ptr<fep3::experimental::IClock>&));
    MOCK_METHOD1(registerClock, fep3::Result(const std::shared_ptr<fep3::arya::IClock>&));
    MOCK_METHOD1(unregisterClock, fep3::Result(const std::string&));

    MOCK_CONST_METHOD0(getClockNames, std::list<std::string>());
    MOCK_CONST_METHOD1(findClock, std::shared_ptr<fep3::arya::IClock>(const std::string&));
    MOCK_CONST_METHOD1(findClockCatelyn,
                       std::shared_ptr<fep3::experimental::IClock>(const std::string&));

    fep3::Result start() override
    {
        _current_offset = std::chrono::steady_clock::now();
        _started = true;
        return {};
    }

    fep3::Result stop() override
    {
        _started = false;
        return {};
    }

    inline ChronoDrivenClockService()
    {
        using namespace std::chrono;
        using namespace ::testing;

        ON_CALL(*this, getTime()).WillByDefault(Invoke([this]() { return getChronoTime(); }));

        ON_CALL(*this, getTime(_)).WillByDefault(Invoke([this](std::string /*dont_care_for_name*/) {
            return getChronoTime();
        }));

        ON_CALL(
            *this,
            registerEventSink(
                ::testing::Matcher<const std::weak_ptr<fep3::experimental::IClock::IEventSink>&>(
                    _)))
            .WillByDefault(Invoke([this](std::weak_ptr<fep3::experimental::IClock::IEventSink>) {
                return fep3::Result{};
            }));

        ON_CALL(
            *this,
            unregisterEventSink(
                ::testing::Matcher<const std::weak_ptr<fep3::experimental::IClock::IEventSink>&>(
                    _)))
            .WillByDefault(Invoke([this](std::weak_ptr<fep3::experimental::IClock::IEventSink>) {
                return fep3::Result{};
            }));
    }

private:
    fep3::Timestamp getChronoTime()
    {
        if (!_started) {
            return fep3::Timestamp{0};
        }
        return Timestamp{std::chrono::steady_clock::now() - _current_offset};
    }

private:
    std::mutex _time_mutex;
    bool _started{false};
    mutable std::chrono::time_point<std::chrono::steady_clock> _current_offset;
};

} // namespace mock
} // namespace fep3
