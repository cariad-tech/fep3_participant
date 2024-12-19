/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "test_dependee_shared_library_export.h"

class TEST_DEPENDEE_SHARED_LIBRARY_EXPORT Dependee {
public:
    static int get2();
};