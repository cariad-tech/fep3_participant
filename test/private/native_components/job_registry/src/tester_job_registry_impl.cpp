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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>
#include <chrono>

#include <fep3/native_components/job_registry/local_job_registry.h>
#include <fep3/core/job.h>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace fep3;
using namespace fep3::native;

struct JobRegistryImplTest : public testing::Test
{
    JobRegistryImplTest()
        : _job_registry_impl(_job_registry_configuration)
    {
    }

    JobRegistryConfiguration _job_registry_configuration;
    JobRegistryImpl _job_registry_impl;
};

/**
* @brief Add and remove a job
* @req_id FEPSDK-2085, FEPSDK-2086
*/
TEST_F(JobRegistryImplTest, AddRemoveJob)
{
    auto my_job = std::unique_ptr<fep3::core::Job>(new fep3::core::Job("name", duration_cast<Duration>(1us)));
    fep3::JobConfiguration my_config(duration_cast<Duration>(1us));

    ASSERT_FEP3_RESULT(_job_registry_impl.addJob("name", std::move(my_job), my_config), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(_job_registry_impl.removeJob("name"), fep3::ERR_NOERROR);
}

/**
* @brief Try to remove a job that is not existing
* @req_id FEPSDK-2086
*/
TEST_F(JobRegistryImplTest, RemoveNotExistingJob)
{
    ASSERT_FEP3_RESULT(_job_registry_impl.removeJob("not_existing_job"), fep3::ERR_NOT_FOUND);
}

/**
* @brief Add a job with same name twice
* @req_id FEPSDK-2085
*/
TEST_F(JobRegistryImplTest, AddJobTwice)
{
    fep3::JobConfiguration my_config(duration_cast<Duration>(1us));

    auto my_job = std::unique_ptr<fep3::core::Job>(new fep3::core::Job("job_name", duration_cast<Duration>(1us)));
    auto my_job2 = std::unique_ptr<fep3::core::Job>(new fep3::core::Job("job_name", duration_cast<Duration>(1us)));

    ASSERT_FEP3_RESULT(_job_registry_impl.addJob("job_name", std::move(my_job), my_config), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(_job_registry_impl.addJob("job_name", std::move(my_job2), my_config), fep3::ERR_RESOURCE_IN_USE);
}

/**
* @brief Execute GetJobs with 0 and 1 added job and check JobInfo
* @req_id FEPSDK-2087
*/
TEST_F(JobRegistryImplTest, GetJobsAndCheckJobInfo)
{
    using namespace std::chrono;

    // initally 0
    {
        auto jobs = _job_registry_impl.getJobInfos();
        ASSERT_EQ(jobs.size(), 0);
    }

    // add job
    {
        auto my_job = std::unique_ptr<fep3::core::Job>(new fep3::core::Job("some_other_jobname", duration_cast<Duration>(1us)));

        auto cycle_sim_time = 1us;
        auto delay_sim_time = 1us;
        Optional<Duration> max_runtime_real_time = 2us;
        auto runtime_violation_strategy = fep3::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation;

        fep3::JobConfiguration my_config(
            duration_cast<Duration>(cycle_sim_time),
            duration_cast<Duration>(delay_sim_time),
            max_runtime_real_time,
            runtime_violation_strategy);

        // add a job, size is 1 now
        auto job_name = "job_add_name";
        ASSERT_FEP3_RESULT(_job_registry_impl.addJob(job_name, std::move(my_job), my_config), fep3::ERR_NOERROR);

        auto jobs = _job_registry_impl.getJobInfos();
        ASSERT_EQ(jobs.size(), 1);

        // check the JobInfo content
        auto first_job = jobs.front();
        ASSERT_EQ(first_job.getName(), job_name);
        ASSERT_EQ(first_job.getConfig()._cycle_sim_time, cycle_sim_time);
        ASSERT_EQ(first_job.getConfig()._delay_sim_time, delay_sim_time);
        ASSERT_EQ(first_job.getConfig()._max_runtime_real_time, max_runtime_real_time);
        ASSERT_EQ(first_job.getConfig()._runtime_violation_strategy, runtime_violation_strategy);
    }
}
