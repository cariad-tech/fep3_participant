/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "job_runner.h"

#include <fep3/components/logging/easy_logger.h>

#include <cassert>
#include <stdexcept>

namespace fep3 {
namespace native {

using Strategy = fep3::JobConfiguration::TimeViolationStrategy;

JobRunner::JobRunner(const std::string& name,
                     const fep3::JobConfiguration::TimeViolationStrategy& time_violation_strategy,
                     const fep3::Optional<fep3::Duration>& max_runtime,
                     const std::shared_ptr<const fep3::ILogger>& logger)
    : _name(name),
      _time_violation_strategy(time_violation_strategy),
      _max_runtime(max_runtime),
      _logger(logger),
      _cancelled(false),
      _skip_output(false),
      _health_service(nullptr)
{
    if (!_logger) {
        throw std::runtime_error("No logger provided");
    }
}

JobRunner::JobRunner(const std::string& name,
                     const fep3::JobConfiguration::TimeViolationStrategy& time_violation_strategy,
                     const fep3::Optional<fep3::Duration>& max_runtime,
                     const std::shared_ptr<const fep3::ILogger>& logger,
                     IHealthService& healthService)
    : JobRunner(name, time_violation_strategy, max_runtime, logger)
{
    _health_service = &healthService;
}

fep3::Result JobRunner::runJob(const Timestamp trigger_time, fep3::IJob& job)
{
    if (trigger_time < Timestamp(0)) {
        _logger->logFatal(a_util::strings::format("Negative trigger timestamp (%lld) in JobRunner.",
                                                  trigger_time.count()));
        assert(!"Jobs may not be triggered with a timestamp < 0");
    }

    using namespace a_util::strings;

    if (_cancelled) {
        return CREATE_ERROR_DESCRIPTION(
            ERR_CANCELLED,
            format("The job '%s' was not executed because it was cancelled", _name.c_str())
                .c_str());
    }

    _skip_output = false;

    IHealthService::JobExecuteResult execution_result;
    execution_result.simulation_time = trigger_time;
    FEP3_ARYA_LOGGER_LOG_DEBUG(_logger,
                               a_util::strings::format("Job %s: Calling executeDataIn with %lld ns",
                                                       _name.c_str(),
                                                       trigger_time.count()));
    execution_result.result_execute_data_in = job.executeDataIn(trigger_time);
    if (!execution_result.result_execute_data_in) {
        _logger->logWarning(a_util::strings::format(
            "Job %s: Execution of data input step failed for this processing cycle.",
            _name.c_str()));
    }

    FEP3_ARYA_LOGGER_LOG_DEBUG(_logger,
                               a_util::strings::format("Job %s: Calling execute", _name.c_str()));
    auto begin = std::chrono::steady_clock::now();
    execution_result.result_execute = job.execute(trigger_time);

    auto end = std::chrono::steady_clock::now();

    auto execution_time = end - begin;

    if (!execution_result.result_execute) {
        _logger->logWarning(a_util::strings::format(
            "Job %s: Execution of data processing step failed for this processing cycle.",
            _name.c_str()));
    }

    auto do_runtime_check = _max_runtime.has_value();
    if (do_runtime_check && execution_time > _max_runtime.value()) {
        FEP3_RETURN_IF_FAILED(applyTimeViolationStrategy(execution_time));
    }

    if (!_skip_output) {
        FEP3_ARYA_LOGGER_LOG_DEBUG(
            _logger, a_util::strings::format("Job %s: Calling executeDataOut", _name.c_str()));
        execution_result.result_execute_data_out = job.executeDataOut(trigger_time);
        if (!execution_result.result_execute_data_out) {
            _logger->logWarning(a_util::strings::format(
                "Job %s: Execution of data output step failed for this processing cycle.",
                _name.c_str()));
        }
    }
    else {
        FEP3_ARYA_LOGGER_LOG_DEBUG(
            _logger,
            a_util::strings::format("Job %s: Skipped calling executeDataOut", _name.c_str()));
    }

    if (_health_service) {
        auto update_job_result = _health_service->updateJobStatus(_name, execution_result);
        FEP3_ARYA_LOGGER_LOG_RESULT(_logger, update_job_result);
    }

    FEP3_ARYA_LOGGER_LOG_DEBUG(_logger,
                               a_util::strings::format("All calls to job %s done", _name.c_str()));
    return execution_result.result_execute;
}

fep3::Result JobRunner::applyTimeViolationStrategy(const Timestamp process_duration)
{
    fep3::Result result = {};
    switch (_time_violation_strategy) {
    case Strategy::ignore_runtime_violation:
        // ignore
        break;
    case Strategy::warn_about_runtime_violation:
        _logger->logWarning(a_util::strings::format(
            "Job %s: Computation time (%d us) exceeded configured maximum runtime.",
            _name.c_str(),
            process_duration.count()));

        result = fep3::ERR_NOERROR;
        break;
    case Strategy::skip_output_publish:
        _logger->logError(a_util::strings::format(
            "Job %s: Computation time (%d us) exceeded configured maximum runtime. "
            "CAUTION: "
            "defined output in data writer queues will not be published during this processing "
            "cycle!",
            _name.c_str(),
            process_duration.count()));

        _skip_output = true;
        result = fep3::ERR_NOERROR;
        break;
    case Strategy::unknown:
        // Should never be the case
        break;
    }
    return result;
}

} // namespace native
} // namespace fep3
