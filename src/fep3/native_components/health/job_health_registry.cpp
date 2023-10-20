/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2022 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "job_health_registry_job_configuration_visitor.h"

#include <algorithm>

namespace fep3 {
namespace native {

void JobHealthRegistry::initialize(const fep3::Jobs& jobs,
                                   std::shared_ptr<fep3::arya::ILogger> logger)
{
    _logger = std::move(logger);
    std::unique_lock lock(_mutex);
    for (const auto& job: jobs) {
        _current_processed_job = &job.second;
        auto job_config = _current_processed_job->job_info.getConfigCopy();
        JobHealthRegistryJobConfigurationVisitor job_conf_visitor(_jobs_healthiness,
                                                                  _current_processed_job);
        job_config->acceptVisitor(job_conf_visitor);
    }
}

void JobHealthRegistry::deinitialize()
{
    {
        std::unique_lock lock(_mutex);
        _jobs_healthiness.clear();
    }
    _logger.reset();
}

Result JobHealthRegistry::resetHealth()
{
    std::unique_lock lock(_mutex);

    for (auto& job_health: _jobs_healthiness) {
        job_health.execute_data_in_error = IJobHealthRegistry::JobHealthiness::ExecuteError{};
        job_health.execute_error = IJobHealthRegistry::JobHealthiness::ExecuteError{};
        job_health.execute_data_out_error = IJobHealthRegistry::JobHealthiness::ExecuteError{};
    }

    return {};
}

std::vector<IJobHealthRegistry::JobHealthiness> JobHealthRegistry::getHealth() const
{
    std::shared_lock lock(_mutex);
    return _jobs_healthiness;
}

fep3::Result JobHealthRegistry::updateJobStatus(
    const std::string& job_name, const fep3::IHealthService::JobExecuteResult& result)
{
    std::unique_lock lock(_mutex);
    auto it = std::find_if(_jobs_healthiness.begin(),
                           _jobs_healthiness.end(),
                           [&](const JobHealthiness& job_healthiness) {
                               return job_healthiness.job_name == job_name;
                           });

    if (it == _jobs_healthiness.end()) {
        RETURN_ERROR_DESCRIPTION(
            ERR_NOT_FOUND,
            a_util::strings::format("Cannot update job status, job: '%s' not found",
                                    job_name.c_str())
                .c_str());
    }

    it->simulation_time = result.simulation_time;

    updateExecuteError(
        it->execute_data_in_error, result.result_execute_data_in, result.simulation_time);
    updateExecuteError(it->execute_error, result.result_execute, result.simulation_time);
    updateExecuteError(
        it->execute_data_out_error, result.result_execute_data_out, result.simulation_time);

    return {};
}

void JobHealthRegistry::updateExecuteError(JobHealthiness::ExecuteError& execute_error,
                                           const fep3::Result result,
                                           fep3::Timestamp simulation_time)
{
    if (!result) {
        ++execute_error.error_count;
        execute_error.last_error = result;
        execute_error.simulation_time = simulation_time;
    }
}

} // namespace native
} // namespace fep3
