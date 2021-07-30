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

#include <functional>

#include <fep3/fep3_errors.h>
#include <fep3/fep3_duration.h>
#include <fep3/fep3_optional.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/components/job_registry/job_registry_intf.h>

namespace fep3
{
namespace native
{

class JobRunner
{
public:
    JobRunner(const std::string& name,
                    const fep3::JobConfiguration::TimeViolationStrategy& time_violation_strategy,
                    const fep3::Optional<fep3::Duration>& max_runtime,
                    const std::shared_ptr<const fep3::ILogger>& logger);

    fep3::Result runJob(const Timestamp trigger_time, fep3::IJob& job);

private:
    fep3::Result applyTimeViolationStrategy(const Timestamp process_duration);

private:
    const std::string _name;
    fep3::JobConfiguration::TimeViolationStrategy _time_violation_strategy;
    fep3::Optional<fep3::Duration> _max_runtime;
    std::shared_ptr<const fep3::ILogger> _logger;

    bool _cancelled;
    bool _skip_output;
};

} // namespace native
} // namespace fep3


