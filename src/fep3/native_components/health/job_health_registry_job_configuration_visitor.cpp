/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "job_health_registry_job_configuration_visitor.h"

namespace fep3 {
namespace native {
JobHealthRegistryJobConfigurationVisitor::JobHealthRegistryJobConfigurationVisitor(
    std::vector<IJobHealthRegistry::JobHealthiness>& jobs_healthiness,
    const catelyn::JobEntry* current_processed_job)
    : _jobs_healthiness(jobs_healthiness), _current_processed_job(current_processed_job)
{
}

Result JobHealthRegistryJobConfigurationVisitor::visitClockTriggeredConfiguration(
    const ClockTriggeredJobConfiguration& configuration)
{
    _jobs_healthiness.push_back(fep3::native::IJobHealthRegistry::JobHealthiness{
        _current_processed_job->job_info.getName(),
        IJobHealthRegistry::JobHealthiness::ClockTriggeredJobInfo{configuration._cycle_sim_time}});
    return {};
}

Result JobHealthRegistryJobConfigurationVisitor::visitDataTriggeredConfiguration(
    const DataTriggeredJobConfiguration& configuration)
{
    _jobs_healthiness.push_back(fep3::native::IJobHealthRegistry::JobHealthiness{
        _current_processed_job->job_info.getName(),
        IJobHealthRegistry::JobHealthiness::DataTriggeredJobInfo{configuration._signal_names}});
    return {};
}

} // namespace native
} // namespace fep3
