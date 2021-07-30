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

#include <fep3/components/clock/clock_base.h>

namespace fep3
{
namespace native
{

/**
* @brief Native implementation of a continuous clock.
*/
class LocalSystemRealClock : public base::ContinuousClock
{
public:
    /**
    * CTOR
    */
    LocalSystemRealClock();

    /**
    * DTOR
    */
    ~LocalSystemRealClock() = default;

public:
    /**
    * @brief Return the passed time since the last reset.
    *
    * @return Passed time
    */
    Timestamp getNewTime() const override;

    /**
    * @brief Reset the clock and return the reset time.
    *
    * @param[in] new_time The new time of the clock
    * @return Passed time
    */
    Timestamp resetTime(Timestamp new_time) override;
private:
    /// Clock offset which is set during reset calls.
    mutable std::chrono::time_point<std::chrono::steady_clock> _current_offset;
};

} // namespace native
} // namespace fep3
