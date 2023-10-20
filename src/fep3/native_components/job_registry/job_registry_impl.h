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

namespace fep3 {
namespace native {

struct JobRegistryConfiguration;

class JobRegistryImpl : public fep3::IJobRegistry {
public:
    JobRegistryImpl(JobRegistryConfiguration& job_registry_configuration);
    ~JobRegistryImpl() = default;

public:
    // Inherited via IJobRegistry
    fep3::Result addJob(const std::string& name,
                        const std::shared_ptr<IJob>& job,
                        const fep3::arya::JobConfiguration& job_config) override;
    fep3::Result addJob(const std::string& name,
                        const std::shared_ptr<fep3::arya::IJob>& job,
                        const fep3::catelyn::JobConfiguration& job_config) override;
    fep3::Result removeJob(const std::string& name) override;
    std::list<fep3::arya::JobInfo> getJobInfos() const override;
    std::list<fep3::catelyn::JobInfo> getJobInfosCatelyn(void) const override;
    fep3::arya::Jobs getJobs() const override;
    fep3::catelyn::Jobs getJobsCatelyn() const override;

private:
    std::map<std::string, std::shared_ptr<fep3::IJob>> _jobs;
    JobRegistryConfiguration& _job_registry_configuration;
};

} // namespace native
} // namespace fep3
