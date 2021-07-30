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
extern "C"
{
#endif

/// Handle to destructor
typedef struct fep3_plugin_c_arya_ODestructionManager* fep3_plugin_c_arya_HDestructionManager;

/// Access structure for @ref fep3::plugin::c::arya::DestructionManager
typedef struct
{
    /// Handle to the wrapped object
    fep3_plugin_c_arya_HDestructionManager _handle;
    // function pointers wrapping methods of wrapped class
    /// @cond no_documentation
    void (FEP3_PLUGIN_CALL *destroy)(fep3_plugin_c_arya_HDestructionManager);
    /// @endcond no_documentation
} fep3_plugin_c_arya_SDestructionManager;

#ifdef __cplusplus
}
#endif