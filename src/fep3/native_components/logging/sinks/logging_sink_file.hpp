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

#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/base/properties/properties.h>
#include "logging_sink_common.hpp"

#include <fstream>

#include <a_util/filesystem.h>
#include <a_util/concurrency/mutex.h>

namespace fep3
{
/**
 * @brief Implementation of the file logging. Can be used as a base class for a custom sink.
 *        Logs will be written to the file defined by the file_path property of this class.
 */
class LoggingSinkFile : public base::Properties<ILoggingService::ILoggingSink> {
public:
    LoggingSinkFile() : _log_file(nullptr)
    {
    }

    fep3::Result log(LogMessage log) const override
    {
        if (_log_file && static_cast<bool>(*_log_file))
        {
            std::string log_msg;
            native::formatLoggingString(log_msg, log);

            std::unique_lock<std::mutex> guard(_file_mutex);
            *_log_file << log_msg << std::endl;
            if (_log_file->fail())
            {
                RETURN_ERROR_DESCRIPTION(ERR_DEVICE_IO, "Failed to write log into file");
            }
            else
            {
                _log_file->flush();
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_BAD_DEVICE, "Unable to write log to file: Output file stream is in an error state!");
        }
        return{};
    }

    bool setProperty(const std::string& name, const std::string& value, const std::string& type) override
    {
        if (name == "file_path")
        {
            std::unique_lock<std::mutex> guard(_file_mutex);
            _log_file.reset(new std::ofstream());
            a_util::filesystem::Path path = a_util::filesystem::Path(value); // Normalize path string
            if (path.isEmpty())
            {
                throw std::runtime_error("File path for file logger is empty.");
            }

            if (_log_file->is_open())
            {
                _log_file->close();
            }

            std::fstream::openmode mode = std::fstream::out;
            if (a_util::filesystem::exists(path))
            {
                mode |= std::fstream::app;
            }

            _log_file->open(path.toString().c_str(), mode);

            if (_log_file->fail())
            {
                throw std::runtime_error(std::string("Unable to open log file ") + value);
            }
        }
        return Properties<ILoggingSink>::setProperty(name, value, type);
    }



protected:
    /// Output file stream opened during configuration to improve runtime performance
    std::unique_ptr<std::ofstream> _log_file;
    mutable a_util::concurrency::mutex _file_mutex;

};
} // namespace fep3
