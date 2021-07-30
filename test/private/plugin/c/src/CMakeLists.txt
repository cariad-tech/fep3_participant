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


set(TEST_C_PLUGIN_1_TARGET test_c_plugin_1)

set(BUILD_INTERFACE_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    )

add_library(${TEST_C_PLUGIN_1_TARGET} SHARED
    test_plugins/plugin_1/test_plugin_1.cpp
    test_plugins/plugin_1/class_a.cpp
    test_plugins/plugin_1/class_a.h
    test_plugins/plugin_1/class_a_intf.h
    test_plugins/plugin_1/class_a_c_intf.h
    test_plugins/plugin_1/class_a_c_access_wrapper.cpp
    test_plugins/plugin_1/class_a_c_access_wrapper.h
    )
target_include_directories(${TEST_C_PLUGIN_1_TARGET} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
    )
set_target_properties(${TEST_C_PLUGIN_1_TARGET} PROPERTIES FOLDER "test/private/plugin/c/test_plugins")

##################################################################
#  test application
##################################################################

project(tester_c_plugin)

add_executable(${PROJECT_NAME}
    tester_c_plugin.cpp
    )
target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::gtest_main
    fep3_participant_private_lib
    )
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} WORKING_DIRECTORY "..")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/plugin/c")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PLUGIN_1="$<TARGET_FILE:${TEST_C_PLUGIN_1_TARGET}>"
    TEST_BUILD_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
