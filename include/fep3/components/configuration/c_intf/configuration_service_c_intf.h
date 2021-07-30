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

#include <fep3/plugin/base/fep3_plugin_export.h>
#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/plugin/c/c_intf/shared_binary_c_intf.h>
#include "property_node_c_intf.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IConfigurationService
typedef struct fep3_arya_OIConfigurationService* fep3_arya_HIConfigurationService;
/// Access structure for fep3::arya::IConfigurationService
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIConfigurationService _handle;
    /// Base class fep3::arya::IComponent
    fep3_arya_SIComponent _component;
    /// @cond no_documentation
    // function pointers wrapping the interface
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *registerNode)
        (fep3_arya_HIConfigurationService handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        , fep3_plugin_c_arya_SDestructionManager destruction_manager_access
        , fep3_arya_SIPropertyNode property_node_access
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *unregisterNode)
        (fep3_arya_HIConfigurationService handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        , const char* name
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *isNodeRegistered)
        (fep3_arya_HIConfigurationService
        , bool* result
        , const char* path
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getNode)
        (fep3_arya_HIConfigurationService
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access
        , fep3_arya_SIPropertyNode* property_node_access
        , const char* path
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getConstNode)
        (fep3_arya_HIConfigurationService
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_const_SIPropertyNode*
        , const char* path
        );
    /// @endcond no_documentation
} fep3_arya_SIConfigurationService;

/** @brief Gets a Configuration Service that implements the interface identified by @p iid and provides access to it via @p access_result
 *
 * @param[in,out] access_result Pointer to an access structure providing access to the component to get;
 *                              if null, no object will be get and the parameter remains unchanged
 * @param[in] iid IID of the component to be created
 * @param[in] handle_to_component Handle to the interface of the component to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle_to_component is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while getting the component
 */
fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getConfigurationService
    (fep3_arya_SIConfigurationService* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    );

/// defines the symbol name of the function that creates a Configuration Service
#define SYMBOL_fep3_plugin_c_arya_createConfigurationService fep3_plugin_c_arya_createConfigurationService

/** @brief Creates a Configuration Service that implements the interface identified by \p iid and provides access to it via \p access_result
 *
 * @param[in,out] access_result Pointer to an access structure providing access to the created component;
 *                              if null, no object will be created and the parameter remains unchanged
 * @param[in] shared_binary_access Access structure to the shared binary the component will reside in
 * @param[in] iid IID of the component to be created
 * @return error code (if any)
 */
FEP3_PLUGIN_EXPORT fep3_plugin_c_InterfaceError FEP3_PLUGIN_CALL fep3_plugin_c_arya_createConfigurationService
    (fep3_arya_SIConfigurationService* access_result
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    );

#ifdef __cplusplus
}
#endif