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

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/base/component_iid.h>
#include <fep3/base/logging/logging_types.h>
#include <fep3/base/properties/properties_intf.h>
#include "logger_intf.h"

#include <string>
#include <memory>

/**
* @brief Main property entry of the logging service properties
*/
#define FEP3_LOGGING_SERVICE_CONFIG "logging"

/**
 * @brief The logging configuration property name for default sinks to use.
 * by native following values are possible as list:
 * \li console
 * \li rpc
 * \li file
 * By default "rpc,console" is used.
 *
 */
#define FEP3_LOGGING_DEFAULT_SINKS_PROPERTY "default_sinks"

/**
 * @brief The logging configuration property path for default sinks to use.
 * by native following values are possible as list:
 * \li console
 * \li rpc
 * \li file
 * By default "rpc,console" is used.
 *
 */
#define FEP3_LOGGING_DEFAULT_SINKS  FEP3_LOGGING_SERVICE_CONFIG "/" FEP3_LOGGING_DEFAULT_SINKS_PROPERTY

/**
 * @brief The logging configuration property name for the default severity level
 *
 */
#define FEP3_LOGGING_DEFAULT_SEVERITY_PROPERTY "default_severity"

/**
 * @brief The logging configuration path name for the default severity level.
 *
 *
 */
#define FEP3_LOGGING_DEFAULT_SEVERITY FEP3_LOGGING_SERVICE_CONFIG "/" FEP3_LOGGING_DEFAULT_SEVERITY_PROPERTY

/**
 * @brief The logging configuration property name for the default file used in the filesink
 *
 */
#define FEP3_LOGGING_DEFAULT_FILE_SINK_PROPERTY "default_sink_file"

/**
 * @brief The logging configuration property path for the default file used in the filesink
 *
 */
#define FEP3_LOGGING_DEFAULT_FILE_SINK FEP3_LOGGING_SERVICE_CONFIG "/" FEP3_LOGGING_DEFAULT_FILE_SINK_PROPERTY


namespace fep3
{
namespace arya
{
    /**
     * Logging service of one participant.
     *
     * The logging service will provide a single logging access point a within a participant as component.
     *
     */
    class ILoggingService
    {
    protected:
        /// DTOR
        ~ILoggingService() = default;

    public:
        /**
         * @brief Logging sink interface for all native as well as custom logging sinks.
         */
        class ILoggingSink : public arya::IProperties
        {
        public:
            /**
             * DTOR public and virtual !
             */
            virtual ~ILoggingSink() = default;
            /**
            * @brief Logging function called by the logging service.
            *
            * @param[in] log The log message with description, severity level, timestamp, logger/domain name and participant name
            *
            * @return A standard result code to tell the logging service if the logging was succesful
            */
            virtual fep3::Result log(arya::LogMessage log) const = 0;
        };

    public:
        /// the fep component interface identifier for the ILoggingService
        FEP_COMPONENT_IID("logging_service.arya.fep3.iid");

        /**
         * @brief Creates a logger object that can be used to emit logs.
         *
         * @param[in] logger_name Name of the logger domain. Used to know from where the log is coming from and for configuration.
         *
         * @return Shared pointer to the created logger object.
         */
        virtual std::shared_ptr<arya::ILogger> createLogger(const std::string& logger_name) = 0;

        /**
         * @brief Registers a logging sink with the given name.
         *
         * @param[in] name Name of the sink (must be unique)
         * @param[in] sink the shared pointer of the sink
         *             the sink pointer must live until the unregister
         *             was called or the logging service is destroyed
         *
         * @return fep3::Result
         * @retval ERR_IN_USE a sink with the name already exists
         * @retval ERR_NO_ERROR success
         */
        virtual fep3::Result registerSink(const std::string& name,
                                          const std::shared_ptr<ILoggingSink>& sink) = 0;
        /**
         * @brief Unregisters a logging sink with the given name.
         *
         * @param[in] name Name of the sink (must be unique and exists)
         *
         * @return fep3::Result
         * @retval ERR_NOT_FOUND the sink with the name does not exists
         * @retval ERR_NO_ERROR success
         */
        virtual fep3::Result unregisterSink(const std::string& name) = 0;

    };
} // namespace arya
using arya::ILoggingService;
} // namespace fep3
