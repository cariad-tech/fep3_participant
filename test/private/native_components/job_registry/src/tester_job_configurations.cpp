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

#include <a_util/strings/strings_convert_decl.h>
#include <a_util/filesystem/path.h>
#include <common/gtest_asserts.h>

#include <fep3/native_components/job_registry/job_configurations.h>
#include <fep3/base/properties/propertynode.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/fep3_duration.h>
#include <fep3/core/job.h>

using namespace fep3;
using namespace native;
using namespace a_util::strings;

namespace
{

std::shared_ptr<base::NativePropertyNode> getJobEntryNode(
        const std::string& name,
        const std::string& cycle_time,
        const std::string& delay_time,
        const std::string& max_runtime,
        const std::string& time_violation_strategy)
{
    auto job_entry_node = std::make_shared<base::NativePropertyNode>(name);

    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY,
                                                                  cycle_time,
                                                                  base::PropertyType<int32_t>::getTypeName()));
    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(FEP3_JOB_DELAY_SIM_TIME_PROPERTY,
                                                                  delay_time,
                                                                  base::PropertyType<int32_t>::getTypeName()));
    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY,
                                                                  max_runtime,
                                                                  base::PropertyType<int32_t>::getTypeName()));
    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY,
                                                                  time_violation_strategy,
                                                                  base::PropertyType<std::string>::getTypeName()));

    return job_entry_node;
}

}

struct JobConfigurationsParsing : public testing::Test
{
    JobConfigurationsParsing()
    {
        _jobs_node->setChild(getJobEntryNode(
                                _job_name1,
                                _cycle_time1,
                                _delay_time1,
                                _max_runtime1,
                                _time_violation_strategy1));
        _jobs_node->setChild(getJobEntryNode(
                                _job_name2,
                                _cycle_time2,
                                _delay_time2,
                                _max_runtime2,
                                _time_violation_strategy2));
    }

    std::shared_ptr<base::NativePropertyNode> _jobs_node = std::make_shared<base::NativePropertyNode>(FEP3_JOBS_PROPERTY);
    const std::string _job_name1{"my_job1"}, _job_name2{"my_job2"},
    _cycle_time1{"200"}, _cycle_time2{"1"},
    _delay_time1{"300"}, _delay_time2{"0"},
    _max_runtime1{"1"}, _max_runtime2{"0"},
    _time_violation_strategy1{JobConfiguration::toString(
                    JobConfiguration::TimeViolationStrategy::ignore_runtime_violation)}
    , _time_violation_strategy2{JobConfiguration::toString(
                    JobConfiguration::TimeViolationStrategy::skip_output_publish)};
    };

/**
* @brief A valid timing configuration is parsed correctly.
*
*/
TEST_F(JobConfigurationsParsing, ValidJobConfigurations)
{
    JobConfigurations job_configurations;
    ASSERT_FEP3_NOERROR(readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations));

    EXPECT_EQ(2, job_configurations.size());

    const auto job_entry1 = job_configurations.find(_job_name1);
    ASSERT_NE(job_entry1, job_configurations.end());
    const auto job_config1 = job_entry1->second;

    EXPECT_EQ(job_config1._cycle_sim_time.count(), a_util::strings::toInt64(_cycle_time1));
    EXPECT_EQ(job_config1._delay_sim_time.count(), a_util::strings::toInt64(_delay_time1) );
    EXPECT_EQ(job_config1._max_runtime_real_time.value().count(), a_util::strings::toInt64(_max_runtime1) );
    EXPECT_EQ(job_config1._runtime_violation_strategy,
              JobConfiguration::fromString(_time_violation_strategy1));

    const auto job_entry2 = job_configurations.find(_job_name2);
    ASSERT_NE(job_entry2, job_configurations.end());
    const auto job_config2 = job_entry2->second;
    EXPECT_EQ(job_config2._cycle_sim_time.count(), a_util::strings::toInt64(_cycle_time2) );
    EXPECT_EQ(job_config2._delay_sim_time.count(), a_util::strings::toInt64(_delay_time2) );
    EXPECT_FALSE(job_config2._max_runtime_real_time.has_value() );
    EXPECT_EQ(job_config2._runtime_violation_strategy,
              JobConfiguration::fromString(_time_violation_strategy2));
}

/**
* @brief Parsing a job configuration specifying an invalid cycle time shall return
* the corresponding error.
*
*/
TEST_F(JobConfigurationsParsing, ErrorConfigInvalidCycleTime)
{
    // check invalid cycle time value
    _jobs_node->getChild(_job_name1)->getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)->setValue("0");

    JobConfigurations job_configurations;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Value has to be > 0.", _job_name1.c_str(), FEP3_JOB_CYCLE_SIM_TIME_PROPERTY));

    // check missing cycle time value
    _jobs_node->getChild(_job_name1)->getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)->setValue("");

    ASSERT_FEP3_RESULT_WITH_MESSAGE(readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Value has to be > 0.", _job_name1.c_str(), FEP3_JOB_CYCLE_SIM_TIME_PROPERTY));
}

/**
* @brief Parsing a job configuration specifying an invalid delay time shall return
* the corresponding error.
*
*/
TEST_F(JobConfigurationsParsing, ErrorConfigInvalidDelayTime)
{
    // check invalid delay time value
    _jobs_node->getChild(_job_name2)->getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY)->setValue("-1");

    JobConfigurations job_configurations;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Value has to be >= 0.", _job_name2.c_str(), FEP3_JOB_DELAY_SIM_TIME_PROPERTY));
}

/**
* @brief Parsing a job configuration specifying an invalid max runtime shall return
* the corresponding error.
*
*/
TEST_F(JobConfigurationsParsing, ErrorConfigInvalidMaxRuntime)
{
    // check invalid max runtime value
    _jobs_node->getChild(_job_name1)->getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)->setValue("-1");

    JobConfigurations job_configurations;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Value has to be >= 0.", _job_name1.c_str(), FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY));
}

/**
* @brief Parsing a job configuration specifying an invalid runtime violation strategy shall return
* the corresponding error.
*
*/
TEST_F(JobConfigurationsParsing, ErrorConfigInvalidRuntimeViolationStrategy)
{
    // check invalid max runtime value
    _jobs_node->getChild(_job_name1)->getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY)->setValue("");

    JobConfigurations job_configurations;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Not a valid runtime violation strategy.", _job_name1.c_str(), FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY));
}
