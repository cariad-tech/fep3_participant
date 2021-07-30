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

#include <memory>

#include <fep3/fep3_errors.h>
#include <fep3/fep3_participant_export.h>
#include "scheduler_intf.h"
namespace fep3
{
namespace arya
{

/**
* @brief Interface for the scheduler registry
*
*/
class ISchedulerRegistry
{
protected:
    /// DTOR
    ~ISchedulerRegistry() = default;

public:
    /**
    * @brief Register the given @p scheduler.
    *
    * Registered schedulers may be set as active scheduler.
    *
    * @param[in] scheduler Scheduler to be registered
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    * @retval ERR_INVALID_STATE The Participant is already running
    * @retval ERR_RESOURCE_IN_USE Scheduler with the given name is already registered
    */
    virtual fep3::Result registerScheduler(std::unique_ptr<arya::IScheduler> scheduler) = 0;

    /**
    * @brief Unregister the scheduler with the given @p scheduler_name.
    *
    * @param[in] scheduler_name The name of the scheduler
    * @returns FEP Result
    * @retval ERR_NOERROR Everything went fine
    * @retval ERR_INVALID_STATE The Participant is already running
    * @retval ERR_NOT_FOUND A scheduler with name @p scheduler_name is not registered
    * @retval ERR_INVALID_ARG You tried to unregister the default scheduler, which is not possible
    */
    virtual fep3::Result unregisterScheduler(const std::string& scheduler_name) = 0;

    /**
    * @brief Return the names of all registered schedulers.
    *
    * @return List of all registered scheduler names
    */
    virtual std::list<std::string> getSchedulerNames() const = 0;
};

} // namespace arya
using arya::ISchedulerRegistry;
} // namespace fep3