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

#include "threaded_executor.h"

#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/native_components/scheduler/job_runner.h>

namespace fep3 {
namespace native {
class DataTriggeredExecutor {
public:
    DataTriggeredExecutor(IThreadPoolExecutor& threaded_executor)
        : _threaded_executor(threaded_executor)
    {
    }

    void post(std::function<void()> f)
    {
        if (_running) {
            _threaded_executor.post(f);
        }
    }

    void start()
    {
        _running = true;
    }

    void stop()
    {
        _running = false;
    }

private:
    IThreadPoolExecutor& _threaded_executor;
    std::atomic<bool> _running{false};
};
} // namespace native
} // namespace fep3
