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

#include <fep3/components/base/component_iid.h>
#include <fep3/fep3_result_decl.h>
#include <fep3/fep3_timestamp.h>

namespace fep3 {
namespace catelyn {

/**
 * Health service interface which provides getter for current participant and system health state
 * and functionality to set a participant to state error.
 */
class IHealthService {
protected:
    ~IHealthService() = default;

public:
    /// The component interface identifier of IHealthService
    FEP_COMPONENT_IID("health_service.catelyn.fep3.iid")

    /**
     * The result of a job execution.
     */
    struct JobExecuteResult {
        /// Simulation time that the job was executed
        fep3::Timestamp simulation_time;
        /// holds last execution error of @ref fep3::arya::IJob::executeDataIn
        Result result_execute_data_in;
        /// holds last execution error of @ref fep3::arya::IJob::execute
        Result result_execute;
        /// holds last execution error of @ref fep3::arya::IJob::executeDataOut
        Result result_execute_data_out;
    };

    /**
     * @brief Update the health service with a job result.
     *
     * @param[in] job_name job name that returned result @p result
     * @param[in] result the result of the job execution.
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_NOT_FOUND The job name is not found in the job registry
     */
    virtual Result updateJobStatus(const std::string& job_name, const JobExecuteResult& result) = 0;

    /**
     * @brief Reset the health state of this participant to state ok and log the provided message.
     * Resetting the health state to ok indicates all errors have been resolved and the participant
     * works correctly.
     * An error health state should only be reset to health state ok from external using rpc,
     * not from inside the participant.
     *
     * @return Result indicating whether setting the health state succeeded or failed.
     * @retval ERR_NOERROR Health state has been set successfully.
     */
    virtual Result resetHealth() = 0;
};

// using arya::IHealthService;
} // namespace catelyn

using catelyn::IHealthService;

} // namespace fep3
