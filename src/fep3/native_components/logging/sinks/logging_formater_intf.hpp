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

#include <fep3/base/logging/logging_types.h>
#include <iostream>

namespace fep3
{
    /**
     * @brief Class for formatting the log messages into an ouput format.
     */
    class ILoggingFormater{
    public:

        virtual std::string formatLogMessage(const LogMessage& log) const = 0;
        virtual ~ILoggingFormater() = default;

        /**
        * @brief Returns the string to be written before writing any actual log entry.
        * To be called in thread safe context.
        *
        * @return String to be written at the start of the logging stream.
        */
        virtual std::string GetStreamBegin() const
        {
            return "";
        };

        /**
        * @brief Appends the end characters in the log stream. To be called in thread safe context.
        *
        * @param [in] ostream The output logging stream.
        */
        virtual void StreamEnd(std::ostream&) const
        {
        };

        /**
        * @brief Checks if existing log entries are compatible to the actual logging format. 
        * To be called in thread safe context.
        *
        * @param [in] ostream The output logging stream.
        *
        * @return True if the log stream can be appended without resulting in corrupted format.
        */
        virtual bool IsStreamAppendable(std::iostream&)
        {
            return true;
        };
    };
} // namespace fep3
