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
// @attention Changes in this file must be reflected in the corresponding C++ interface file data_sample_intf.h

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include "raw_memory_c_intf.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to data sample
typedef struct fep3_arya_OIDataSample const* fep3_arya_const_HIDataSample;

/// Access structure for @ref fep3::arya::IDataSample
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_const_HIDataSample _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getTime)(fep3_arya_const_HIDataSample, int64_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getSize)(fep3_arya_const_HIDataSample, size_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getCounter)(fep3_arya_const_HIDataSample, uint32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *read)(fep3_arya_const_HIDataSample, size_t*, fep3_arya_SIRawMemory);
    /// @endcond no_documentation
} fep3_arya_const_SIDataSample;

#ifdef __cplusplus
}
#endif
