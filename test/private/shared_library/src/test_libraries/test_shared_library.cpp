/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "test_dependee_shared_library.h"

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>

extern "C" {
FEP3_PLUGIN_EXPORT int FEP3_PLUGIN_CALL get1()
{
    return 1;
}

FEP3_PLUGIN_EXPORT int FEP3_PLUGIN_CALL get2FromDependeeLibrary()
{
    try {
        return Dependee::get2();
    }
    catch (...) {
        return -1;
    }
}
}
