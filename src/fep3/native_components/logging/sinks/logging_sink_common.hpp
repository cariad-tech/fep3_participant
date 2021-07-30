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

#include <a_util/datetime.h>
#include <fep3/base/logging/logging_types.h>

namespace fep3
{
namespace native
{

/**
* @brief (Optional) Helper function to ensure a consistent format for all kind of logs
*
* @param [out] log_msg The formatted logging string. The function will only append data to this parameter.
* @param [in] log The source log with all the information data.
*/
inline void formatLoggingString(std::string& log_msg, const LogMessage& log)
{
    log_msg.append(a_util::strings::format("[%s - %s]: ",
        a_util::datetime::getCurrentLocalDate().format("%d.%m.%Y").c_str(),
        a_util::datetime::getCurrentLocalTime().format("%H:%M:%S").c_str()));

    log_msg.append(a_util::strings::format("%s@%s ", log._participant_name.c_str(), log._logger_name.c_str()));
    log_msg.append(a_util::strings::format(" ST: %s[ns]  ", log._timestamp.c_str()));

    log_msg.append(getString(log._severity));

    log_msg.append(a_util::strings::format(" %s",
        log._message.c_str()));
}
}
} // namespace fep3
