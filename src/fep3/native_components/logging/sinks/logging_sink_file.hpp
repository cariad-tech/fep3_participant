/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "logging_formater_csv.hpp"
#include "logging_formater_json.hpp"
#include "logging_sink_file_base.hpp"

namespace fep3 {
namespace native {

/**
 * @brief Implementation of the file logging with the line oriented CSV format.
 */
class LoggingSinkFileCsv : public LoggingSinkFileBase {
public:
    LoggingSinkFileCsv() : LoggingSinkFileBase(std::make_unique<LoggingFormaterCsv>())
    {
    }
};

/**
 * @brief Implementation of the file logging with the line oriented JSON format.
 */
class LoggingSinkFileJson : public LoggingSinkFileBase {
public:
    LoggingSinkFileJson() : LoggingSinkFileBase(std::make_unique<LoggingFormaterJson>())
    {
    }
};

} // namespace native
} // namespace fep3
