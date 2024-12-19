/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/native_components/job_registry/job_configurations.h>

#include <common/gtest_asserts.h>
using namespace fep3;
using namespace native;
using namespace a_util::strings;
using namespace std::literals::chrono_literals;

namespace {

std::shared_ptr<base::NativePropertyNode> getJobEntryNode(
    const std::string& name,
    const std::string& cycle_time,
    const std::string& delay_time,
    const std::string& max_runtime,
    const std::string& time_violation_strategy)
{
    auto job_entry_node = std::make_shared<base::NativePropertyNode>(name);

    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_CYCLE_SIM_TIME_PROPERTY,
        base::DefaultPropertyTypeConversion<int32_t>::fromString(cycle_time)));
    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_DELAY_SIM_TIME_PROPERTY,
        base::DefaultPropertyTypeConversion<int32_t>::fromString(delay_time)));
    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY,
        base::DefaultPropertyTypeConversion<int32_t>::fromString(max_runtime)));
    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY, time_violation_strategy));

    return job_entry_node;
}

} // namespace

std::shared_ptr<base::NativePropertyNode> getJobEntryNodeClockTriggerType(
    const std::string& name,
    const std::string& cycle_time,
    const std::string& delay_time,
    const std::string& max_runtime,
    const std::string& time_violation_strategy)
{
    auto job_entry_node =
        getJobEntryNode(name, cycle_time, delay_time, max_runtime, time_violation_strategy);

    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_TRIGGER_TYPE_PROPERTY, std::string{FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY}));

    return job_entry_node;
}

std::shared_ptr<base::NativePropertyNode> getJobEntryNodeDataTriggerType(
    const std::string& name,
    const std::string& max_runtime,
    const std::string& time_violation_strategy,
    const std::vector<std::string>& signal_names)
{
    auto job_entry_node = std::make_shared<base::NativePropertyNode>(name);

    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY,
        base::DefaultPropertyTypeConversion<int32_t>::fromString(max_runtime)));

    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY, time_violation_strategy));

    job_entry_node->setChild(std::make_shared<base::NativePropertyNode>(
        FEP3_JOB_TRIGGER_TYPE_PROPERTY, std::string{FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY}));

    job_entry_node->setChild(base::makeNativePropertyNode<std::vector<std::string>>(
        FEP3_JOB_TRIGGER_SIGNAL_PROPERTY, signal_names));

    return job_entry_node;
}

struct JobConfigurationsParsing : public testing::Test {
    JobConfigurationsParsing()
    {
        _jobs_node->setChild(getJobEntryNode(
            _job_name1, _cycle_time1, _delay_time1, _max_runtime1, _time_violation_strategy1));
        _jobs_node->setChild(getJobEntryNode(
            _job_name2, _cycle_time2, _delay_time2, _max_runtime2, _time_violation_strategy2));
    }

    std::shared_ptr<base::NativePropertyNode> _jobs_node =
        std::make_shared<base::NativePropertyNode>(FEP3_JOBS_PROPERTY);
    const std::string _job_name1{"my_job1"}, _job_name2{"my_job2"}, _cycle_time1{"200"},
        _cycle_time2{"1"}, _delay_time1{"300"}, _delay_time2{"0"}, _max_runtime1{"1"},
        _max_runtime2{"0"},
        _time_violation_strategy1{JobConfiguration::toString(
            JobConfiguration::TimeViolationStrategy::ignore_runtime_violation)},
        _time_violation_strategy2{JobConfiguration::toString(
            JobConfiguration::TimeViolationStrategy::skip_output_publish)};
};

class TriggeredJobConfigurationParsing : public testing::Test,
                                         public catelyn::IJobConfigurationVisitor {
public:
    ~TriggeredJobConfigurationParsing() = default;

    TriggeredJobConfigurationParsing()
    {
        _jobs_node_triggered->setChild(getJobEntryNodeDataTriggerType(
            _job_data_triggered, _max_runtime1, _time_violation_strategy, _signal_names));
        _jobs_node_triggered->setChild(getJobEntryNodeClockTriggerType(_job_clock_triggered,
                                                                       _cycle_time2,
                                                                       _delay_time2,
                                                                       _max_runtime2,
                                                                       _time_violation_strategy));
    }

    virtual fep3::Result visitDataTriggeredConfiguration(
        const DataTriggeredJobConfiguration& configuration) override
    {
        EXPECT_EQ(configuration._max_runtime_real_time.value().count(),
                  a_util::strings::toInt64(_max_runtime1));
        EXPECT_EQ(configuration._runtime_violation_strategy,
                  JobConfiguration::fromString(_time_violation_strategy));
        EXPECT_EQ(configuration._signal_names, _signal_names);
        return {};
    }

    virtual fep3::Result visitClockTriggeredConfiguration(
        const ClockTriggeredJobConfiguration& configuration) override
    {
        EXPECT_EQ(configuration._cycle_sim_time.count(), a_util::strings::toInt64(_cycle_time2));
        EXPECT_EQ(configuration._delay_sim_time.count(), a_util::strings::toInt64(_delay_time2));
        EXPECT_EQ(configuration._max_runtime_real_time.value().count(),
                  a_util::strings::toInt64(_max_runtime2));
        EXPECT_EQ(configuration._runtime_violation_strategy,
                  JobConfiguration::fromString(_time_violation_strategy));
        return {};
    }

public:
    std::shared_ptr<base::NativePropertyNode> _jobs_node_triggered =
        std::make_shared<base::NativePropertyNode>(FEP3_JOBS_PROPERTY);
    const std::string _job_data_triggered{"data_triggered_job"},
        _job_clock_triggered{"clock_triggered_job"}, _cycle_time1{"200"}, _cycle_time2{"300"},
        _delay_time1{"100"}, _delay_time2{"200"}, _max_runtime1{"1"}, _max_runtime2{"2"},
        _time_violation_strategy{JobConfiguration::toString(
            JobConfiguration::TimeViolationStrategy::ignore_runtime_violation)};
    std::vector<std::string> _signal_names{"data_signal1",
                                           "data_signal2"}; // accept multiple signals
};

/**
 * @brief A valid timing configuration is parsed correctly.
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
    EXPECT_EQ(job_config1._delay_sim_time.count(), a_util::strings::toInt64(_delay_time1));
    EXPECT_EQ(job_config1._max_runtime_real_time.value().count(),
              a_util::strings::toInt64(_max_runtime1));
    EXPECT_EQ(job_config1._runtime_violation_strategy,
              JobConfiguration::fromString(_time_violation_strategy1));

    const auto job_entry2 = job_configurations.find(_job_name2);
    ASSERT_NE(job_entry2, job_configurations.end());
    const auto job_config2 = job_entry2->second;
    EXPECT_EQ(job_config2._cycle_sim_time.count(), a_util::strings::toInt64(_cycle_time2));
    EXPECT_EQ(job_config2._delay_sim_time.count(), a_util::strings::toInt64(_delay_time2));
    EXPECT_FALSE(job_config2._max_runtime_real_time.has_value());
    EXPECT_EQ(job_config2._runtime_violation_strategy,
              JobConfiguration::fromString(_time_violation_strategy2));
}

/**
 * @brief Parsing a time triggered or date triggered job configuration
 */
TEST_F(TriggeredJobConfigurationParsing, ValidTriggeredJobConfigurations)
{
    JobConfigurationPtrs job_configurations;
    ASSERT_FEP3_NOERROR(
        readJobConfigurationsFromPropertyNode(*_jobs_node_triggered, job_configurations));

    EXPECT_EQ(2, job_configurations.size());

    const auto job_entry1 = job_configurations.find(_job_data_triggered);
    ASSERT_NE(job_entry1, job_configurations.end());
    const auto job_config1 = std::move(job_entry1->second);
    ASSERT_FEP3_NOERROR(job_config1->acceptVisitor(*this));

    const auto job_entry2 = job_configurations.find(_job_clock_triggered);
    ASSERT_NE(job_entry2, job_configurations.end());
    const auto job_config2 = std::move(job_entry2->second);
    ASSERT_FEP3_NOERROR(job_config2->acceptVisitor(*this));
}

/**
 * @brief Parsing a time triggered or date triggered job configuration into old job configuration
 */
TEST_F(TriggeredJobConfigurationParsing, ValidTriggeredJobConfigurationsWithInvalidArg)
{
    JobConfigurationPtrs job_configurations1;
    _jobs_node_triggered->getChild(_job_clock_triggered)
        ->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)
        ->setValue("unexpected_name");
    ASSERT_FEP3_RESULT(
        readJobConfigurationsFromPropertyNode(*_jobs_node_triggered, job_configurations1),
        ERR_INVALID_ARG);
    EXPECT_EQ(1, job_configurations1.size());

    JobConfigurationPtrs job_configurations2;
    _jobs_node_triggered->getChild(_job_data_triggered)
        ->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)
        ->setValue("unexpected_name");
    ASSERT_FEP3_RESULT(
        readJobConfigurationsFromPropertyNode(*_jobs_node_triggered, job_configurations2),
        ERR_INVALID_ARG);
    EXPECT_EQ(0, job_configurations2.size());
}

/**
 * @brief Parsing a time triggered or date triggered job configuration into old job configuration
 */
TEST_F(TriggeredJobConfigurationParsing, ValidJobConfigurationsWithDataTriggeredConfig)
{
    JobConfigurations job_configurations;
    ASSERT_FEP3_NOERROR(
        readJobConfigurationsFromPropertyNode(*_jobs_node_triggered, job_configurations));
    EXPECT_EQ(1, job_configurations.size());

    const auto job_entry2 = job_configurations.find(_job_clock_triggered);
    ASSERT_NE(job_entry2, job_configurations.end());
    const auto job_config2 = job_entry2->second;
    EXPECT_EQ(job_config2._cycle_sim_time.count(), a_util::strings::toInt64(_cycle_time2));
    EXPECT_EQ(job_config2._delay_sim_time.count(), a_util::strings::toInt64(_delay_time2));
    EXPECT_EQ(job_config2._max_runtime_real_time.value().count(),
              a_util::strings::toInt64(_max_runtime2));
    EXPECT_EQ(job_config2._runtime_violation_strategy,
              JobConfiguration::fromString(_time_violation_strategy));
}

/**
 * @brief Parsing a time triggered or date triggered job configuration into old job configuration
 */
TEST_F(TriggeredJobConfigurationParsing, ValidJobConfigurationsWithInvalidArg)
{
    JobConfigurations job_configurations;
    _jobs_node_triggered->getChild(_job_clock_triggered)
        ->getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY)
        ->setValue("unexpected_name");
    ASSERT_FEP3_RESULT(
        readJobConfigurationsFromPropertyNode(*_jobs_node_triggered, job_configurations),
        ERR_INVALID_ARG);
    EXPECT_EQ(0, job_configurations.size());
}

/**
 * @brief Parsing a job configuration specifying an invalid cycle time shall return
 * the corresponding error.
 */
TEST_F(JobConfigurationsParsing, ErrorConfigInvalidCycleTime)
{
    // check invalid cycle time value
    _jobs_node->getChild(_job_name1)->getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)->setValue("0");

    JobConfigurations job_configurations;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Value has to be > 0.",
                                _job_name1.c_str(),
                                FEP3_JOB_CYCLE_SIM_TIME_PROPERTY));

    // check missing cycle time value
    _jobs_node->getChild(_job_name1)->getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY)->setValue("");

    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Value has to be > 0.",
                                _job_name1.c_str(),
                                FEP3_JOB_CYCLE_SIM_TIME_PROPERTY));
}

/**
 * @brief Parsing a job configuration specifying an invalid delay time shall return
 * the corresponding error.
 */
TEST_F(JobConfigurationsParsing, ErrorConfigInvalidDelayTime)
{
    // check invalid delay time value
    _jobs_node->getChild(_job_name2)->getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY)->setValue("-1");

    JobConfigurations job_configurations;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Value has to be >= 0.",
                                _job_name2.c_str(),
                                FEP3_JOB_DELAY_SIM_TIME_PROPERTY));
}

/**
 * @brief Parsing a job configuration specifying an invalid max runtime shall return
 * the corresponding error.
 */
TEST_F(JobConfigurationsParsing, ErrorConfigInvalidMaxRuntime)
{
    // check invalid max runtime value
    _jobs_node->getChild(_job_name1)
        ->getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY)
        ->setValue("-1");

    JobConfigurations job_configurations;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(".*'%s'. Invalid value '%s'. Value has to be >= 0.",
                                _job_name1.c_str(),
                                FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY));
}

/**
 * @brief Parsing a job configuration specifying an invalid runtime violation strategy shall return
 * the corresponding error.
 */
TEST_F(JobConfigurationsParsing, ErrorConfigInvalidRuntimeViolationStrategy)
{
    // check invalid max runtime value
    _jobs_node->getChild(_job_name1)
        ->getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY)
        ->setValue("");

    JobConfigurations job_configurations;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        readJobConfigurationsFromPropertyNode(*_jobs_node, job_configurations),
        fep3::ERR_INVALID_ARG,
        a_util::strings::format(
            ".*'%s'. Invalid value '%s'. Not a valid runtime violation strategy.",
            _job_name1.c_str(),
            FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY));
}
