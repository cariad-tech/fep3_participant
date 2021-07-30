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

// @attention Changes in this file must be reflected in the corresponding C++ interface file properties_intf.h

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IProperties
typedef struct fep3_arya_OIProperties* fep3_arya_HIProperties;
/// Handle to const @ref fep3::arya::IProperties
typedef struct fep3_arya_OIProperties const* fep3_arya_const_HIProperties;

// forward declaration for usage in fep3_arya_SIProperties
struct fep3_arya_const_SIProperties;
/// Access structure for @ref fep3::arya::IProperties
typedef struct fep3_arya_SIProperties
{
    /// Handle to the wrapped object
    fep3_arya_HIProperties _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *setProperty)
        (fep3_arya_HIProperties
        , bool* // result
        , const char* // name
        , const char* // value
        , const char* // type
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getProperty)
        (fep3_arya_HIProperties, void(*)(void*, const char*), void*, const char*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getPropertyType)
        (fep3_arya_HIProperties, void(*)(void*, const char*), void*, const char*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *isEqual)
        (fep3_arya_HIProperties, bool*, fep3_arya_const_SIProperties);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *copyTo)
        (fep3_arya_HIProperties, fep3_arya_SIProperties);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getPropertyNames)
        (fep3_arya_HIProperties, void(*)(void*, const char*), void*);
    /// @endcond no_documentation
} fep3_arya_SIProperties;

/// Access structure for const @ref fep3::arya::IProperties
typedef struct fep3_arya_const_SIProperties
{
    /// Handle to the wrapped object
    fep3_arya_const_HIProperties _handle;
    // function pointers wrapping methods of the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getProperty)
        (fep3_arya_const_HIProperties, void(*)(void*, const char*), void*, const char*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getPropertyType)
        (fep3_arya_const_HIProperties, void(*)(void*, const char*), void*, const char*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *isEqual)
        (fep3_arya_const_HIProperties, bool*, fep3_arya_const_SIProperties);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *copyTo)
        (fep3_arya_const_HIProperties, fep3_arya_SIProperties);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getPropertyNames)
        (fep3_arya_const_HIProperties, void(*)(void*, const char*), void*);
    /// @endcond no_documentation
} fep3_arya_const_SIProperties;

#ifdef __cplusplus
}
#endif
