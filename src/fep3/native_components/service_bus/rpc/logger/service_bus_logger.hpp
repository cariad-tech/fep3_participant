/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <a_util/result/error_def.h>

namespace service_bus_helper {

#define SB_LOG_AND_RETURN_ERROR_DESCRIPTION(_errcode, ...)                                         \
    {                                                                                              \
        std::string message_to_log = a_util::strings::format(__VA_ARGS__);                         \
        _logger_proxy->logError(message_to_log);                                                   \
        RETURN_ERROR_DESCRIPTION(_errcode, message_to_log.c_str());                                \
    }

} // namespace service_bus_helper
