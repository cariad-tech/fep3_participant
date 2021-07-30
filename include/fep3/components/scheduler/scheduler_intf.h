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

#include <fep3/components/base/component_iid.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/job_registry/job_intf.h>

/**
* @brief Name of the native scheduler implementation.
*/
#define FEP3_SCHEDULER_CLOCK_BASED "clock_based_scheduler"

namespace fep3
{
namespace arya
{

/**
* @brief Interface for a Scheduler
*
*/
class IScheduler
{
public:
    /// DTOR
    virtual ~IScheduler() = default;

    /**
    * @brief Get the Name of the scheduler
    *
    * @return Name of the scheduler
    */
    virtual std::string getName() const = 0;

    /**
    * @brief Initialize scheduler.
    *
    * @note The native implementation of @ref FEP3_SCHEDULER_CLOCK_BASED
    * uses the clock service and configurations of every registered job to create a timer thread
    * for every job which triggers the job at the configured cycle time.
    *
    * @param[in] clock The local clock service
    * @param[in] jobs The configurations of all jobs registered at the job registry
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    */
    virtual fep3::Result initialize(arya::IClockService& clock,
        const arya::Jobs& jobs) = 0;

    /**
    * @brief Start the scheduler.
    *
    * @note The native implementation of @ref FEP3_SCHEDULER_CLOCK_BASED
    * starts every job timer thread and the scheduler service thread.
    *
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    */
    virtual fep3::Result start() = 0;

    /**
    * @brief Stop the scheduler.
    *
    * @note The native implementation of the @ref FEP3_SCHEDULER_CLOCK_BASED
    * stops the scheduler service thread and every job timer thread.
    *
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    */
    virtual fep3::Result stop() = 0;

    /**
    * @brief Deinitialize the scheduler.
    *
    * The native implementation of the @ref FEP3_SCHEDULER_CLOCK_BASED
    * stops the scheduler service thread, every job timer thread and clears all job timer threads.
    *
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    */
    virtual fep3::Result deinitialize() = 0;
};

} // namespace arya
using arya::IScheduler;
} // namespace fep3