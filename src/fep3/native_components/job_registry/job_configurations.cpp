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


#include "job_configurations.h"

#include <a_util/strings/strings_convert_decl.h>

namespace
{

using namespace fep3;
using namespace native;

std::string err_msg_node_missing{ "Invalid configuration for job '%s'. Missing subnode '%s'." };
std::string err_msg_node_invalid{ "Invalid configuration for job '%s'. Invalid value '%s'. %s." };


Result convertToDurationIfValidValue(const std::string& string_value,
                                     Duration& duration_value,
                                     const std::function<bool(Duration)>& validity_check,
                                     const std::string& job_name,
                                     const std::string& node_name,
                                     const std::string& error_message)
{
    duration_value = Duration{ a_util::strings::toInt64(string_value) };

    if (!validity_check(duration_value))
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                                 err_msg_node_invalid.c_str(),
                                 job_name.c_str(),
                                 node_name.c_str(),
                                 error_message.c_str());
    }

    return {};
}

Result convertToDurationIfValidValue(const std::string& string_value,
                                     Optional<Duration>& optional_duration_value,
                                     const std::function<bool(Duration)>& validity_check,
                                     const std::string& job_name,
                                     const std::string& node_name,
                                     const std::string& error_message)
{
    Duration tmp_value{};
    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(string_value,
                                                        tmp_value,
                                                        validity_check,
                                                        job_name,
                                                        node_name,
                                                        error_message));

    if (0 != tmp_value.count())
    {
        optional_duration_value = tmp_value;
    }

    return {};
}

Result parseCycleTimeNode(const IPropertyNode& job_entry,
                          Duration& configuration_value)
{
    const auto cycle_time_node = job_entry.getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY);
    if (!cycle_time_node)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_CYCLE_SIM_TIME_PROPERTY);
    }

    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(cycle_time_node->getValue(),
                                                        configuration_value,
                                                        [](const Duration duration) {return duration.count() > 0; },
                                                        job_entry.getName(),
                                                        FEP3_JOB_CYCLE_SIM_TIME_PROPERTY,
                                                        "Value has to be > 0"));

    return {};
}

Result parseDelayTimeNode(const IPropertyNode& job_entry,
                          Duration& configuration_value)
{
    const auto delay_time_node = job_entry.getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY);
    if (!delay_time_node)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_DELAY_SIM_TIME_PROPERTY);
    }

    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(delay_time_node->getValue(),
                                                        configuration_value,
                                                        [](const Duration duration) {return duration.count() >= 0; },
                                                        job_entry.getName(),
                                                        FEP3_JOB_DELAY_SIM_TIME_PROPERTY,
                                                        "Value has to be >= 0"));

    return {};
}

Result parseMaxRuntimeNode(const IPropertyNode& job_entry,
                           Optional<Duration>& configuration_value)
{
    const auto max_runtime_node = job_entry.getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY);
    if (!max_runtime_node)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY);
    }

    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(max_runtime_node->getValue(),
                                                        configuration_value,
                                                        [](const Duration duration) {return duration.count() >= 0; },
                                                        job_entry.getName(),
                                                        FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY,
                                                        "Value has to be >= 0"));

    return {};
}

Result parseRuntimeViolationStrategyNode(const IPropertyNode& job_entry,
                                         JobConfiguration::TimeViolationStrategy& configuration_value)
{
    const auto runtime_violation_strategy_node = job_entry.getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY);
    if (!runtime_violation_strategy_node)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY);
    }

    const auto runtime_violation_strategy = JobConfiguration::fromString(runtime_violation_strategy_node->getValue());
    if (JobConfiguration::TimeViolationStrategy::unknown == runtime_violation_strategy)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                                 err_msg_node_invalid.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY,
                                 "Not a valid runtime violation strategy");
    }

    configuration_value = runtime_violation_strategy;

    return {};
}

Result readJobConfigFromPropertyNode(
        const IPropertyNode& job_entry,
        JobConfiguration& job_configuration)
{
    FEP3_RETURN_IF_FAILED(parseCycleTimeNode(job_entry, job_configuration._cycle_sim_time));
    FEP3_RETURN_IF_FAILED(parseDelayTimeNode(job_entry, job_configuration._delay_sim_time));
    FEP3_RETURN_IF_FAILED(parseMaxRuntimeNode(job_entry, job_configuration._max_runtime_real_time));
    FEP3_RETURN_IF_FAILED(parseRuntimeViolationStrategyNode(job_entry, job_configuration._runtime_violation_strategy));

    return {};
}

} // namespace

namespace fep3
{
namespace native
{

Result readJobConfigurationsFromPropertyNode(
        const base::NativePropertyNode& jobs_node,
        JobConfigurations& job_configurations)
{
    job_configurations = JobConfigurations();
    const auto job_nodes = jobs_node.getChildren();

    for (const auto& job_entry : job_nodes)
    {
        JobConfiguration job_configuration{Duration{std::chrono::milliseconds{100}}};
        FEP3_RETURN_IF_FAILED(readJobConfigFromPropertyNode(*job_entry, job_configuration));
        job_configurations.insert(std::make_pair(
                                        job_entry->getName(), job_configuration));
    }

    return {};
}

} // namespace native
} // namespace fep3
