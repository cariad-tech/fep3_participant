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
// @attention Changes in this file must be reflected in the corresponding C++ interface file stream_type_intf.h

#pragma once

#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/base/properties/c_intf/properties_c_intf.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to stream type
typedef struct fep3_arya_OIStreamType const* fep3_arya_const_HIStreamType;

/// Access structure for @ref fep3::arya::IStreamType
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_const_HIStreamType _handle;
    /// base class fep3::arya::IProperties
    fep3_arya_const_SIProperties _properties;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getMetaTypeName)(fep3_arya_const_HIStreamType, void(*)(void*, const char*), void*);
    /// @endcond no_documentation
} fep3_arya_const_SIStreamType;

#ifdef __cplusplus
}
#endif
