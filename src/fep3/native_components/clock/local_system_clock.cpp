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


#include "local_system_clock.h"

#include <a_util/system/system.h>

#include <fep3/components/clock/clock_service_intf.h>

namespace fep3
{
namespace native
{

LocalSystemRealClock::LocalSystemRealClock()
    : ContinuousClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME)
    , _current_offset(std::chrono::steady_clock::now())
{
}

Timestamp LocalSystemRealClock::getNewTime() const
{
    using namespace std::chrono;

    if (_started)
    {
        return Timestamp{steady_clock::now() - _current_offset};
    }
    else
    {
        return Timestamp{ 0 };
    }
}

Timestamp LocalSystemRealClock::resetTime(Timestamp new_time)
{
    using namespace std::chrono;

    _current_offset = steady_clock::now() - new_time;

    return getNewTime();
}

} // namespace native
} // namespace fep3
