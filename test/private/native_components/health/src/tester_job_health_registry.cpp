/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/job_registry/mock_job.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/native_components/health/job_health_registry.h>

#include <boost/asio.hpp>

#include <common/gtest_asserts.h>

using namespace ::testing;
using namespace fep3;
using namespace std::chrono_literals;

using LoggerMock = NiceMock<fep3::mock::LoggerWithDefaultBehavior>;

struct JobHealthRegistryTest : public ::testing::Test {
protected:
    using JobHealthiness = fep3::native::IJobHealthRegistry::JobHealthiness;
    using ExecuteError = fep3::native::IJobHealthRegistry::JobHealthiness::ExecuteError;

    JobHealthRegistryTest()
    {
    }

    void SetUp() override
    {
        _logger_mock = std::make_shared<LoggerMock>();
        createJobs();
        _job_health_registry.initialize(_jobs, _logger_mock);
    }

    void createJobs()
    {
        auto job_mock{std::make_shared<NiceMock<fep3::mock::Job>>()};
        auto job_mock2{std::make_shared<NiceMock<fep3::mock::Job>>()};
        auto job_mock3{std::make_shared<NiceMock<fep3::mock::Job>>()};
        _jobs.emplace(
            "job1",
            JobEntry{std::shared_ptr<IJob>{job_mock}, fep3::JobInfo{"job1", fep3::Duration{100}}});
        _jobs.emplace(
            "job2",
            JobEntry{std::shared_ptr<IJob>{job_mock2}, fep3::JobInfo{"job2", fep3::Duration{10}}});
        _jobs.emplace("job3",
                      JobEntry{std::shared_ptr<IJob>{job_mock3},
                               fep3::JobInfo{"job3",
                                             std::make_unique<fep3::DataTriggeredJobConfiguration>(
                                                 std::vector<std::string>{"signal_name"})}});
    }

    void checkLastErrorStruct(const ExecuteError& last_execution_error,
                              const ExecuteError& expected_last_execution_error)
    {
        ASSERT_EQ(last_execution_error.error_count, expected_last_execution_error.error_count);
        ASSERT_EQ(last_execution_error.simulation_time,
                  expected_last_execution_error.simulation_time);
        ASSERT_EQ(last_execution_error.last_error.getErrorCode(),
                  expected_last_execution_error.last_error.getErrorCode());
        if (expected_last_execution_error.last_error == fep3::Result{fep3::ERR_NOERROR}) {
            ASSERT_EQ(last_execution_error.last_error.getDescription(),
                      fep3::Result{fep3::ERR_NOERROR}.getDescription());
        }
        else {
            ASSERT_EQ(last_execution_error.last_error.getDescription(),
                      expected_last_execution_error.last_error.getDescription());
        }
    }

    void checkHealthinessStruct(const std::string& job_name,
                                fep3::Timestamp simulation_time,
                                ExecuteError execute_data_in_error,
                                ExecuteError execute_error,
                                ExecuteError execute_data_out_error)
    {
        const std::vector<fep3::native::IJobHealthRegistry::JobHealthiness> jobs_healthiness =
            _job_health_registry.getHealth();
        const auto job_healthiness =
            std::find_if(jobs_healthiness.begin(),
                         jobs_healthiness.end(),
                         [&](const auto& job_health) { return job_health.job_name == job_name; });
        ASSERT_NE(job_healthiness, jobs_healthiness.cend());
        ASSERT_EQ(job_healthiness->simulation_time, simulation_time);
        ASSERT_NO_FATAL_FAILURE(
            checkLastErrorStruct(job_healthiness->execute_data_in_error, execute_data_in_error));
        ASSERT_NO_FATAL_FAILURE(
            checkLastErrorStruct(job_healthiness->execute_error, execute_error));
        ASSERT_NO_FATAL_FAILURE(
            checkLastErrorStruct(job_healthiness->execute_data_out_error, execute_data_out_error));
    }

    fep3::Jobs _jobs;
    fep3::native::JobHealthRegistry _job_health_registry;
    std::shared_ptr<LoggerMock> _logger_mock{};
};

TEST_F(JobHealthRegistryTest, checkJobHealthinessOnStart)
{
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job1", 0ns, {}, {}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job2", 0ns, {}, {}, {}));

    const std::vector<fep3::native::IJobHealthRegistry::JobHealthiness> jobs_healthiness =
        _job_health_registry.getHealth();
    auto job_healthiness =
        std::find_if(jobs_healthiness.begin(), jobs_healthiness.end(), [&](const auto& job_health) {
            return job_health.job_name == "job1";
        });
    ASSERT_EQ(std::get<fep3::native::IJobHealthRegistry::JobHealthiness::ClockTriggeredJobInfo>(
                  job_healthiness->job_info)
                  .cycle_time,
              100ns);
    job_healthiness =
        std::find_if(jobs_healthiness.begin(), jobs_healthiness.end(), [&](const auto& job_health) {
            return job_health.job_name == "job2";
        });
    ASSERT_EQ(std::get<fep3::native::IJobHealthRegistry::JobHealthiness::ClockTriggeredJobInfo>(
                  job_healthiness->job_info)
                  .cycle_time,
              10ns);
    job_healthiness =
        std::find_if(jobs_healthiness.begin(), jobs_healthiness.end(), [&](const auto& job_health) {
            return job_health.job_name == "job3";
        });
    ASSERT_EQ(std::get<fep3::native::IJobHealthRegistry::JobHealthiness::DataTriggeredJobInfo>(
                  job_healthiness->job_info)
                  .trigger_signals[0],
              "signal_name");
}

TEST_F(JobHealthRegistryTest, testJobHealthinessUpdate)
{
    for (uint8_t i = 0; i < 11; ++i) {
        ASSERT_FEP3_NOERROR(_job_health_registry.updateJobStatus(
            "job1",
            {std::chrono::nanoseconds(100 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
        ASSERT_FEP3_NOERROR(_job_health_registry.updateJobStatus(
            "job2",
            {std::chrono::nanoseconds(10 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
    }

    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job1", 1000ns, {}, {}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job2", 100ns, {}, {}, {}));

    for (uint8_t i = 11; i < 21; ++i) {
        ASSERT_FEP3_NOERROR(_job_health_registry.updateJobStatus(
            "job1",
            {std::chrono::nanoseconds(100 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
    }

    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job1", 2000ns, {}, {}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job2", 100ns, {}, {}, {}));

    // make job return error
    auto execute_error_description = CREATE_ERROR_DESCRIPTION(-20, "Job1 went wrong");
    _job_health_registry.updateJobStatus(
        "job1", {2100ns, fep3::Result{}, execute_error_description, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessStruct("job1", 2100ns, {}, {1, 2100ns, execute_error_description}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job2", 100ns, {}, {}, {}));

    // always last error is returned
    execute_error_description = CREATE_ERROR_DESCRIPTION(-22, "Job1 went wrong again");
    _job_health_registry.updateJobStatus(
        "job1", {2200ns, fep3::Result{}, execute_error_description, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessStruct("job1", 2200ns, {}, {2, 2200ns, execute_error_description}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job2", 100ns, {}, {}, {}));

    // after a successful job execute, last error is kept
    _job_health_registry.updateJobStatus(
        "job1", {std::chrono::nanoseconds(2300ns), fep3::Result{}, fep3::Result{}, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessStruct("job1", 2300ns, {}, {2, 2200ns, execute_error_description}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job2", 100ns, {}, {}, {}));

    // for a good measure test job2 also
    for (uint8_t i = 11; i < 21; ++i) {
        ASSERT_FEP3_NOERROR(_job_health_registry.updateJobStatus(
            "job2",
            {std::chrono::nanoseconds(10 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
    }
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job2", 200ns, {}, {}, {}));
    // make job return error on execute_data_in
    execute_error_description = CREATE_ERROR_DESCRIPTION(-22, "Job2 went wrong");
    _job_health_registry.updateJobStatus(
        "job2", {210ns, execute_error_description, fep3::Result{}, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessStruct("job2", 210ns, {1, 210ns, execute_error_description}, {}, {}));
}

TEST_F(JobHealthRegistryTest, testJobsHealthinessClearAfterDeInitialize)
{
    std::vector<fep3::native::IJobHealthRegistry::JobHealthiness> jobs_healthiness =
        _job_health_registry.getHealth();
    ASSERT_EQ(_jobs.size(), jobs_healthiness.size());
    _job_health_registry.deinitialize();

    jobs_healthiness = _job_health_registry.getHealth();
    ASSERT_TRUE(jobs_healthiness.empty());
}

TEST_F(JobHealthRegistryTest, testUpdateNonExistingJob)
{
    ASSERT_FEP3_RESULT(
        _job_health_registry.updateJobStatus(
            "jobNotExisting",
            {std::chrono::nanoseconds(100ns), fep3::Result{}, fep3::Result{}, fep3::Result{}}),
        fep3::Result{ERR_NOT_FOUND});
}

TEST_F(JobHealthRegistryTest, testResetHealth)
{
    for (uint8_t i = 0; i < 21; ++i) {
        ASSERT_FEP3_NOERROR(_job_health_registry.updateJobStatus(
            "job1",
            {std::chrono::nanoseconds(100 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
        ASSERT_FEP3_NOERROR(_job_health_registry.updateJobStatus(
            "job2",
            {std::chrono::nanoseconds(10 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
    }
    auto execute_error_description = CREATE_ERROR_DESCRIPTION(-20, "Job1 went wrong");
    _job_health_registry.updateJobStatus(
        "job1", {2100ns, fep3::Result{}, execute_error_description, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessStruct("job1", 2100ns, {}, {1, 2100ns, execute_error_description}, {}));

    execute_error_description = CREATE_ERROR_DESCRIPTION(-22, "Job2 went wrong");
    _job_health_registry.updateJobStatus(
        "job2", {210ns, fep3::Result{}, execute_error_description, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessStruct("job2", 210ns, {}, {1, 210ns, execute_error_description}, {}));

    ASSERT_FEP3_NOERROR(_job_health_registry.resetHealth());
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job1", 2100ns, {}, {}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct("job2", 210ns, {}, {}, {}));
}

TEST_F(JobHealthRegistryTest, testMultiThread)
{
    // Launch the pool with ten threads.
    const size_t number_of_test_threads = 10;
    boost::asio::thread_pool pool(number_of_test_threads);

    const uint32_t number_of_success_updates = 50;
    const uint32_t number_of_error_updates = 50;

    for (uint8_t i = 0; i < number_of_success_updates; ++i) {
        // Submit a function to the pool.
        boost::asio::post(pool, [&]() {
            ASSERT_FEP3_NOERROR(_job_health_registry.updateJobStatus(
                "job1", {100ns, fep3::Result{}, fep3::Result{}, fep3::Result{}}));
        });
    }

    const auto execute_error_description = CREATE_ERROR_DESCRIPTION(-20, "Job1 went wrong");
    for (uint8_t i = 0; i < number_of_error_updates; ++i) {
        boost::asio::post(pool, [&]() {
            _job_health_registry.updateJobStatus(
                "job1", {100ns, fep3::Result{}, execute_error_description, fep3::Result{}});
        });
    }

    // Wait for all tasks in the pool to complete.
    pool.join();

    ASSERT_NO_FATAL_FAILURE(checkHealthinessStruct(
        "job1", 100ns, {}, {number_of_error_updates, 100ns, execute_error_description}, {}));
}
