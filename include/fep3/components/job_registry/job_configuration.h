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


#pragma once

#include <string>
#include <vector>

#include <fep3/fep3_duration.h>
#include <fep3/fep3_optional.h>


namespace fep3
{
namespace arya
{

/**
* @brief Configuration of a job
*
*/
class JobConfiguration
{
public:
    /// Strategy enum configuring behaviour in case of an operational time violation
    enum class TimeViolationStrategy
    {
        /// dummy value
        unknown = 0,
        /// Time violations are ignored
        ignore_runtime_violation,
        /// A warning will be logged when an operational time violation is detected
        warn_about_runtime_violation,
        /// Configured output samples will not be published when an operational time violation is detected
        skip_output_publish
    };

public:
    /**
    * @brief CTOR
    *
    * @param[in] cycle_sim_time The cycle time to be used for the job (simulation time)
    * @param[in] first_delay_sim_time The cycle delay time to the 0 point of the time base (simulation time)
    * @param[in] max_runtime_real_time The maximum duration that a single job execution is expected to need for computation (real time).
    *                                   Provide no value if you have no expectations on the jobs runtime.
    * @param[in] runtime_violation_strategy The violation strategy
    */
    JobConfiguration(arya::Duration cycle_sim_time,
                     arya::Duration first_delay_sim_time = arya::Duration(0),
                     arya::Optional<arya::Duration> max_runtime_real_time = {},
                     TimeViolationStrategy runtime_violation_strategy = TimeViolationStrategy::ignore_runtime_violation)
        : _cycle_sim_time(cycle_sim_time)
        , _delay_sim_time(first_delay_sim_time)
        , _max_runtime_real_time(std::move(max_runtime_real_time))
        , _runtime_violation_strategy(runtime_violation_strategy)
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
    * @return Tiolation strategy matching the provided string.
    */
    static TimeViolationStrategy fromString(const std::string& strategy_string)
    {
        if ("ignore_runtime_violation" == strategy_string)
        {
            return TimeViolationStrategy::ignore_runtime_violation;
        }
        else if ("warn_about_runtime_violation" == strategy_string)
        {
            return TimeViolationStrategy::warn_about_runtime_violation;
        }
        else if ("skip_output_publish" == strategy_string)
        {
            return TimeViolationStrategy::skip_output_publish;
        }
        else
        {
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
        switch (runtime_violation_strategy)
        {
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
        switch (_runtime_violation_strategy)
        {
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
    arya::Duration                         _cycle_sim_time;
    /// The cycle delay time to the 0 point of the time base (simulation time)
    arya::Duration                         _delay_sim_time;
    /// The maximum duration that a single job execution is expected to need for computation (real time)
    arya::Optional<arya::Duration>               _max_runtime_real_time;
    /// The strategy that will be applied in case of a longer computation time than expected
    TimeViolationStrategy             _runtime_violation_strategy;
};

} // namespace arya
using arya::JobConfiguration;
} // namespace fep3
