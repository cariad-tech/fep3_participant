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

#include "mock_scheduler_service.h"
#include <fep3/components/base/c_access_wrapper/transferable_component_base.h>

namespace fep3
{
namespace mock
{

class TransferableSchedulerServiceWithAccessToSchedulers
    : public SchedulerService<fep3::plugin::c::TransferableComponentBase>
{
private:
    fep3::Result registerScheduler(std::unique_ptr<IScheduler> scheduler) override
    {
        const auto& pointer_to_scheduler = scheduler.get();
        registered_schedulers_.emplace_back(std::move(scheduler));
        // call the mocked method to enable setting of expectations
        return registerScheduler_(pointer_to_scheduler);
    }

public:
    std::list<::fep3::IScheduler*> getRegisteredSchedulers() const
    {
        std::list<::fep3::IScheduler*> schedulers;
        for(const auto& scheduler : registered_schedulers_)
        {
            schedulers.push_back(scheduler.get());
        }
        return schedulers;
    }
private:
    std::list<std::unique_ptr<::fep3::IScheduler>> registered_schedulers_;
};

}
}
