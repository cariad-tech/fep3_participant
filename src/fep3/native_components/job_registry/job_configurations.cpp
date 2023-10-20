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

#include "job_configurations.h"

#include <functional>

namespace {

using namespace fep3;
using namespace native;
using namespace std::literals::chrono_literals;

std::string err_msg_node_missing{"Invalid configuration for job '%s'. Missing subnode '%s'."};
std::string err_msg_node_invalid{"Invalid configuration for job '%s'. Invalid value '%s'. %s."};

Result convertToDurationIfValidValue(const std::string& string_value,
                                     Duration& duration_value,
                                     const std::function<bool(Duration)>& validity_check,
                                     const std::string& job_name,
                                     const std::string& node_name,
                                     const std::string& error_message)
{
    duration_value = Duration{a_util::strings::toInt64(string_value)};

    if (!validity_check(duration_value)) {
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
    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(
        string_value, tmp_value, validity_check, job_name, node_name, error_message));

    if (0 != tmp_value.count()) {
        optional_duration_value = tmp_value;
    }

    return {};
}

Result parseCycleTimeNode(const IPropertyNode& job_entry, Duration& configuration_value)
{
    const auto cycle_time_node = job_entry.getChild(FEP3_JOB_CYCLE_SIM_TIME_PROPERTY);
    if (!cycle_time_node) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_CYCLE_SIM_TIME_PROPERTY);
    }

    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(
        cycle_time_node->getValue(),
        configuration_value,
        [](const Duration duration) { return duration.count() > 0; },
        job_entry.getName(),
        FEP3_JOB_CYCLE_SIM_TIME_PROPERTY,
        "Value has to be > 0"));

    return {};
}

Result parseDelayTimeNode(const IPropertyNode& job_entry, Duration& configuration_value)
{
    const auto delay_time_node = job_entry.getChild(FEP3_JOB_DELAY_SIM_TIME_PROPERTY);
    if (!delay_time_node) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_DELAY_SIM_TIME_PROPERTY);
    }

    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(
        delay_time_node->getValue(),
        configuration_value,
        [](const Duration duration) { return duration.count() >= 0; },
        job_entry.getName(),
        FEP3_JOB_DELAY_SIM_TIME_PROPERTY,
        "Value has to be >= 0"));

    return {};
}

Result parseMaxRuntimeNode(const IPropertyNode& job_entry, Optional<Duration>& configuration_value)
{
    const auto max_runtime_node = job_entry.getChild(FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY);
    if (!max_runtime_node) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY);
    }

    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(
        max_runtime_node->getValue(),
        configuration_value,
        [](const Duration duration) { return duration.count() >= 0; },
        job_entry.getName(),
        FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY,
        "Value has to be >= 0"));

    return {};
}

Result parseRuntimeViolationStrategyNode(
    const IPropertyNode& job_entry,
    arya::JobConfiguration::TimeViolationStrategy& configuration_value)
{
    const auto runtime_violation_strategy_node =
        job_entry.getChild(FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY);

    if (!runtime_violation_strategy_node) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY);
    }

    const auto runtime_violation_strategy =
        arya::JobConfiguration::fromString(runtime_violation_strategy_node->getValue());

    if (arya::JobConfiguration::TimeViolationStrategy::unknown == runtime_violation_strategy) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                                 err_msg_node_invalid.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY,
                                 "Not a valid runtime violation strategy");
    }

    configuration_value = runtime_violation_strategy;

    return {};
}

Result parseJobTriggerSignalNode(const IPropertyNode& job_entry,
                                 std::vector<std::string>& trigger_signal_names)
{
    const auto trigger_signal_name_node = job_entry.getChild(FEP3_JOB_TRIGGER_SIGNAL_PROPERTY);

    if (!trigger_signal_name_node) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_TRIGGER_TYPE_PROPERTY);
    }

    trigger_signal_names = base::getPropertyValue<std::vector<std::string>>(
        *job_entry.getChild(FEP3_JOB_TRIGGER_SIGNAL_PROPERTY));
    return {};
}

Result parseJobTriggerTypeNode(const IPropertyNode& job_entry, std::string& trigger_type)
{
    const auto trigger_type_node = job_entry.getChild(FEP3_JOB_TRIGGER_TYPE_PROPERTY);
    if (!trigger_type_node) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 err_msg_node_missing.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_TRIGGER_TYPE_PROPERTY);
    }

    trigger_type = trigger_type_node->getValue();

    if (trigger_type != FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY &&
        trigger_type != FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY) {
        std::string error_msg =
            a_util::strings::format("Invalid Job Trigger Type %s", trigger_type.c_str());
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                                 err_msg_node_invalid.c_str(),
                                 job_entry.getName().c_str(),
                                 FEP3_JOB_TRIGGER_TYPE_PROPERTY,
                                 error_msg.c_str());
    }

    return {};
}

Result readJobConfigFromPropertyNode(const IPropertyNode& job_entry,
                                     arya::JobConfiguration& job_configuration)
{
    std::string trigger_type;
    Result trigger_result = parseJobTriggerTypeNode(job_entry, trigger_type);

    // In case the node is written in arya Format, the FEP3_JOB_TRIGGER_TYPE_PROPERTY will be
    // missing
    if ((trigger_result == ERR_NOT_FOUND) ||
        (trigger_type == FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY)) {
        FEP3_RETURN_IF_FAILED(parseCycleTimeNode(job_entry, job_configuration._cycle_sim_time));
        FEP3_RETURN_IF_FAILED(parseDelayTimeNode(job_entry, job_configuration._delay_sim_time));
        FEP3_RETURN_IF_FAILED(
            parseMaxRuntimeNode(job_entry, job_configuration._max_runtime_real_time));
        FEP3_RETURN_IF_FAILED(parseRuntimeViolationStrategyNode(
            job_entry, job_configuration._runtime_violation_strategy));
    }
    else {
        if (trigger_type == FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY) {
            return ERR_INVALID_TYPE;
        }
        else {
            return ERR_INVALID_ARG;
        }
    }

    return {};
}

std::pair<Result, std::unique_ptr<fep3::catelyn::JobConfiguration>> readJobConfigFromPropertyNode(
    const IPropertyNode& job_entry)
{
    std::string trigger_type;
    const auto node_parse_result = parseJobTriggerTypeNode(job_entry, trigger_type);
    if (!node_parse_result) {
        return {node_parse_result, nullptr};
    }

    if (trigger_type == FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY) {
        auto job_configuration =
            std::make_unique<fep3::catelyn::ClockTriggeredJobConfiguration>(0ms);
        auto parse_result = (parseCycleTimeNode(job_entry, job_configuration->_cycle_sim_time));
        parse_result |= parseDelayTimeNode(job_entry, job_configuration->_delay_sim_time);
        parse_result |= parseMaxRuntimeNode(job_entry, job_configuration->_max_runtime_real_time);
        parse_result |= parseRuntimeViolationStrategyNode(
            job_entry, job_configuration->_runtime_violation_strategy);

        return parse_result ? std::make_pair(fep3::Result{}, std::move(job_configuration)) :
                              std::make_pair(parse_result, nullptr);
    }
    else if (trigger_type == FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY) {
        auto job_configuration = std::make_unique<fep3::catelyn::DataTriggeredJobConfiguration>(
            std::vector<std::string>());

        auto parse_result =
            parseMaxRuntimeNode(job_entry, job_configuration->_max_runtime_real_time);
        parse_result |= parseRuntimeViolationStrategyNode(
            job_entry, job_configuration->_runtime_violation_strategy);
        parse_result |= parseJobTriggerSignalNode(job_entry, job_configuration->_signal_names);

        return parse_result ? std::make_pair(fep3::Result{}, std::move(job_configuration)) :
                              std::make_pair(parse_result, nullptr);
    }
    // the check is done already in parseJobTriggerTypeNode
    else {
        return std::make_pair(fep3::Result{ERR_INVALID_ARG}, nullptr);
    }
}

} // namespace

namespace fep3 {
namespace native {

Result readJobConfigurationsFromPropertyNode(const base::NativePropertyNode& jobs_node,
                                             JobConfigurations& job_configurations)
{
    job_configurations = JobConfigurations{};
    const auto job_nodes = jobs_node.getChildren();

    for (const auto& job_entry: job_nodes) {
        arya::JobConfiguration job_configuration{0ms};
        // return ERR_INVALID_TYPE if data triggered job configuration exists
        auto res = readJobConfigFromPropertyNode(*job_entry, job_configuration);

        if (res == ERR_INVALID_TYPE) {
            // Do not fail, only skip the data triggered job
            continue;
        }
        else {
            // Fail if other errors occur.
            FEP3_RETURN_IF_FAILED(res);
            job_configurations.insert({job_entry->getName(), job_configuration});
        }
    }

    return {};
}

Result readJobConfigurationsFromPropertyNode(const base::NativePropertyNode& jobs_node,
                                             JobConfigurationPtrs& job_configurations)
{
    job_configurations = JobConfigurationPtrs();
    const auto job_nodes = jobs_node.getChildren();

    for (const auto& job_entry: job_nodes) {
        auto [parse_result, parsed_config] = readJobConfigFromPropertyNode(*job_entry);
        FEP3_RETURN_IF_FAILED(parse_result);
        job_configurations.emplace(job_entry->getName(), std::move(parsed_config));
    }

    return {};
}

} // namespace native
} // namespace fep3
