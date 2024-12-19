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

#include <fep3/components/base/components_intf.h>
#include <fep3/components/job_registry/job_registry_intf.h>

#include <functional>

namespace fep3 {
namespace core {

/**
 * @brief Job class implementing @ref fep3::arya::IJob
 */
class Job : public fep3::arya::IJob {
public:
    /// ExecuteCallback typedef
    typedef std::function<Result(fep3::arya::Timestamp)> ExecuteCallback;

    /**
     * @brief CTOR
     *
     * @param[in] name Name of the Job
     * @param[in] fc Function
     * @param[in] config Configuration of the job
     */
    Job(std::string name,
        std::unique_ptr<fep3::catelyn::JobConfiguration> config,
        ExecuteCallback fc)
        : _job_info(std::move(name), std::move(config)), _execution_cb(fc)
    {
    }

    /**
     * @brief CTOR
     *
     * @param[in] name Name of the Job
     * @param[in] config Configuration of the job
     */
    Job(std::string name, std::unique_ptr<fep3::catelyn::JobConfiguration> config)
        : Job(std::move(name), std::move(config), [](fep3::arya::Timestamp) -> Result {
              return Result();
          })
    {
    }

    /**
     * @brief CTOR
     *
     * @param[in] name Name of the Job
     * @param[in] cycle_time Cycle time of the job (simulation time)
     */
    Job(std::string name, fep3::arya::Duration cycle_time)
        : Job(name, cycle_time, [](fep3::arya::Timestamp) -> Result { return Result(); })
    {
    }

    /**
     * @brief CTOR
     *
     * @param[in] name Name of the Job
     * @param[in] cycle_time Cycle time of the job (simulation time)
     * @param[in] fc Function
     */
    Job(std::string name, fep3::arya::Duration cycle_time, ExecuteCallback fc)
        : Job(std::move(name),
              std::make_unique<fep3::catelyn::ClockTriggeredJobConfiguration>(cycle_time),
              fc)
    {
    }

    /**
     * @brief CTOR for backwards compatiblity
     *
     * @param[in] name Name of the job
     * @param[in] config Configuration of the job
     */
    Job(std::string name, fep3::arya::JobConfiguration config)
        : Job(std::move(name), fep3::catelyn::ClockTriggeredJobConfiguration(config))
    {
    }

    /**
     * @brief CTOR for backwards compatiblity
     *
     * @param[in] name Name of the job
     * @param[in] fc Function
     * @param[in] config Configuration of the job
     */
    Job(std::string name, fep3::arya::JobConfiguration config, ExecuteCallback fc)
        : Job(std::move(name), fep3::catelyn::ClockTriggeredJobConfiguration(config), fc)
    {
    }

    /**
     * @brief CTOR
     *
     * @tparam T check config type
     * @param[in] name Name of the job
     * @param[in] fc Function
     * @param[in] config Configuration of the job
     */
    template <
        typename T,
        typename = std::enable_if_t<std::is_base_of<fep3::catelyn::JobConfiguration, T>::value>>
    Job(std::string name, T config, ExecuteCallback fc)
        : Job(std::move(name), std::make_unique<T>(std::move(config)), fc)
    {
    }

    /**
     * @brief CTOR
     *
     * @tparam T check job configuration type
     * @param[in] name Name of the job
     * @param[in] config Configuration of the job
     */
    template <
        typename T,
        typename = std::enable_if_t<std::is_base_of<fep3::catelyn::JobConfiguration, T>::value>>
    Job(std::string name, T config) : Job(std::move(name), std::make_unique<T>(std::move(config)))
    {
    }

protected:
    /**
     * @brief Reads input samples.
     *
     * Typically here the samples are read using DataReaders.
     *
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_UNEXPECTED An unexpected error occurred
     */

    fep3::Result executeDataIn(fep3::arya::Timestamp /*time_of_execution*/) override
    {
        return {};
    }

    /**
     * @brief Actual processing of the job.
     *
     * Typically here the data of input samples is processed and output data is created.
     * The execute method can either be overridden or a callback of type @ref
     * fep3::core::Job::ExecuteCallback can be passed using one of the constructors.
     *
     * @param[in] time_of_execution The current simulation time
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_UNEXPECTED An unexpected error occurred
     */
    fep3::Result execute(fep3::arya::Timestamp time_of_execution) override
    {
        return _execution_cb(time_of_execution);
    }

    /**
     * @brief Writes output samples.
     *
     * Typically here the samples are published using DataWriter's.
     *
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_UNEXPECTED An unexpected error occurred
     */
    fep3::Result executeDataOut(fep3::arya::Timestamp /*time_of_execution*/) override
    {
        return {};
    }

public:
    /**
     * @brief Gets the @ref fep3::arya::JobInfo for the job.
     *
     * @return Job info for job
     */
    const fep3::catelyn::JobInfo& getJobInfo() const
    {
        return _job_info;
    }

    /**
     * @brief Reconfigures the job using the @p configuration.
     *
     * @tparam T check job configuration type
     * @param[in] configuration Configuration the job should be reconfigured with
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     */
    template <
        typename T,
        typename = std::enable_if_t<std::is_base_of<fep3::catelyn::JobConfiguration, T>::value>>
    fep3::Result reconfigure(const T& configuration)
    {
        return _job_info.reconfigure(std::make_unique<T>(configuration));
    }

    /**
     * @brief Reconfigures the job using the @p configuration.
     *
     * @param[in] configuration Configuration the job should be reconfigured with
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     */
    fep3::Result reconfigure(std::unique_ptr<fep3::catelyn::JobConfiguration> configuration)
    {
        return _job_info.reconfigure(std::move(configuration));
    }

    /**
     * @brief Resets the job.

     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     */
    virtual fep3::Result reset()
    {
        return {};
    }

private:
    fep3::catelyn::JobInfo _job_info;
    ExecuteCallback _execution_cb;
};

/**
 * @brief Jobs will be added to the @p job_registry.
 * If one job cannot be added, the function returns not adding the following ones.
 *
 * @param[in] jobs List of jobs to be added
 * @param[in] job_registry The job registry to add to
 * @return fep3::Result
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
 */
inline fep3::Result addJobsToJobRegistry(const std::vector<std::shared_ptr<Job>>& jobs,
                                         fep3::catelyn::IJobRegistry& job_registry)
{
    for (const auto& job: jobs) {
        FEP3_RETURN_IF_FAILED(
            job_registry.addJob(job->getJobInfo().getName(), job, job->getJobInfo().getConfig()));
    }

    return {};
}

/**
 * @brief Jobs will be added to the job registry of the @p component.
 * If one job cannot be added, the function returns not adding the following ones.
 *
 * @param[in] jobs List of jobs to be added
 * @param[in] components The component container in which the job registry will be looked up
 * @return fep3::Result
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
inline fep3::Result addToComponents(const std::vector<std::shared_ptr<Job>>& jobs,
                                    const fep3::arya::IComponents& components)
{
    // do not lock here... this is task of the job registry
    auto job_registry = fep3::getComponent<fep3::catelyn::IJobRegistry>(components);
    if (!job_registry) {
        RETURN_ERROR_DESCRIPTION(ERR_NO_INTERFACE,
                                 a_util::strings::format("could not find '%s' in components",
                                                         fep3::arya::IJobRegistry::FEP3_COMP_IID)
                                     .c_str());
    }

    return core::addJobsToJobRegistry(jobs, *job_registry);
}

/**
 * @brief Jobs will be removed from the @p job_registry.
 * If one job cannot be removed, the function will still try to remove the following ones within @p
 * job_names.
 *
 * @param[in] job_names List of jobs to be removed
 * @param[in] job_registry The job registry to remove from
 * @return fep3::Result.
 *           If it's an actual error, it is the error code for the job that failed last.
 *           If more than one remove fails, the error description contains a list of error
 * descriptions separated by ';'.
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_NOT_FOUND A job with the given name is not registered
 */
inline fep3::Result removeJobsFromJobRegistry(const std::vector<std::string>& job_names,
                                              fep3::catelyn::IJobRegistry& job_registry)
{
    auto result = fep3::Result{};
    std::string message = "";
    for (const auto& job_name: job_names) {
        const auto current_result = job_registry.removeJob(job_name);
        result |= current_result;

        if (!current_result) {
            message = message + "; " + current_result.getDescription();
        }
    }
    if (!result) {
        return CREATE_ERROR_DESCRIPTION(
            result,
            message.size() < 2 ? message.c_str() :
                                 std::string(message.begin() + 2, message.end()).c_str());
    }

    return result;
}

/**
 * @brief Jobs will be removed from the job registry of the @p component.
 * If one job cannot be removed, the function will still try to remove the following ones within @p
 * job_names.
 *
 * @param[in] job_names List of jobs to be removed
 * @param[in] components The component container in which the job registry will be looked up
 * @return fep3::Result.
 *           If it's an actual error, it is the error code for the job that failed last.
 *           If more than one remove fails, the error description contains a list of error
 * descriptions separated by ';'.
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_NOT_FOUND A job with the given name is not registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
inline fep3::Result removeFromComponents(const std::vector<std::string>& job_names,
                                         const fep3::arya::IComponents& components)
{
    // do not lock here... this is task of the job registry
    auto job_registry = components.getComponent<fep3::catelyn::IJobRegistry>();
    if (!job_registry) {
        RETURN_ERROR_DESCRIPTION(ERR_NO_INTERFACE,
                                 a_util::strings::format("could not find '%s' in components",
                                                         fep3::arya::IJobRegistry::FEP3_COMP_IID)
                                     .c_str());
    }

    return core::removeJobsFromJobRegistry(job_names, *job_registry);
}

/**
 * @brief The job with the given name will be removed from the job registry of the @p components.
 * If one job cannot be removed, the function will still try to remove the following ones within @p
 * job_names.
 *
 * @param[in] job_name job to be removed
 * @param[in] components The component container in which the job registry will be looked up
 * @return fep3::Result.
 *           If it's an actual error, it is the error code for the job that failed last.
 *           If more than one remove fails, the error description contains a list of error
 * descriptions separated by ';'.
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_NOT_FOUND A job with the given name is not registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
inline fep3::Result removeFromComponents(const std::string& job_name,
                                         const fep3::arya::IComponents& components)
{
    const std::vector<std::string> jobs = {job_name};
    return core::removeFromComponents(jobs, components);
}

/**
 * @brief the job Jobs will be added to the job registry of the @p component.
 *
 * @param[in] job_name the job name
 * @param[in] job shared pointer to a jobpointer
 * @param[in] job_config configuration of the job
 * @param[in] components The component container in which the job registry will be looked up
 * @return fep3::Result
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
inline fep3::Result addToComponents(const std::string& job_name,
                                    const std::shared_ptr<fep3::arya::IJob>& job,
                                    const fep3::arya::JobConfiguration& job_config,
                                    const fep3::arya::IComponents& components)
{
    // do not lock here... this is task of the job registry
    auto job_registry = fep3::getComponent<fep3::catelyn::IJobRegistry>(components);
    if (!job_registry) {
        RETURN_ERROR_DESCRIPTION(ERR_NO_INTERFACE,
                                 a_util::strings::format("could not find '%s' in components",
                                                         fep3::arya::IJobRegistry::FEP3_COMP_IID)
                                     .c_str());
    }

    return job_registry->addJob(job_name, job, job_config);
}

/**
 * @brief the job Jobs will be added to the job registry of the @p component.
 *
 * @param[in] job_name the job name
 * @param[in] job shared pointer to a jobpointer
 * @param[in] job_config configuration of the job
 * @param[in] components The component container in which the job registry will be looked up
 * @return fep3::Result
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
template <typename JobConfiguration>
inline std::enable_if_t<std::is_base_of_v<fep3::catelyn::JobConfiguration, JobConfiguration>,
                        fep3::Result>
addToComponents(const std::string& job_name,
                const std::shared_ptr<fep3::arya::IJob>& job,
                const JobConfiguration& job_config,
                const fep3::arya::IComponents& components)
{
    // do not lock here... this is task of the job registry
    auto job_registry = fep3::getComponent<fep3::catelyn::IJobRegistry>(components);
    if (!job_registry) {
        RETURN_ERROR_DESCRIPTION(ERR_NO_INTERFACE,
                                 a_util::strings::format("could not find '%s' in components",
                                                         fep3::arya::IJobRegistry::FEP3_COMP_IID)
                                     .c_str());
    }

    return job_registry->addJob(job_name, job, job_config);
}

} // namespace core
} // namespace fep3
