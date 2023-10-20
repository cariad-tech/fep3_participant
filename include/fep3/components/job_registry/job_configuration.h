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

#include <fep3/fep3_duration.h>
#include <fep3/fep3_optional.h>
#include <fep3/fep3_result_decl.h>

#include <memory>
#include <vector>

namespace fep3 {
namespace arya {

/**
 * @brief Configuration of a job
 */
class JobConfiguration {
public:
    /// Strategy enum configuring behavior in case of an operational time violation
    enum class TimeViolationStrategy
    {
        /// dummy value
        unknown = 0,
        /// Time violations are ignored
        ignore_runtime_violation,
        /// A warning will be logged when an operational time violation is detected
        warn_about_runtime_violation,
        /// Configured output samples will not be published when an operational time violation is
        /// detected
        skip_output_publish
    };

public:
    /**
     * @brief CTOR
     *
     * @param[in] cycle_sim_time The cycle time to be used for the job (simulation time)
     * @param[in] first_delay_sim_time The cycle delay time to the 0 point of the time base
     * (simulation time)
     * @param[in] max_runtime_real_time The maximum duration that a single job execution is expected
     * to need for computation (real time). Provide no value if you have no expectations on the
     * job's runtime.
     * @param[in] runtime_violation_strategy The violation strategy
     */
    JobConfiguration(arya::Duration cycle_sim_time,
                     arya::Duration first_delay_sim_time = arya::Duration(0),
                     arya::Optional<arya::Duration> max_runtime_real_time = {},
                     TimeViolationStrategy runtime_violation_strategy =
                         TimeViolationStrategy::ignore_runtime_violation)
        : _cycle_sim_time(cycle_sim_time),
          _delay_sim_time(first_delay_sim_time),
          _max_runtime_real_time(std::move(max_runtime_real_time)),
          _runtime_violation_strategy(runtime_violation_strategy)
    {
    }

    /**
     * @brief Return a time violation strategy for a given string.
     *
     * The string parameter must match one of the time violation strategy names.
     * In case of no match, the unknown strategy is returned.
     *
     * @param[in] strategy_string The string to derive a time violation strategy from.
     *
     * @return TimeViolationStrategy strategy matching the provided string.
     */
    static TimeViolationStrategy fromString(const std::string& strategy_string)
    {
        if ("ignore_runtime_violation" == strategy_string) {
            return TimeViolationStrategy::ignore_runtime_violation;
        }
        else if ("warn_about_runtime_violation" == strategy_string) {
            return TimeViolationStrategy::warn_about_runtime_violation;
        }
        else if ("skip_output_publish" == strategy_string) {
            return TimeViolationStrategy::skip_output_publish;
        }
        else {
            return TimeViolationStrategy::unknown;
        }
    }

    /**
     * @brief Return a time violation strategy as string for a given strategy.
     *
     * @param[in] runtime_violation_strategy The strategy to derive a string from.
     *
     * @return The configured time violation strategy as std::string.
     */
    static std::string toString(const TimeViolationStrategy& runtime_violation_strategy)
    {
        switch (runtime_violation_strategy) {
        case TimeViolationStrategy::ignore_runtime_violation:
            return "ignore_runtime_violation";
        case TimeViolationStrategy::warn_about_runtime_violation:
            return "warn_about_runtime_violation";
        case TimeViolationStrategy::skip_output_publish:
            return "skip_output_publish";
        default:
            return "unknown";
        }
    }

public:
    /**
     * @brief Return the configured time violation strategy as string.
     *
     * @return The configured time violation strategy as std::string.
     */
    std::string toString() const
    {
        switch (_runtime_violation_strategy) {
        case TimeViolationStrategy::ignore_runtime_violation:
            return "ignore_runtime_violation";
        case TimeViolationStrategy::warn_about_runtime_violation:
            return "warn_about_runtime_violation";
        case TimeViolationStrategy::skip_output_publish:
            return "skip_output_publish";
        default:
            return "unknown";
        }
    }

public:
    /// The cycle time to be used for the job (simulation time)
    arya::Duration _cycle_sim_time;
    /// The cycle delay time to the 0 point of the time base (simulation time)
    arya::Duration _delay_sim_time;
    /// The maximum duration that a single job execution is expected to need for computation (real
    /// time)
    arya::Optional<arya::Duration> _max_runtime_real_time;
    /// The strategy that will be applied in case of a longer computation time than expected
    TimeViolationStrategy _runtime_violation_strategy;
};

} // namespace arya

namespace catelyn {
class ClockTriggeredJobConfiguration;
class DataTriggeredJobConfiguration;

/**
 * @brief IJobConfigurationVisitor of a job.
 * Visitor interface to implement the visitor pattern of job configuration.
 * Currently it supports two types job configuration, clock triggered and data triggered job
 * configuraiton.
 */
class IJobConfigurationVisitor {
public:
    /**
     * DTOR
     */
    virtual ~IJobConfigurationVisitor() = default;

    /**
     * @brief Accept a clocked triggered job configuration
     *
     * @param[in] configuration The clock triggered job configuration
     * @return fep3::Result
     * @retval NO_ERROR if succeeded
     */
    virtual fep3::Result visitClockTriggeredConfiguration(
        const catelyn::ClockTriggeredJobConfiguration& configuration) = 0;

    /**
     * @brief Accept a data triggered job configuration
     *
     * @param[in] configuration The data triggered job configuration
     * @return fep3::Result
     * @retval NO_ERROR if succeeded
     */
    virtual fep3::Result visitDataTriggeredConfiguration(
        const catelyn::DataTriggeredJobConfiguration& configuration) = 0;
};

/**
 * @brief IConfiguration of a job, the base class of configurations allow visitor pattern.
 */
class JobConfiguration {
public:
    /// The strategy that will be applied in case of a longer computation time than expected
    using TimeViolationStrategy = fep3::arya::JobConfiguration::TimeViolationStrategy;

    /**
     * @brief CTOR
     * @param[in] max_runtime_real_time The maximum duration that a single job execution is expected
     * to need for computation (real time). Provide no value if you have no expectations on the
     * job's runtime.
     * @param[in] runtime_violation_strategy The violation strategy
     */
    JobConfiguration(arya::Optional<arya::Duration> max_runtime_real_time = {},
                     TimeViolationStrategy runtime_violation_strategy =
                         TimeViolationStrategy::ignore_runtime_violation)
        : _max_runtime_real_time(max_runtime_real_time),
          _runtime_violation_strategy(runtime_violation_strategy)
    {
    }

    virtual ~JobConfiguration() = default;
    /**
     * @brief Accept the visitor
     *
     * @param[in] visitor The job configuration visitor
     * @return fep3::Result
     * @retval NO_ERROR if succeeded
     */
    virtual fep3::Result acceptVisitor(catelyn::IJobConfigurationVisitor& visitor) const = 0;

    /**
     * @brief CTOR
     *
     * @return @ref JobConfiguration a clone of JobConfiguration.
     */
    virtual std::unique_ptr<catelyn::JobConfiguration> clone() const = 0;

    /**
     * @brief Return a time violation strategy for a given string.
     *
     * The string parameter must match one of the time violation strategy names.
     * In case of no match, the unknown strategy is returned.
     *
     * @param[in] strategy_string The string to derive a time violation strategy from.
     *
     * @return TimeViolationStrategy strategy matching the provided string.
     */
    static TimeViolationStrategy fromString(const std::string& strategy_string)
    {
        return fep3::arya::JobConfiguration::fromString(strategy_string);
    }

    /**
     * @brief Return a time violation strategy as string for a given strategy.
     *
     * @param[in] runtime_violation_strategy The strategy to derive a string from.
     *
     * @return The configured time violation strategy as std::string.
     */
    static std::string toString(const TimeViolationStrategy& runtime_violation_strategy)
    {
        return fep3::arya::JobConfiguration::toString(runtime_violation_strategy);
    }

    /**
     * @brief Return the configured time violation strategy as string.
     *
     * @return The configured time violation strategy as std::string.
     */
    std::string toString() const
    {
        return toString(_runtime_violation_strategy);
    }

    /// The maximum duration that a single job execution is expected to need for computation (real
    /// time)
    arya::Optional<arya::Duration> _max_runtime_real_time;
    /// The strategy that will be applied in case of a longer computation time than expected
    TimeViolationStrategy _runtime_violation_strategy;
};

/**
 * @brief Data triggered job configuration class.
 */
class DataTriggeredJobConfiguration : public catelyn::JobConfiguration {
public:
    /**
     * @brief CTOR
     * Data triggered job configuration class.
     *
     * @param[in] signal_names The signal names to trigger the job
     * @param[in] max_runtime_real_time The maximum duration that a single job execution is expected
     * to need for computation (real time). Provide no value if you have no expectations on the
     * job's runtime.
     * @param[in] runtime_violation_strategy The violation strategy
     */
    explicit DataTriggeredJobConfiguration(
        const std::vector<std::string>& signal_names,
        arya::Optional<arya::Duration> max_runtime_real_time = {},
        TimeViolationStrategy runtime_violation_strategy =
            TimeViolationStrategy::ignore_runtime_violation)
        : _signal_names(signal_names),
          catelyn::JobConfiguration(max_runtime_real_time, runtime_violation_strategy)
    {
    }

    /**
     * @brief Accept a job configuration visitor
     *
     * @param[in] visitor The job configuration visitor for data triggered configuration
     * @return fep3::Result
     * @retval NO_ERROR if succeeded
     */
    fep3::Result acceptVisitor(catelyn::IJobConfigurationVisitor& visitor) const override
    {
        return visitor.visitDataTriggeredConfiguration(*this);
    }

    /**
     * @brief Provide a clone of itself as a unique pointer
     *
     * @return @ref JobConfiguration a clone of JobConfiguration
     */
    std::unique_ptr<catelyn::JobConfiguration> clone() const override
    {
        return std::make_unique<catelyn::DataTriggeredJobConfiguration>(*this);
    }

    /// The signal name to trigger the job
    std::vector<std::string> _signal_names;
};

/**
 * @brief Clock triggered job configuration class
 */
class ClockTriggeredJobConfiguration : public catelyn::JobConfiguration {
public:
    /**
     * @brief CTOR
     *
     * @param[in] cycle_sim_time The cycle time to be used for the job (simulation time)
     * @param[in] first_delay_sim_time The cycle delay time to the 0 point of the time base
     * (simulation time)
     * @param[in] max_runtime_real_time The maximum duration that a single job execution is expected
     *                                  to need for computation (real time).
     *                                  Provide no value if you have no expectations on the job's
     * runtime.
     * @param[in] runtime_violation_strategy The violation strategy
     */
    explicit ClockTriggeredJobConfiguration(
        arya::Duration cycle_sim_time,
        arya::Duration first_delay_sim_time = arya::Duration(0),
        arya::Optional<arya::Duration> max_runtime_real_time = {},
        TimeViolationStrategy runtime_violation_strategy =
            TimeViolationStrategy::ignore_runtime_violation)
        : _cycle_sim_time(cycle_sim_time),
          _delay_sim_time(first_delay_sim_time),
          catelyn::JobConfiguration(max_runtime_real_time, runtime_violation_strategy)
    {
    }

    /**
     * @brief CTOR
     *
     * @param[in] job_config The basic job configuration
     */
    ClockTriggeredJobConfiguration(const fep3::arya::JobConfiguration& job_config)
        : _cycle_sim_time(job_config._cycle_sim_time),
          _delay_sim_time(job_config._delay_sim_time),
          catelyn::JobConfiguration(job_config._max_runtime_real_time,
                                    job_config._runtime_violation_strategy)
    {
    }

    /**
     * @brief Accept a job configuration visitor
     *
     * @param[in] visitor The job configuration visitor for clock triggered configuration
     * @return fep3::Result
     * @retval NO_ERROR if succeeded
     */
    fep3::Result acceptVisitor(catelyn::IJobConfigurationVisitor& visitor) const override
    {
        return visitor.visitClockTriggeredConfiguration(*this);
    }

    /**
     * @brief Provide a clone of itself as a unique pointer
     *
     * @return @ref JobConfiguration a clone of JobConfiguration
     */
    std::unique_ptr<catelyn::JobConfiguration> clone() const override
    {
        return std::make_unique<catelyn::ClockTriggeredJobConfiguration>(*this);
    }

    /// The cycle time to be used for the job (simulation time)
    arya::Duration _cycle_sim_time;
    /// The cycle delay time to the 0 point of the time base (simulation time)
    arya::Duration _delay_sim_time;
};

} // namespace catelyn

using catelyn::ClockTriggeredJobConfiguration;
using catelyn::DataTriggeredJobConfiguration;
using catelyn::IJobConfigurationVisitor;
using catelyn::JobConfiguration;
} // namespace fep3
