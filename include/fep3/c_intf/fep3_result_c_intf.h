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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Data structure for fep3::Result
typedef struct
{
    /// @cond no_documentation
    int32_t _error_code;
    int32_t _line;
    const char* _error_description;
    const char* _file;
    const char* _function;
    /// @endcond no_documentation
} fep3_SResult;

/// Typedef for callback taking a @ref fep3_SResult
typedef void(*fep3_result_callback_type)(void* destination, fep3_SResult result);

#ifdef __cplusplus
}
#endif