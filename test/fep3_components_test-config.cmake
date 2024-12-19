# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

if(fep3_components_test_FOUND)
    return()
endif()

# gtest and gmock is required for target fep3_components_test
find_package(GTest)
if(TARGET GTest::gtest_main)
    message(STATUS "Found GTest")
    if(TARGET GTest::gmock)
        message(STATUS "Found GMock")
    else()
        message(STATUS "Could NOT find GMock")
    endif()
else()
    message(STATUS "Could NOT find GTest")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/lib/cmake/fep3_components_test_targets.cmake)

set(fep3_components_test_FOUND true)
