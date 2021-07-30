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

#include <fep3/plugin/base/fep3_calling_convention.h>

#ifdef __cplusplus
extern "C" {
#endif

/// handle to shared binary
typedef struct fep3_plugin_c_arya_OISharedBinary* fep3_plugin_c_arya_HISharedBinary;

/// access structure to a shared binary
typedef struct
{
    /// Handle to the wrapped object
    fep3_plugin_c_arya_HISharedBinary _handle;
    /// @cond no_documentation
    void (FEP3_PLUGIN_CALL *destroy)(fep3_plugin_c_arya_HISharedBinary);
    /// @endcond no_documentation
} fep3_plugin_c_arya_SISharedBinary;

#ifdef __cplusplus
}
#endif
