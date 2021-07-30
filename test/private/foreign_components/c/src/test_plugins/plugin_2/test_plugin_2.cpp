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


#include "test_plugin_2.h"

void fep3_plugin_getPluginVersion
    (void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "test plugin 2, version 0.0.1");
}

::test_plugin_2::mock::MockComponentC* g_mock_component_c = nullptr;

void setMockComponentC(::test_plugin_2::mock::MockComponentC* mock_component_c)
{
    g_mock_component_c = mock_component_c;
}