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

#include <string>

#include <fep3/components/base/component_iid.h>
#include <fep3/base/health/health_types.h>

namespace fep3
{
namespace experimental
{

/**
 * Health service interface which provides getter for current participant and system health state
 * and functionality to set a participant to state error.
 */
class IHealthService
{

public:
    FEP_COMPONENT_IID("health_service.experimental.fep3.iid")
    virtual ~IHealthService() = default;

    /**
     * @brief Set the health state of this participant to state error and log the provided message.
     * Setting the health state to error indicates an error has occurred which prevents the participant
     * from working correctly. The error has to be taken care of from external before the health state may be reset.
     *
     * @param message log message indicating why the health state has been set to error.
     * @return Result indicating whether setting the health state succeeded or failed.
     * @retval ERR_NOERROR Health state has been set successfully.
     */
    virtual Result setHealthToError(const std::string& message) = 0;

    /**
     * @brief Reset the health state of this participant to state ok and log the provided message.
     * Resetting the health state to ok indicates all errors have been resolved and the participant
     * works correctly.
     * An error health state should only be reset to health state ok from external using rpc,
     * not from inside the participant.
     *
     * @param message log message indicating why the health state has been set to ok.
     * @return Result indicating whether setting the health state succeeded or failed.
     * @retval ERR_NOERROR Health state has been set successfully.
     */
    virtual Result resetHealth(const std::string& message) = 0;

    /**
     * @brief Get the current health state of the participant.
     *
     * @return HealthState The current health state of the participant.
     */
    virtual HealthState getHealth() = 0;
};

//using arya::IHealthService;
} // namespace experimental
} // namespace fep3
