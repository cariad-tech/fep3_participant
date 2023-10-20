#
# Copyright @ 2022 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

if(fep3_components_test_FOUND)
    return()
endif()

# gtest and gmock is required for target fep3_components_test
find_package(GTest CONFIG REQUIRED COMPONENTS gtest gtest_main gmock gmock_main)

include(${CMAKE_CURRENT_LIST_DIR}/lib/cmake/fep3_components_test_targets.cmake)

set(fep3_components_test_FOUND true)
