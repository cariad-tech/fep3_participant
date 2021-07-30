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

#include <fep3/fep3_errors.h>
#include <plugins/rti_dds/simulation_bus/rti_conext_dds_include.h>

/**
 * Convert std timestamp into dds timestamp
 */
dds::core::Time convertTimestamp(const std::chrono::nanoseconds& timestamp);

/**
 * Convert dds timestamp into std timestamp
 */
std::chrono::nanoseconds convertTimestamp(const dds::core::Time& timestamp);

/**
 * Convert dds exception into fep3::Result
 */
fep3::Result convertExceptionToResult(const dds::core::Exception & exception);

/**
 * Convert std::exception into fep3::Result
 */
fep3::Result convertExceptionToResult(const std::exception& exception);
