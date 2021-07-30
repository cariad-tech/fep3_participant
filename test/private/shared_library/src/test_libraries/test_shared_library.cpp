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


#include <string>

#include <fep3/plugin/base/fep3_plugin_export.h>
#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/shared_library.h>
#include "test_dependee_shared_library.h"

extern "C"
{
    FEP3_PLUGIN_EXPORT int FEP3_PLUGIN_CALL get1()
    {
        return 1;
    }

    FEP3_PLUGIN_EXPORT int FEP3_PLUGIN_CALL get2FromDependeeLibrary()
    {
        try
        {
            return Dependee::get2();
        }
        catch(...)
        {
            return -1;
        }
    }
}
