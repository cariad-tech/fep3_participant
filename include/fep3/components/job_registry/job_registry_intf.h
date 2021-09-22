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

#include <list>

#include "job_intf.h"
#include "job_info.h"
#include <fep3/components/base/component_iid.h>

/**
* @brief Main property entry of the job registry properties
*/
#define FEP3_JOB_REGISTRY_CONFIG "job_registry"
/**
* @brief The jobs node name
* Use this node to address specific job configuration nodes by path from inside the job registry configuration node.
*
*/
#define FEP3_JOBS_PROPERTY "jobs"
/**
* @brief The jobs node
* Use this to address specific job configuration nodes by path.
*/
#define FEP3_JOB_REGISTRY_JOBS FEP3_JOB_REGISTRY_CONFIG "/" FEP3_JOBS_PROPERTY
/**
* @brief Job entry cycle time configuration property
* Use this to access the cycle time configuration property of a specific job configuration entry.
*
*/
#define FEP3_JOB_CYCLE_SIM_TIME_PROPERTY "cycle_sim_time"
/**
* @brief Job entry delay time configuration property
* Use this to access the delay time configuration property of a specific job configuration entry.
*
*/
#define FEP3_JOB_DELAY_SIM_TIME_PROPERTY "delay_sim_time"
/**
* @brief Job entry max runtime configuration property
* Use this to access the max runtime configuration property of a specific job configuration entry.
*
*/
#define FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY "max_runtime_real_time"
/**
* @brief Job entry time violation strategy configuration property
* Use this to access the time violation strategy configuration property of a specific job configuration entry.
*
*/
#define FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY "runtime_violation_strategy"

namespace fep3
{
namespace arya
{

/**
* @brief Interface of the job registry
*
* The job registry may be used to register jobs.
* Registered jobs may be triggered by the active scheduler.
*/
class IJobRegistry
{
public:
    /// The component interface identifier of IJobRegistry
    FEP_COMPONENT_IID("job_registry.arya.fep3.iid");

protected:
    /// DTOR
    ~IJobRegistry() = default;

public:
    /**
    * @brief Register the given @p job at the job registry.
    *
    * @param[in] name The name of the job to be registered
    * @param[in] job The job to be registered
    * @param[in] job_config The job configuration of the job to be registered
    *
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
    */
    virtual fep3::Result addJob(const std::string& name, const std::shared_ptr<arya::IJob>& job, const arya::JobConfiguration& job_config) = 0;

    /**
    * @brief Unregister the job with the given @p name from the job registry.
    *
    * @param[in] name The name of the job to be removed
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    * @retval ERR_NOT_FOUND A job with the given @p name is not registered
    */
    virtual fep3::Result removeJob(const std::string& name) = 0;

    /**
    * Return the job infos of all registered jobs.
    * @return List containing job infos of all registered jobs
    */
    virtual std::list<arya::JobInfo> getJobInfos() const = 0;

    /**
     * @brief Get all registered jobs.
     *
     * @return All registered Jobs
     */
    virtual arya::Jobs getJobs() const = 0;
};

} // namespace arya
using arya::IJobRegistry;
} // namespace fep3
