/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "logging_formater_intf.hpp"

#include <fep3/base/properties/properties.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/fep3_errors.h>
#include <fep3/fep3_filesystem.h>

#include <fstream>
#include <mutex>

namespace fep3 {
namespace native {

/**
 * @brief Implementation of the file logging. Can be used as a base class for a custom sink.
 *        Logs will be written to the file defined by the file_path property of this class.
 */
class LoggingSinkFileBase : public base::Properties<ILoggingService::ILoggingSink> {
public:
    LoggingSinkFileBase(std::unique_ptr<ILoggingFormater> logging_formater)
        : _logging_formater(std::move(logging_formater)), _log_file(nullptr)
    {
    }

    ~LoggingSinkFileBase()
    {
        if (_log_file) {
            _logging_formater->StreamEnd(*_log_file);
        }
    }

    // defining the destructor deactivates move semanticss
    LoggingSinkFileBase(LoggingSinkFileBase&&) = default;
    LoggingSinkFileBase& operator=(LoggingSinkFileBase&& other) = default;

    bool setProperty(const std::string& name,
                     const std::string& value,
                     const std::string& type) override
    {
        if (name == "file_path") {
            bool logExists;
            // scope here because initialize calls log which locks also.
            {
                std::unique_lock<std::mutex> guard(_file_mutex);
                _log_file = std::make_unique<std::fstream>();
                auto path = fs::path(value); // Normalize path string
                if (path.empty()) {
                    throw std::runtime_error("File path for file logger is empty.");
                }

                if (_log_file->is_open()) {
                    _log_file->close();
                }

                std::fstream::openmode mode = std::fstream::in | std::fstream::out;
                logExists = fs::exists(path);
                if (logExists) {
                    mode |= std::fstream::ate;
                }
                else {
                    mode |= std::fstream::trunc;
                }
                _log_file->open(path.string().c_str(), mode);

                if (_log_file->fail()) {
                    throw std::runtime_error(std::string("Unable to open log file ") + value);
                }
            }

            return initialize(logExists) &&
                   Properties<ILoggingSink>::setProperty(name, value, type);
        }
        return Properties<ILoggingSink>::setProperty(name, value, type);
    }

    fep3::Result log(LogMessage log) const override final
    {
        auto formated_string = _logging_formater->formatLogMessage(log);
        formated_string.append("\n");
        return logMessage(formated_string);
    }

private:
    bool initialize(bool logExists)
    {
        if (!_log_file) {
            return false;
        }

        if (logExists && !_logging_formater->IsStreamAppendable((*_log_file))) {
            logMessage("\nLog file is corrupt, appending will result in invalid logging format\n");
            logMessage(_logging_formater->GetStreamBegin());
            return false;
        }
        return logMessage(_logging_formater->GetStreamBegin()) == a_util::result::SUCCESS;
    }

    fep3::Result logMessage(const std::string& log_message) const
    {
        if (_log_file && static_cast<bool>(*_log_file)) {
            std::unique_lock<std::mutex> guard(_file_mutex);

            *_log_file << log_message;

            if (_log_file->fail()) {
                RETURN_ERROR_DESCRIPTION(ERR_DEVICE_IO, "Failed to write log into file");
            }
            else {
                _log_file->flush();
            }
            return {};
        }
        else {
            RETURN_ERROR_DESCRIPTION(
                ERR_BAD_DEVICE,
                "Unable to write log to file: Output file stream is in an error state!");
        }
    }

protected:
    /// Output file stream opened during configuration to improve runtime performance
    std::unique_ptr<std::fstream> _log_file;
    mutable std::mutex _file_mutex;
    std::unique_ptr<ILoggingFormater> _logging_formater;
};

} // namespace native
} // namespace fep3
