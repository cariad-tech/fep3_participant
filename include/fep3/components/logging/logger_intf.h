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

#include <fep3/fep3_result_decl.h>

namespace fep3
{
namespace arya
{

/**
 * Logger interface to log messages to.
 */
class ILogger
{
protected:
    /// DTOR
    ~ILogger() = default;

public:

    /**
    * Log informational messages that highlight the progress of the application.
    * @param[in] message Info message
    *
    * @return Standard Result value
    *
    * @retval ERR_MEMORY            The internal logging queue is full (console and listener)
    * @retval ERR_POINTER           No log file has been configured or opened (only file)
    * @retval ERR_DEVICE_IO         Writing log to file failed (only file)
    * @retval ERR_EXCEPTION_RAISED  The RPC failed and threw an exception (only listener)
    */
    virtual fep3::Result logInfo(const std::string& message) const = 0;

    /**
    * Log potentially harmful situations
    * @param[in] message Warning message
    *
    * @return Standard Result value. See @ref logInfo() for all possible values.
    */
    virtual fep3::Result logWarning(const std::string& message) const = 0;

    /**
    * Log error events that might still allow the application to continue running
    * @param[in] message Error message
    *
    * @return Standard Result value. See @ref logInfo() for all possible values.
    */
    virtual fep3::Result logError(const std::string& message) const = 0;

    /**
    * Log very severe error events that will presumably lead the application to abort
    * @param[in] message Fatal message
    *
    * @return Standard Result value. See @ref logInfo() for all possible values.
    */
    virtual fep3::Result logFatal(const std::string& message) const = 0;

    /**
    * Log informational events that are most useful for debugging
    * @param[in] message Debug message
    *
    * @return Standard Result value. See @ref logInfo() for all possible values.
    */
    virtual fep3::Result logDebug(const std::string& message) const = 0;

    /**
    * Check whether the logger is enabled for INFO priority.
    * @return @c true if INFO is enabled, @c false otherwise
    */
    virtual bool isInfoEnabled() const = 0;
    /**
    * Check whether the logger is enabled for WARNING priority.
    * @return @c true if WARNING is enabled, @c false otherwise
    */
    virtual bool isWarningEnabled() const = 0;
    /**
    * Check whether the logger is enabled for ERROR priority.
    * @return @c true if ERROR is enabled, @c false otherwise
    */
    virtual bool isErrorEnabled() const = 0;
    /**
    * Check whether the logger is enabled for FATAL priority.
    * @return @c true if FATAL is enabled, @c false otherwise
    */
    virtual bool isFatalEnabled() const = 0;
    /**
    * Check whether the logger is enabled for DEBUG priority.
    * @return @c true if DEBUG is enabled, @c false otherwise
    */
    virtual bool isDebugEnabled() const = 0;

};

} // namespace arya
using arya::ILogger;
} // namespace fep3
