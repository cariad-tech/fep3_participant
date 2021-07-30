#
# Copyright @ 2021 VW Group. All rights reserved.
# 
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
# 
# You may add additional accurate notices of copyright ownership.
# 
#

option(fep3_participant_cmake_enable_functional_tests
       "Enable functional tests - requires googletest (default: OFF)" OFF)
option(fep3_participant_cmake_enable_private_tests
       "Enable private tests - requires googletest (default: OFF)" OFF)

if (NOT fep3_participant_cmake_integrated_tests)
    project(fep3-participant-tests)

    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD 14)

    # if Conan is used, we have to include the generated file and doing some basic setup
    if(CONAN_COMPILER)
        if ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
            include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
        elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
            include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
        elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
            include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
        elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo_multi.cmake)
            include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo_multi.cmake)
        else()
            message(FATAL_ERROR "Conan build info can't be found.")
        endif()

        if(CORTEX_WORKSPACE)
            conan_basic_setup(TARGETS)
        else()
            conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
        endif()
    endif()

    find_package(fep3_participant REQUIRED)
    find_package(fep3_participant_core REQUIRED)
    find_package(fep3_participant_cpp REQUIRED)
    enable_testing()
endif ()

find_package(Threads)
find_package(GTest CONFIG REQUIRED COMPONENTS gtest gtest_main gmock gmock_main)

include(GoogleTest)

if (WIN32 AND CMAKE_CL_64)
    add_definitions(-DWIN64)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (UNIX AND fep3_participant_cmake_integrated_tests AND CMAKE_BUILD_WITH_INSTALL_RPATH)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
endif()

if (fep3_participant_cmake_enable_private_tests)
    # build private tests
    add_subdirectory(private)
endif()

# always build utils of test
add_subdirectory(utils)

if (fep3_participant_cmake_enable_functional_tests)
    add_subdirectory(function)
endif()

if (UNIX AND fep3_participant_cmake_integrated_tests AND CMAKE_BUILD_WITH_INSTALL_RPATH)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()
