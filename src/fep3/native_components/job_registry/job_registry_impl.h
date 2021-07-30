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
#include <memory>
#include <string>

#include <fep3/fep3_errors.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/native_components/job_registry/job_configurations.h>
#include <fep3/native_components/job_registry/local_job_registry.h>


namespace fep3
{
namespace native
{

struct JobRegistryConfiguration;

class JobRegistryImpl : public fep3::IJobRegistry
{
public:
    JobRegistryImpl(JobRegistryConfiguration& job_registry_configuration);
    ~JobRegistryImpl() = default;

public:
    // Inherited via IJobRegistry
    fep3::Result addJob(const std::string & name, const std::shared_ptr<IJob>& job, const JobConfiguration & job_config) override;
    fep3::Result removeJob(const std::string & name) override;
    std::list<JobInfo> getJobInfos() const override;
    fep3::Jobs getJobs() const override;

private:
    std::map<std::string, std::shared_ptr<fep3::IJob>> _jobs;
    JobRegistryConfiguration& _job_registry_configuration;
};

} // namespace native
} // namespace fep3
