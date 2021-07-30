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
// @attention Changes in this file must be reflected in the corresponding C++ interface file participant_info_intf.h

#pragma once

#include <fep3/plugin/base/fep3_plugin_export.h>
#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/plugin/c/c_intf/shared_binary_c_intf.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IParticipantInfo
typedef struct fep3_arya_OIParticipantInfo* fep3_arya_HIParticipantInfo;
/// Access structure for fep3::arya::IParticipantInfo
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIParticipantInfo _handle;
    /// Base class fep3::arya::IComponent
    fep3_arya_SIComponent _component;
    /// @cond no_documentation
    // function pointers wrapping the interface
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getName)(fep3_arya_HIParticipantInfo, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getSystemName)(fep3_arya_HIParticipantInfo, void(*)(void*, const char*), void*);
    /// @endcond no_documentation
} fep3_arya_SIParticipantInfo;

/** @brief Gets a participant info that implements the interface identified by @p iid and provides access to it via @p access_result
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
fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getParticipantInfo
    (fep3_arya_SIParticipantInfo* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    );

/// defines the symbol name of the function that creates a participant info
#define SYMBOL_fep3_plugin_c_arya_createParticipantInfo fep3_plugin_c_arya_createParticipantInfo

/** @brief Creates a participant info that implements the interface identified by \p iid and provides access to it via \p access_result
 *
 * @param[in,out] access_result Pointer to an access structure providing access to the created component;
 *                              if null, no object will be created and the parameter remains unchanged
 * @param[in] shared_binary_access Access structure to the shared binary the component will reside in
 * @param[in] iid IID of the component to be created
 * @return error code (if any)
 */
FEP3_PLUGIN_EXPORT fep3_plugin_c_InterfaceError FEP3_PLUGIN_CALL fep3_plugin_c_arya_createParticipantInfo
    (fep3_arya_SIParticipantInfo* access_result
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    );

#ifdef __cplusplus
}
#endif