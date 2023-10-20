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

#include <fep3/components/job_registry/job_registry_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
// for "argument dependent lookup" in gtest macros the comparison operator has to be in the
// same namespace as the type of the arguments to be compared
namespace arya {

inline bool operator==(const JobConfiguration& lhs, const JobConfiguration& rhs)
{
    return (lhs._cycle_sim_time == rhs._cycle_sim_time &&
            lhs._delay_sim_time == rhs._delay_sim_time &&
            lhs._max_runtime_real_time == rhs._max_runtime_real_time &&
            lhs._runtime_violation_strategy == rhs._runtime_violation_strategy);
}

inline bool operator==(const arya::JobInfo& lhs, const arya::JobInfo& rhs)
{
    return (lhs.getName() == rhs.getName() && lhs.getConfig() == rhs.getConfig());
}

} // namespace arya

namespace catelyn {

inline bool operator==(const catelyn::JobConfiguration& lhs, const catelyn::JobConfiguration& rhs)
{
    const catelyn::ClockTriggeredJobConfiguration* lhsp1 =
        dynamic_cast<const catelyn::ClockTriggeredJobConfiguration*>(&lhs);

    if (lhsp1 != nullptr) {
        const catelyn::ClockTriggeredJobConfiguration* rhsp1 =
            dynamic_cast<const catelyn::ClockTriggeredJobConfiguration*>(&rhs);
        if (rhsp1 != nullptr) {
            return (lhsp1->_cycle_sim_time == rhsp1->_cycle_sim_time &&
                    lhsp1->_delay_sim_time == rhsp1->_delay_sim_time &&
                    lhsp1->_max_runtime_real_time == rhsp1->_max_runtime_real_time &&
                    lhsp1->_runtime_violation_strategy == rhsp1->_runtime_violation_strategy);
        }
        else {
            return false;
        }
    }

    const catelyn::DataTriggeredJobConfiguration* lhsp2 =
        dynamic_cast<const catelyn::DataTriggeredJobConfiguration*>(&lhs);

    if (lhsp2 != nullptr) {
        const catelyn::DataTriggeredJobConfiguration* rhsp2 =
            dynamic_cast<const catelyn::DataTriggeredJobConfiguration*>(&rhs);
        if (rhsp2 != nullptr) {
            return (lhsp2->_signal_names == rhsp2->_signal_names &&
                    lhsp2->_max_runtime_real_time == rhsp2->_max_runtime_real_time &&
                    lhsp2->_runtime_violation_strategy == rhsp2->_runtime_violation_strategy);
        }

        else {
            return false;
        }
    }
    return false;
}

inline bool operator==(const catelyn::JobInfo& lhs, const catelyn::JobInfo& rhs)
{
    return (lhs.getName() == rhs.getName() && lhs.getConfig() == rhs.getConfig());
}
} // namespace catelyn

namespace mock {
namespace arya {

struct Job : public fep3::arya::IJob {
    ~Job() = default;
    MOCK_METHOD(fep3::Result, executeDataIn, (Timestamp), (override));
    MOCK_METHOD(fep3::Result, execute, (Timestamp), (override));
    MOCK_METHOD(fep3::Result, executeDataOut, (Timestamp), (override));
};

/**
 * @brief Matcher for class fep3::arya::Jobs
 */
MATCHER_P(JobsMatcher, other, "Equality matcher for fep3::arya::Jobs")
{
    if (arg.size() != other.size()) {
        return false;
    }
    else {
        auto arg_iter = arg.begin();
        auto other_iter = other.begin();
        for (; arg_iter != arg.end() && other_iter != other.end(); ++arg_iter, ++other_iter) {
            const fep3::arya::JobEntry& arg_job_entry = arg_iter->second;
            const fep3::arya::JobEntry& other_job_entry = other_iter->second;
            const fep3::arya::JobInfo& arg_job_info = arg_job_entry.job_info;
            const fep3::arya::JobInfo& other_job_info = other_job_entry.job_info;
            if (arg_iter->first != other_iter->first
                // note: the pointers to the job are actually differing, because one side is the
                // pointer to the wrapped job
                //|| arg_job_entry.job != other_job_entry.job
                || !(arg_job_info == other_job_info)) {
                break;
            }
        }
        return (arg_iter == arg.end());
    }
}

/**
 * @brief Matcher for class fep3::arya::Jobs
 */
MATCHER_P(JobConfigurationMatcher, other, "Equality matcher for fep3::arya::JobConfiguration")
{
    return arg == other;
}

} // namespace arya

namespace catelyn {

/**
 * @brief Matcher for class fep3::arya::Jobs
 */
MATCHER_P(JobsMatcher, other, "Equality matcher for fep3::catelyn::Jobs")
{
    if (arg.size() != other.size()) {
        return false;
    }
    else {
        auto arg_iter = arg.begin();
        auto other_iter = other.begin();
        for (; arg_iter != arg.end() && other_iter != other.end(); ++arg_iter, ++other_iter) {
            const fep3::JobEntry& arg_job_entry = arg_iter->second;
            const fep3::JobEntry& other_job_entry = other_iter->second;
            const fep3::JobInfo& arg_job_info = arg_job_entry.job_info;
            const fep3::JobInfo& other_job_info = other_job_entry.job_info;
            if (arg_iter->first != other_iter->first
                // note: the pointers to the job are actually differing, because one side is the
                // pointer to the wrapped job
                //|| arg_job_entry.job != other_job_entry.job
                || !(arg_job_info == other_job_info)) {
                break;
            }
        }
        return (arg_iter == arg.end());
    }
}

/**
 * @brief Matcher for class fep3::catelyn::Jobs
 */
MATCHER_P(JobConfigurationMatcher, other, "Equality matcher for fep3::catelyn::JobConfiguration")
{
    return arg == other;
}

} // namespace catelyn

using arya::Job;
using catelyn::JobConfigurationMatcher;
using catelyn::JobsMatcher;
} // namespace mock
} // namespace fep3

///@endcond nodoc
