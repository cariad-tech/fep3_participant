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


#include "job_runner.h"

#include <cassert>
#include <a_util/strings/strings_format.h>

namespace fep3
{
namespace native
{

using Strategy = fep3::JobConfiguration::TimeViolationStrategy;

JobRunner::JobRunner(
    const std::string& name,
    const fep3::JobConfiguration::TimeViolationStrategy& time_violation_strategy,
    const fep3::Optional<fep3::Duration>& max_runtime,
    const std::shared_ptr<const fep3::ILogger>& logger)
        : _name(name)
        , _time_violation_strategy(time_violation_strategy)
        , _max_runtime(max_runtime)
        , _logger(logger)
        , _cancelled(false)
        , _skip_output(false)
{
    if (!_logger)
    {
        throw std::runtime_error("No logger provided");
    }
}

fep3::Result JobRunner::runJob(const Timestamp trigger_time, fep3::IJob& job)
{
    if (trigger_time < Timestamp(0))
    {
        _logger->logFatal(a_util::strings::format("Negative trigger timestamp (%lld) in JobRunner.", trigger_time.count()));
        assert(!"Jobs may not be triggered with a timestamp < 0");
    }

    using namespace a_util::strings;

    if (_cancelled)
    {
        return CREATE_ERROR_DESCRIPTION(
            ERR_CANCELLED,
            format("The job '%s' was not executed because it was cancelled", _name.c_str()).c_str());
    }

    _skip_output = false;

    if (fep3::isFailed(job.executeDataIn(trigger_time)))
    {
        _logger->logWarning(
            a_util::strings::format("Job %s: Execution of data input step failed for this processing cycle.",
                _name.c_str()));
    }

    auto begin = std::chrono::steady_clock::now();
    auto result = job.execute(trigger_time);
    auto end = std::chrono::steady_clock::now();

    auto execution_time = end - begin;

    if (isFailed(result))
    {
        _logger->logWarning(
            a_util::strings::format("Job %s: Execution of data processing step failed for this processing cycle.",
                _name.c_str()));
    }

    auto do_runtime_check = _max_runtime.has_value();
    if (do_runtime_check
            && execution_time > _max_runtime.value())
    {
        FEP3_RETURN_IF_FAILED(applyTimeViolationStrategy(execution_time));
    }


    if (!_skip_output)
    {
        if (fep3::isFailed(job.executeDataOut(trigger_time)))
        {
            _logger->logWarning(
                a_util::strings::format("Job %s: Execution of data output step failed for this processing cycle.",
                    _name.c_str()));
        }
    }

    return result;
}

fep3::Result JobRunner::applyTimeViolationStrategy(const Timestamp process_duration)
{
    fep3::Result result = {};
    switch (_time_violation_strategy)
    {
        case Strategy::ignore_runtime_violation:
            // ignore
            break;
        case Strategy::warn_about_runtime_violation:
            _logger->logWarning(
                a_util::strings::format("Job %s: Computation time (%d us) exceeded configured maximum runtime.",
                    _name.c_str(),
                    process_duration));

            result = fep3::ERR_NOERROR;
            break;
        case Strategy::skip_output_publish:
            _logger->logError(
                a_util::strings::format(
                    "Job %s: Computation time (%d us) exceeded configured maximum runtime. "
                    "CAUTION: "
                    "defined output in data writer queues will not be published during this processing cycle!",
                    _name.c_str(),
                    process_duration));

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
