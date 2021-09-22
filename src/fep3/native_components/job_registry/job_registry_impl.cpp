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


#include "job_registry_impl.h"

#include <algorithm>
#include <iterator>

namespace fep3
{
namespace native
{

JobRegistryImpl::JobRegistryImpl(JobRegistryConfiguration& job_registry_configuration)
    : _job_registry_configuration(job_registry_configuration)
{
}

fep3::Result JobRegistryImpl::addJob(const std::string& name, const std::shared_ptr<fep3::IJob>& job, const JobConfiguration& job_config)
{
    if (_jobs.count(name) > 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_RESOURCE_IN_USE,
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
    if (_jobs.count(name) == 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
            a_util::strings::format(
                "Removing job from job registry failed. A job with the name '%s' does not exist.",
                name.c_str())
            .c_str());
    }
    _jobs.erase(name);

    FEP3_RETURN_IF_FAILED(_job_registry_configuration.removeJobEntry(name));

    return {};
}

std::list<JobInfo> JobRegistryImpl::getJobInfos() const
{
    std::list<fep3::JobInfo> job_list;
    JobConfigurations job_configurations;

    const auto result = readJobConfigurationsFromPropertyNode(
                *_job_registry_configuration._jobs_node,
                job_configurations);

    if (isFailed(result))
    {
        return job_list;
    }

    std::transform(job_configurations.begin(), job_configurations.end(),
        std::back_inserter(job_list),
        [](auto map_entry){return fep3::JobInfo{map_entry.first, map_entry.second};});

    return job_list;
}

fep3::Jobs JobRegistryImpl::getJobs() const
{
    fep3::Jobs jobs;

    for (const auto& job_info : getJobInfos())
    {
        const auto job = _jobs.find(job_info.getName());
        if (_jobs.end() != job)
        {
            jobs.emplace(std::make_pair(job_info.getName(), JobEntry{
                                            job->second, JobInfo{
                                                job_info.getName(), job_info.getConfig()}}));
        }
    }

    return jobs;
}

} // namespace native
} // namespace fep3
