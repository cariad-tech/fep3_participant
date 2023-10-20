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

#pragma once

#include <fep3/components/health_service/health_service_intf.h>
#include <fep3/components/job_registry/job_intf.h>
#include <fep3/components/logging/logger_intf.h>

#include <shared_mutex>
#include <variant>

namespace fep3 {
namespace native {

class IJobHealthRegistry {
protected:
    IJobHealthRegistry(const IJobHealthRegistry&) = default;
    IJobHealthRegistry& operator=(const IJobHealthRegistry&) = default;
    IJobHealthRegistry(IJobHealthRegistry&&) = default;
    IJobHealthRegistry& operator=(IJobHealthRegistry&&) = default;

public:
    virtual ~IJobHealthRegistry() = default;
    IJobHealthRegistry() = default;
    /**
     * @brief Healthiness structure of a job.
     */
    struct JobHealthiness {
        /**
         * Holds information about the clock triggered job.
         */
        struct ClockTriggeredJobInfo {
            const fep3::Timestamp cycle_time;
        };
        /**
         * Holds information about the data triggered job.
         */
        struct DataTriggeredJobInfo {
            const std::vector<std::string> trigger_signals;
        };

        /// name of the job
        const std::string job_name;
        /// job relevant information
        const std::variant<ClockTriggeredJobInfo, DataTriggeredJobInfo> job_info;
        /// last simulation time that JobHealthiness was updated.
        fep3::Timestamp simulation_time = std::chrono::nanoseconds(0);
        /**
         * .@brief Holds information about the last error of job execution.
         */
        struct ExecuteError {
            /// number of times the job returned an non zero error code<summary>
            uint64_t error_count = 0;
            /// last simulation time that error was returned.
            fep3::Timestamp simulation_time = std::chrono::nanoseconds(0);
            /// last non zero error from job execution<summary>
            fep3::Result last_error = fep3::ERR_NOERROR;
        };
        /// holds last execution error of @ref fep3::arya::IJob::executeDataIn
        ExecuteError execute_data_in_error = {};
        /// holds last execution error of @ref fep3::arya::IJob::execute
        ExecuteError execute_error = {};
        /// holds last execution error of @ref fep3::arya::IJob::executeDataOut
        ExecuteError execute_data_out_error = {};
    };
    virtual void initialize(const fep3::Jobs& jobs,
                            std::shared_ptr<fep3::arya::ILogger> logger) = 0;
    virtual void deinitialize() = 0;
    virtual Result resetHealth() = 0;
    virtual Result updateJobStatus(const std::string& job_name,
                                   const fep3::IHealthService::JobExecuteResult& result) = 0;
    virtual std::vector<JobHealthiness> getHealth() const = 0;
};

class JobHealthRegistry : public IJobHealthRegistry {
public:
    JobHealthRegistry() = default;
    void initialize(const fep3::Jobs& jobs, std::shared_ptr<fep3::arya::ILogger> logger) override;
    void deinitialize() override;
    Result resetHealth() override;
    Result updateJobStatus(const std::string& job_name,
                           const fep3::IHealthService::JobExecuteResult& result) override;
    std::vector<JobHealthiness> getHealth() const override;

private:
    void updateExecuteError(JobHealthiness::ExecuteError& execute_error,
                            const fep3::Result result,
                            fep3::Timestamp simulation_time);
    std::vector<JobHealthiness> _jobs_healthiness;
    std::shared_ptr<fep3::arya::ILogger> _logger;
    const catelyn::JobEntry* _current_processed_job = nullptr;
    mutable std::shared_mutex _mutex;
};

} // namespace native
} // namespace fep3
