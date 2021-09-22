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
// @attention Changes in this file must be reflected in the corresponding C++ interface file property_node_intf.h

#pragma once

#include <stddef.h>

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/c_intf/fep3_result_c_intf.h>
#include <fep3/plugin/c/c_intf/destruction_manager_c_intf.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IPropertyNode
typedef struct fep3_arya_OIPropertyNode* fep3_arya_HIPropertyNode;
/// Handle to const @ref fep3::arya::IPropertyNode
typedef struct fep3_arya_OIPropertyNode const* fep3_arya_const_HIPropertyNode;

// forward declaration for usage in fep3_arya_SIPropertyNode
struct fep3_arya_const_SIPropertyNode;
/// Access structure for @ref fep3::arya::IPropertyNode
typedef struct fep3_arya_SIPropertyNode
{
    /// Handle to the wrapped object
    fep3_arya_HIPropertyNode _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getName)
        (fep3_arya_HIPropertyNode, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getValue)
        (fep3_arya_HIPropertyNode, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getTypeName)
        (fep3_arya_HIPropertyNode, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *setValue)
        (fep3_arya_HIPropertyNode
        , fep3_result_callback_type result_callback
        , void* result_destination
        , const char* value
        , const char* type_name
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *isEqual)
        (fep3_arya_HIPropertyNode, bool*, fep3_arya_const_SIPropertyNode);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *reset)
        (fep3_arya_HIPropertyNode);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getChildren)
        (fep3_arya_HIPropertyNode
        , void(*)(void*, fep3_plugin_c_arya_SDestructionManager, fep3_arya_SIPropertyNode)
        , void*
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getNumberOfChildren)
        (fep3_arya_HIPropertyNode, size_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getChild)
        (fep3_arya_HIPropertyNode
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_SIPropertyNode*
        , const char* name
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *isChild)
        (fep3_arya_HIPropertyNode, bool*, const char* name);
    /// @endcond no_documentation
} fep3_arya_SIPropertyNode;

/// Access structure for const @ref fep3::arya::IPropertyNode
typedef struct fep3_arya_const_SIPropertyNode
{
    /// Handle to the wrapped object
    fep3_arya_const_HIPropertyNode _handle;
    // function pointers wrapping methods of the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getName)
        (fep3_arya_const_HIPropertyNode, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getValue)
        (fep3_arya_const_HIPropertyNode, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getTypeName)
        (fep3_arya_const_HIPropertyNode, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *isEqual)
        (fep3_arya_const_HIPropertyNode, bool*, fep3_arya_const_SIPropertyNode);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getChildren)
        (fep3_arya_const_HIPropertyNode
        , void(*)(void*, fep3_plugin_c_arya_SDestructionManager, fep3_arya_SIPropertyNode)
        , void*
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getNumberOfChildren)
        (fep3_arya_const_HIPropertyNode, size_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getChild)
        (fep3_arya_const_HIPropertyNode handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access
        , fep3_arya_SIPropertyNode* property_node_access
        , const char* name
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *isChild)
        (fep3_arya_const_HIPropertyNode, bool*, const char* name);
    /// @endcond no_documentation
} fep3_arya_const_SIPropertyNode;

#ifdef __cplusplus
}
#endif
