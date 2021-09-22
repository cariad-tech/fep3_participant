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

#include <gmock/gmock.h>

#include "mock_scheduler.h"

namespace fep3
{
namespace mock
{

struct SchedulerWithAccessToJobs
    : public Scheduler
{
private:
    fep3::Result initialize(fep3::IClockService& clock_service, const fep3::Jobs& jobs) override
    {
        _jobs = jobs;
        // call the mocked method to enable setting of expectations
        return Scheduler::initialize(clock_service, jobs);
    }

public:
    std::list<::fep3::IJob*> getJobs() const
    {
        std::list<::fep3::IJob*> jobs;
        for(const auto& job_entry : _jobs)
        {
            jobs.push_back(job_entry.second.job.get());
        }
        return jobs;
    }
private:
    fep3::Jobs _jobs;
};

}
}
