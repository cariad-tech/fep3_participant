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

#include <chrono>

/**
 * FEP 3 SDK Participant Library namespace
 */
namespace fep3
{
/**
 * FEP 3 SDK Participant namespace for all interfaces and public implementations introduced in version 3.0.
 */
namespace arya
{

/**
 * Alias for timestamp in nanoseconds
 */
using Timestamp = std::chrono::nanoseconds;

} // namespace arya
using arya::Timestamp;
} // namespace fep3
