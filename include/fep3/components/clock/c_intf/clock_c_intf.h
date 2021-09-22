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
// @attention Changes in this file must be reflected in the corresponding C++ interface file clock_intf.h

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/c/c_intf/destruction_manager_c_intf.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IClock::IEventSink
typedef struct fep3_arya_IClock_OIEventSink* fep3_arya_IClock_HIEventSink;
/// Access structure for fep3::arya::IClock::IEventSink
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_IClock_HIEventSink _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *timeUpdateBegin)(fep3_arya_IClock_HIEventSink, int64_t, int64_t);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *timeUpdating)(fep3_arya_IClock_HIEventSink, int64_t);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *timeUpdateEnd)(fep3_arya_IClock_HIEventSink, int64_t);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *timeResetBegin)(fep3_arya_IClock_HIEventSink, int64_t, int64_t);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *timeResetEnd)(fep3_arya_IClock_HIEventSink, int64_t);
    /// @endcond no_documentation
} fep3_arya_IClock_SIEventSink;

/// Handle to @ref fep3::arya::IClock
typedef struct fep3_arya_OIClock* fep3_arya_HIClock;
/// Access structure for @ref fep3::arya::IClock
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIClock _handle;
    // function pointers wrapping the interface IClock
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getName)(fep3_arya_HIClock, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getType)(fep3_arya_HIClock, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getTime)(fep3_arya_HIClock, int64_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *reset)(fep3_arya_HIClock, int64_t);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *start)(fep3_arya_HIClock, fep3_plugin_c_arya_SDestructionManager*, fep3_arya_IClock_SIEventSink);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *stop)(fep3_arya_HIClock);
    /// @endcond no_documentation
} fep3_arya_SIClock;

#ifdef __cplusplus
}
#endif
