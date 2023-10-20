/**
* @file
* @copyright
* @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#ifdef __has_include
    #if __has_include(<filesystem>)
        #include <filesystem>
namespace fs = std::filesystem;
    #else
        #include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
    #endif
#else
    #error __has_include is not defined, but is needed to detect correct filesystem!
#endif