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

#include <fep3/core/job.h>

#include <gtest/gtest.h>

#include <condition_variable>
#include <thread>

namespace fep3 {
namespace test {
namespace helper {

using namespace std::literals::chrono_literals;

struct SimpleJobBuilder {
    SimpleJobBuilder(const std::string job_name = "my_job",
                     Duration cycle_time = 1ns,
                     Duration delay_time = 0ns)
        : _job_name(job_name),
          _data_job_config(std::vector<std::string>{"my_signal"}),
          _job_config(cycle_time, delay_time)
    {
    }

    SimpleJobBuilder(const std::string job_name, const std::vector<std::string>& signal_names)
        : _job_name(job_name), _data_job_config(signal_names), _job_config(0ns, 0ns)
    {
    }

    template <typename T>
    std::shared_ptr<T> makeJob(Timestamp expected_call_time) const
    {
        auto my_job = std::shared_ptr<T>(new T(_job_name, _job_config, expected_call_time));
        return my_job;
    }

    template <typename T>
    std::shared_ptr<T> makeJob() const
    {
        auto my_job = std::shared_ptr<T>(new T(_job_name, _job_config));
        return my_job;
    }

    template <typename T>
    std::shared_ptr<T> makeDataTriggeredJob() const
    {
        auto my_job = std::shared_ptr<T>(new T(_job_name, _data_job_config));
        return my_job;
    }

    std::shared_ptr<fep3::core::Job> makeJob() const
    {
        auto my_job = std::shared_ptr<fep3::core::Job>(new fep3::core::Job(_job_name, _job_config));
        return my_job;
    }

    std::shared_ptr<fep3::core::Job> makeClockJob() const
    {
        auto my_job =
            std::shared_ptr<fep3::core::Job>(new fep3::core::Job(_job_name, makeClockJobConfig()));
        return my_job;
    }

    std::shared_ptr<fep3::core::Job> makeDataJob() const
    {
        auto my_job =
            std::shared_ptr<fep3::core::Job>(new fep3::core::Job(_job_name, _data_job_config));
        return my_job;
    }

    fep3::arya::JobInfo makeJobInfo() const
    {
        return fep3::arya::JobInfo(_job_name, _job_config);
    }

    fep3::arya::JobConfiguration makeJobConfig() const
    {
        return _job_config;
    }

    fep3::ClockTriggeredJobConfiguration makeClockJobConfig() const
    {
        return fep3::ClockTriggeredJobConfiguration(_job_config);
    }

    fep3::DataTriggeredJobConfiguration makeDataJobConfig() const
    {
        return _data_job_config;
    }

    fep3::JobInfo makeJobInfoClockTriggered() const
    {
        return fep3::JobInfo(_job_name, _job_config);
    }

    fep3::JobInfo makeJobInfoDataTriggered() const
    {
        return fep3::JobInfo(
            _job_name, std::make_unique<catelyn::DataTriggeredJobConfiguration>(_data_job_config));
    }

    fep3::ClockTriggeredJobConfiguration makeClockTriggeredJobConfig() const
    {
        return fep3::ClockTriggeredJobConfiguration(_job_config);
    }

    const std::string _job_name;
    const std::string _signal_name;
    const fep3::arya::JobConfiguration _job_config;
    const fep3::DataTriggeredJobConfiguration _data_job_config;
};

class TestJob : public fep3::core::Job {
public:
    TestJob(std::string name,
            fep3::arya::JobConfiguration config,
            Timestamp expected_call_time = Timestamp(std::chrono::nanoseconds(0)))
        : fep3::core::Job(name, config),
          _expected_call_time(expected_call_time),
          _job_config(config)
    {
    }

    virtual fep3::Result execute(Timestamp time_of_execution) override
    {
        std::unique_lock<std::mutex> lk(_my_mutex);

        _calls.push_back(time_of_execution);
        // std::cout << "Job was called with time " << time_of_execution.count() << std::endl;
        if (_calls.size() > 0 && _calls.back() >= _expected_call_time) {
            _expected_calls_reached.notify_all();
        }
        return {};
    }

    void assertNumberOfCalls(Timestamp max_time)
    {
        std::unique_lock<std::mutex> lk(_my_mutex);

        EXPECT_EQ(_calls.size(), (max_time / _job_config._cycle_sim_time) + 1ull);
    }

    void assertNumberOfCalls(size_t expected_calls)
    {
        std::unique_lock<std::mutex> lk(_my_mutex);

        EXPECT_EQ(_calls.size(), expected_calls);
    }

    void assertCallTimeResolution(Timestamp allowed_step_deviation = Timestamp(0)) const
    {
        using namespace std::chrono;

        std::unique_lock<std::mutex> lk(_my_mutex);
        (void)allowed_step_deviation;

        Timestamp time_expected = 0us;
        // It is guaranteed to be a ClockTriggeredJobConfiguration
        const auto cycle_time = _job_config._cycle_sim_time;

        for (const auto& time_actual: _calls) {
            EXPECT_NEAR(static_cast<double>(time_actual.count()),
                        static_cast<double>(time_expected.count()),
                        static_cast<double>(allowed_step_deviation.count()));

            time_expected += cycle_time;
        }
    }

    void assertSteadilyRisingCallTimes() const
    {
        std::unique_lock<std::mutex> lk(_my_mutex);

        Timestamp last_call_time{-1};

        for (const auto& time_actual: _calls) {
            ASSERT_GT(time_actual.count(), last_call_time.count());

            last_call_time = time_actual;
        }
    }

    void waitForExpectedCallTime(Timestamp timeout)
    {
        using namespace std::chrono;
        std::unique_lock<std::mutex> lk(_my_mutex);

        if (!expectedCallTimeReached()) {
            ASSERT_NE(_expected_calls_reached.wait_for(lk, timeout), std::cv_status::timeout);
        }
    }

public:
    Timestamp _expected_call_time;
    const fep3::arya::JobConfiguration _job_config;

private:
    std::vector<Timestamp> _calls{};

    std::condition_variable _expected_calls_reached;
    mutable std::mutex _my_mutex;

    bool expectedCallTimeReached()
    {
        return _calls.size() > 0 && _calls.back() >= _expected_call_time;
    }
};

class SleepingJob : public TestJob {
public:
    SleepingJob(std::string name,
                fep3::arya::JobConfiguration config,
                Duration sleep_time = Duration(0),
                Timestamp expected_call_time = Timestamp(0))
        : TestJob(name, config, expected_call_time), _sleep_time(sleep_time)
    {
    }

    fep3::Result execute(Timestamp time_of_execution) override
    {
        auto result = TestJob::execute(time_of_execution);
        std::this_thread::sleep_for(_sleep_time);
        return result;
    }

private:
    Duration _sleep_time;
};

} // namespace helper
} // namespace test
} // namespace fep3
