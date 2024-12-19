/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "job_registry_impl.h"

#include "job_configurations.h"
#include "local_job_registry.h"

namespace fep3 {
namespace native {

JobRegistryImpl::JobRegistryImpl(JobRegistryConfiguration& job_registry_configuration)
    : _job_registry_configuration(job_registry_configuration)
{
}

fep3::Result JobRegistryImpl::addJob(const std::string& name,
                                     const std::shared_ptr<fep3::IJob>& job,
                                     const arya::JobConfiguration& job_config)
{
    return addJob(name, job, fep3::catelyn::ClockTriggeredJobConfiguration(job_config));
}

fep3::Result JobRegistryImpl::addJob(const std::string& name,
                                     const std::shared_ptr<arya::IJob>& job,
                                     const catelyn::JobConfiguration& job_config)
{
    if (_jobs.count(name) > 0) {
        RETURN_ERROR_DESCRIPTION(
            ERR_RESOURCE_IN_USE,
            a_util::strings::format(
                "Adding job to job registry failed. A job with the name '%s' already exists.",
                name.c_str())
                .c_str());
    }
    _jobs.emplace(std::make_pair(name, job));

    FEP3_RETURN_IF_FAILED(_job_registry_configuration.addJobEntry(name, job_config));

    return {};
}

fep3::Result JobRegistryImpl::removeJob(const std::string& name)
{
    if (_jobs.count(name) == 0) {
        RETURN_ERROR_DESCRIPTION(
            ERR_NOT_FOUND,
            a_util::strings::format(
                "Removing job from job registry failed. A job with the name '%s' does not exist.",
                name.c_str())
                .c_str());
    }
    _jobs.erase(name);

    FEP3_RETURN_IF_FAILED(_job_registry_configuration.removeJobEntry(name));

    return {};
}

std::list<fep3::arya::JobInfo> JobRegistryImpl::getJobInfos() const
{
    std::list<fep3::arya::JobInfo> job_list;
    JobConfigurations job_configurations;

    const auto result = readJobConfigurationsFromPropertyNode(
        *_job_registry_configuration._jobs_node, job_configurations);

    if (!result) {
        return job_list;
    }

    std::transform(job_configurations.begin(),
                   job_configurations.end(),
                   std::back_inserter(job_list),
                   [](auto map_entry) {
                       return fep3::arya::JobInfo{map_entry.first, map_entry.second};
                   });

    return job_list;
}

std::list<fep3::catelyn::JobInfo> JobRegistryImpl::getJobInfosCatelyn(void) const
{
    std::list<fep3::catelyn::JobInfo> job_list;
    JobConfigurationPtrs job_configurations;

    const auto result = readJobConfigurationsFromPropertyNode(
        *_job_registry_configuration._jobs_node, job_configurations);

    if (!result) {
        return job_list;
    }

    std::transform(job_configurations.begin(),
                   job_configurations.end(),
                   std::back_inserter(job_list),
                   [](auto& map_entry) {
                       return fep3::catelyn::JobInfo{
                           map_entry.first,
                           std::unique_ptr<catelyn::JobConfiguration>{std::move(map_entry.second)}};
                   });

    return job_list;
}

fep3::arya::Jobs JobRegistryImpl::getJobs() const
{
    arya::Jobs jobs;

    for (const auto& job_info: getJobInfos()) {
        const auto job = _jobs.find(job_info.getName());
        if (_jobs.end() != job) {
            jobs.emplace(std::make_pair(
                job_info.getName(),
                fep3::arya::JobEntry{
                    job->second, fep3::arya::JobInfo{job_info.getName(), job_info.getConfig()}}));
        }
    }

    return jobs;
}

fep3::catelyn::Jobs JobRegistryImpl::getJobsCatelyn() const
{
    fep3::catelyn::Jobs jobs;

    for (const auto& job_info: getJobInfosCatelyn()) {
        const auto job = _jobs.find(job_info.getName());
        if (_jobs.end() != job) {
            jobs.emplace(job_info.getName(),
                         fep3::catelyn::JobEntry{
                             job->second,
                             fep3::catelyn::JobInfo{job_info.getName(), job_info.getConfigCopy()}});
        }
    }

    return jobs;
}

} // namespace native
} // namespace fep3
