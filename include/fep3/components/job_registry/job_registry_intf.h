/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/components/base/component_iid.h>
#include <fep3/components/job_registry/job_intf.h>

#include <list>

/**
 * @brief Main property entry of the job registry properties
 */
#define FEP3_JOB_REGISTRY_CONFIG "job_registry"

/**
 * @brief The jobs node name
 * Use this node to address specific job configuration nodes by path from inside the job registry
 * configuration node.
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
 */
#define FEP3_JOB_CYCLE_SIM_TIME_PROPERTY "cycle_sim_time"

/**
 * @brief Job entry delay time configuration property
 * Use this to access the delay time configuration property of a specific job configuration entry.
 */
#define FEP3_JOB_DELAY_SIM_TIME_PROPERTY "delay_sim_time"

/**
 * @brief Job entry max runtime configuration property
 * Use this to access the max runtime configuration property of a specific job configuration entry.
 */
#define FEP3_JOB_MAX_RUNTIME_REAL_TIME_PROPERTY "max_runtime_real_time"

/**
 * @brief Job entry time violation strategy configuration property
 * Use this to access the time violation strategy configuration property of a specific job
 * configuration entry.
 */
#define FEP3_JOB_RUNTIME_VIOLATION_STRATEGY_PROPERTY "runtime_violation_strategy"

/**
 * @brief Job entry trigger type configuration property
 * Use this to access the trigger type configuration property of a specific job configuration entry.
 */
#define FEP3_JOB_TRIGGER_TYPE_PROPERTY "trigger_type"

/**
 * @brief Value of the "trigger_type" for clock triggered job configuration
 */
#define FEP3_JOB_CYCLIC_TRIGGER_TYPE_PROPERTY "cyclic_trigger"

/**
 * @brief Value of the "trigger_type" for data triggered job configuration
 */
#define FEP3_JOB_DATA_TRIGGER_TYPE_PROPERTY "data_trigger"

/**
 * @brief Job entry signal name for data triggered job
 * Use this to access the signal name configuration property of a specific data triggered job
 * configuration entry.
 */
#define FEP3_JOB_TRIGGER_SIGNAL_PROPERTY "trigger_signals"

namespace fep3 {
namespace arya {

/**
 * @brief Interface of the job registry
 *
 * The job registry may be used to register jobs.
 * Registered jobs may be triggered by the active scheduler.
 */
class IJobRegistry {
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
    virtual fep3::Result addJob(const std::string& name,
                                const std::shared_ptr<arya::IJob>& job,
                                const arya::JobConfiguration& job_config) = 0;

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
     * @brief Return the job info of all registered clock triggered jobs.
     * @return List containing job info of all registered clock triggered jobs
     * @note Returns only job info of clock triggered jobs, to get both
     * clock and data triggered job infos use @ref fep3::catelyn::IJobRegistry::getJobInfosCatelyn
     */
    virtual std::list<arya::JobInfo> getJobInfos() const = 0;

    /**
     * @brief Get all registered jobs.
     *
     * @return All registered clock triggered Jobs
     * @note Returns only clock triggered jobs. To get both
     * clock and data triggered jobs use @ref fep3::catelyn::IJobRegistry::getJobsCatelyn
     */
    virtual arya::Jobs getJobs() const = 0;
};

} // namespace arya

namespace catelyn {

/**
 * @brief Interface of the job registry
 *
 * The job registry may be used to register jobs.
 * Registered jobs may be triggered by the active scheduler.
 */
class IJobRegistry : virtual public arya::IJobRegistry {
public:
    /// The component interface identifier of IJobRegistry
    FEP_COMPONENT_IID("job_registry.catelyn.fep3.iid");

protected:
    /// DTOR
    ~IJobRegistry() = default;

public:
    using arya::IJobRegistry::addJob;

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
    virtual fep3::Result addJob(const std::string& name,
                                const std::shared_ptr<arya::IJob>& job,
                                const catelyn::JobConfiguration& job_config) = 0;

    /**
     * @brief Return the job info of all registered jobs.
     * @return List containing job info of all registered jobs
     * @note Returns clock and data triggered jobs info
     * whereas @ref fep3::arya::IJobRegistry::getJobInfos returns
     * only clock triggered job infos.
     */
    virtual std::list<fep3::catelyn::JobInfo> getJobInfosCatelyn() const = 0;

    /**
     * @brief Get all registered jobs.
     *
     * @return All registered Jobs
     * @note Returns clock and data triggered jobs,
     * whereas @ref fep3::arya::IJobRegistry::getJobs returns
     * only clock triggered jobs.
     */
    virtual fep3::catelyn::Jobs getJobsCatelyn() const = 0;
};
} // namespace catelyn

using catelyn::IJobRegistry;
} // namespace fep3
