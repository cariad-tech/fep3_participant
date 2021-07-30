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
// @attention Changes in this file must be reflected in the corresponding C++ interface file job_intf.h

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/c_intf/fep3_result_c_intf.h>
#include <fep3/plugin/c/c_intf/destruction_manager_c_intf.h>
// C interface dependencies
#include "job_info_c_intf.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IJob
typedef struct fep3_arya_OIJob* fep3_arya_HIJob;
/// Access structure for @ref fep3::arya::IJob
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIJob _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *executeDataIn)
        (fep3_arya_HIJob
        , fep3_result_callback_type result_callback
        , void* result_destination
        , int64_t
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *execute)
        (fep3_arya_HIJob
        , fep3_result_callback_type result_callback
        , void* result_destination
        , int64_t
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *executeDataOut)
        (fep3_arya_HIJob
        , fep3_result_callback_type result_callback
        , void* result_destination
        , int64_t
        );
    /// @endcond no_documentation
} fep3_arya_SIJob;

/// Handle to @ref fep3::arya::JobEntry
typedef struct fep3_arya_OJobEntry* fep3_arya_HJobEntry;
/// Access structure for @ref fep3::arya::JobEntry
typedef struct
{
    /// @cond no_documentation
    fep3_plugin_c_arya_SDestructionManager _job_reference_manager;
    fep3_arya_SIJob _job;
    fep3_arya_SJobInfo _job_info;
    /// @endcond no_documentation
} fep3_arya_SJobEntry;

#ifdef __cplusplus
}
#endif
