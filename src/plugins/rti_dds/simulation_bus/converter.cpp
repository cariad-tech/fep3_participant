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


#include "converter.h"

dds::core::Time convertTimestamp(const std::chrono::nanoseconds& timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timestamp);
    auto nanoseconds = timestamp - seconds;
    return dds::core::Time(static_cast<uint32_t>(seconds.count()),
                           static_cast<uint32_t>(nanoseconds.count()));
}

std::chrono::nanoseconds convertTimestamp(const dds::core::Time& timestamp)
{
    auto seconds = std::chrono::seconds(timestamp.sec());
    auto nanoseconds = std::chrono::nanoseconds(timestamp.nanosec());
    return seconds + nanoseconds;
}

fep3::Result convertExceptionToResult(const dds::core::Exception & exception)
{
    RETURN_ERROR_DESCRIPTION(fep3::ERR_FAILED,
        "simulation bus: rti connext: %s",
        exception.what());
}

fep3::Result convertExceptionToResult(const std::exception& exception)
{
    RETURN_ERROR_DESCRIPTION(fep3::ERR_FAILED,
        "simulation bus: rti connext: %s",
        exception.what());
}