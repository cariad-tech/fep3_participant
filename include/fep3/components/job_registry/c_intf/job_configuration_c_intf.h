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
// @attention Changes in this file must be reflected in the corresponding C++ interface file job_configuration.h

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Access structure for @ref fep3::arya::JobConfiguration
typedef struct
{
    /// @cond no_documentation
    int64_t _cycle_sim_time;
    int64_t _delay_sim_time;
    bool _max_runtime_real_time_validity;
    int64_t _max_runtime_real_time;
    int32_t _runtime_violation_strategy;
    /// @endcond no_documentation
} fep3_arya_SJobConfiguration;

#ifdef __cplusplus
}
#endif
