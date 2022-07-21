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

#include "logging_sink_file_base.hpp"
#include "logging_formater_fep.hpp"
#include "logging_formater_json.hpp"

namespace fep3
{

/**
 * @brief Implementation of the file logging with the FEP Console format.
 */
class LoggingSinkFile : public LoggingSinkFileBase {
public:
    LoggingSinkFile() 
        : LoggingSinkFileBase(std::make_unique<LoggingFormaterFep>())
    {
    }
};

/**
 * @brief Implementation of the file logging with the line oriented JSON format.
 */
class LoggingSinkFileJson : public LoggingSinkFileBase {
public:
    LoggingSinkFileJson()
        : LoggingSinkFileBase(std::make_unique<LoggingFormaterJson>())
    {
    }
};

} // namespace fep3
