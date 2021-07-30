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

#include <fep3/components/scheduler/scheduler_intf.h>

namespace fep3
{
namespace mock
{

struct Scheduler : public fep3::IScheduler
{
    MOCK_CONST_METHOD0(getName, std::string());
    MOCK_METHOD2(initialize, fep3::Result(fep3::IClockService&, const fep3::Jobs&));
    MOCK_METHOD0(start, fep3::Result());
    MOCK_METHOD0(stop, fep3::Result());
    MOCK_METHOD0(deinitialize, fep3::Result());
};

}
}
