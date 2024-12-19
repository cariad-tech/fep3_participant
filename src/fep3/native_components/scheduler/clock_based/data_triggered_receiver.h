/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "data_triggered_executor.h"

#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/native_components/scheduler/job_runner.h>

#include <atomic>

namespace fep3 {
namespace native {
template <typename JobRunnerType = JobRunner>
class DataTriggeredReceiver : public fep3::arya::ISimulationBus::IDataReceiver {
public:
    DataTriggeredReceiver(const std::function<Timestamp()>& time_getter,
                          std::shared_ptr<fep3::IJob> data_triggered_job,
                          const std::string& signal_name,
                          const JobRunnerType& job_runner,
                          DataTriggeredExecutor& data_triggered_executor,
                          std::shared_ptr<const fep3::ILogger> logger = nullptr)
        : _time_getter(time_getter),
          _data_triggered_job(data_triggered_job),
          _signal_name(signal_name),
          _job_runner(job_runner),
          _data_triggered_executor(data_triggered_executor),
          _logger(logger)
    {
    }

    /**
     * @brief Receives a stream type item
     *
     * @param[in] type The received stream type
     */
    void operator()(const std::shared_ptr<const fep3::IStreamType>&) override{
        // no need to trigger the job
    };

    /**
     * @brief Receives a data sample item
     *
     * This operator will block the data reader thread on RTI DDS.
     * Thus it will not be triggered again if the previous job not finished.
     * Use AsyncWaitSet if you don't want to be blocking on other readers.
     *
     * @param[in] sample The received data sample
     */
    void operator()(const std::shared_ptr<const fep3::IDataSample>&) override
    {
        FEP3_ARYA_LOGGER_LOG_DEBUG(
            _logger,
            a_util::strings::format("Received callback from signal: %s", _signal_name.c_str()));

        // if already running we prevent posting another task
        // however this does not prevent a task being posted
        // if the previous task is not yet running in the thread pool
        // and set the flag
        if (!_running) {
            FEP3_ARYA_LOGGER_LOG_DEBUG(
                _logger,
                a_util::strings::format("Triggering job with callback from signal: %s",
                                        _signal_name.c_str()));
            postTaskToExecutor();
        }
        else {
            FEP3_ARYA_LOGGER_LOG_WARNING(
                _logger,
                a_util::strings::format(
                    "Job for signal '%s' still running, can not be triggered now",
                    _signal_name.c_str()));
        }
    };

    const std::string& getSignalName() const
    {
        return _signal_name;
    }

private:
    void postTaskToExecutor()
    {
        fep3::Result result = _data_triggered_executor.post([this]() {
            // atomic_exchange returns the previous value of running
            if (std::atomic_exchange(&_running, true)) {
                FEP3_ARYA_LOGGER_LOG_WARNING(
                    _logger,
                    a_util::strings::format(
                        "Job for signal '%s' still running, can not be triggered now",
                        _signal_name.c_str()));
                return;
            }
            else {
                _job_runner.runJob(_time_getter(), *_data_triggered_job);
                _running = false;
            }
        });

        if (!result) {
            FEP3_ARYA_LOGGER_LOG_WARNING(
                _logger,
                a_util::strings::format(
                    "Signal '%s' received but scheduler cannot trigger job, error: %s",
                    _signal_name.c_str(),
                    result.getDescription()));
        }
    }

    const std::function<Timestamp()> _time_getter;
    std::shared_ptr<fep3::IJob> _data_triggered_job;
    const std::string _signal_name;
    JobRunnerType _job_runner;
    fep3::native::DataTriggeredExecutor& _data_triggered_executor;
    std::atomic<bool> _running{false};
    std::shared_ptr<const fep3::ILogger> _logger;
};
} // namespace native
} // namespace fep3
