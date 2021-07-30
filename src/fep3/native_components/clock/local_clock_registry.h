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

#include <mutex>

#include <fep3/components/clock/clock_registry_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include "fep3/components/logging/easy_logger.h"

namespace fep3
{
namespace native
{

/**
 * @brief Thread safe clocks wrapper
 */
class ClocksSynchronized
{
public:
    fep3::Result registerClock(const std::shared_ptr<IClock>& clock);
    fep3::Result unregisterClock(const std::string& name);
    std::list<std::string> getClockNames() const;
    std::shared_ptr<IClock> findClock(const std::string& name) const;

private:
    mutable std::mutex _mutex;
     IClockRegistry::Clocks _clocks;
};

/**
* @brief Native implementation of a clock registry.
*/
class LocalClockRegistry
    : public IClockRegistry
    , public base::EasyLogging
{
public: // inherited via IClockRegistry
    fep3::Result registerClock(const std::shared_ptr<IClock>& clock) override;
    fep3::Result unregisterClock(const std::string& clock_name) override;
    std::list<std::string> getClockNames() const override;
    std::shared_ptr<IClock> findClock(const std::string& clock_name) const override;

private:
    ClocksSynchronized _clocks_synchronized;
};

} // namespace native
} // namespace fep3
