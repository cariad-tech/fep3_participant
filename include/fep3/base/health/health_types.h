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

 //Guideline - FEP System Library API Exception
#ifndef _FEP_BASE_HEALTH_TYPES_H
#define _FEP_BASE_HEALTH_TYPES_H

#include <string>

namespace fep3
{
namespace experimental
{
/// Health states of a participant.
enum class HealthState
{
    /// good health state
    ok = 0,
    /// participant is no longer working correctly
    error = 1,
    /// unknown health state
    unknown = 2
};

/**
* @brief Return a health state for a given string.
*
* The string parameter must match one of the health state names.
* In case of no match, the unknown state is returned.
*
* @param health_state_string The string to derive a health state from.
*
* @return HealthState matching the provided string.
*/
inline HealthState stringToHealthState(const std::string& health_state_string)
{
    if ("ok" == health_state_string)
    {
        return HealthState::ok;
    }
    else if ("error" == health_state_string)
    {
        return HealthState::error;
    }
    else
    {
        return HealthState::unknown;
    }
}

/**
 * @brief Return the configured health state as string.
 *
 * @param health_state The health state to derive a string from.
 *
 * @return The configured health state as std::string.
 */
inline std::string healthStateToString(const HealthState& health_state)
{
    switch (health_state)
    {
    case HealthState::ok:
        return "ok";
    case HealthState::error:
        return "error";
    default:
        return "unknown";
    }
}

//using arya::HealthState;
//using arya::stringToHealthState;
//using arya::healthStateToString;
} // namespace experimental
} // namespace fep3

#endif //_FEP_BASE_HEALTH_TYPES_H
