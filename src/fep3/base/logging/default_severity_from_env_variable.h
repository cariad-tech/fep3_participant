/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */
#pragma once
#include <fep3/base/logging/logging_types.h>

#include <a_util/strings/strings_convert_decl.h>

#include <environment_variable.h>
#include <optional>

inline constexpr const char* fep3_default_logging_severity_env = "FEP3_DEFAULT_LOGGING_SEVERITY";

namespace fep3::base {
inline fep3::LoggerSeverity getDefaultLoggingSeverity()
{
    auto severity_from_env = fep3::environment_variable::get(fep3_default_logging_severity_env);
    fep3::LoggerSeverity default_severity(LoggerSeverity::info);
    if (severity_from_env) {
        int32_t parsed_severity(0);
        if (a_util::strings::toInt32(severity_from_env.value().c_str(), parsed_severity)) {
            default_severity = static_cast<fep3::LoggerSeverity>(parsed_severity);
        }
    }

    return default_severity;
}
} // namespace fep3::base
