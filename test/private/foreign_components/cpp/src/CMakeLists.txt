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


find_package(dev_essential REQUIRED COMPONENTS pkg_rpc)

add_library(test_cpp_plugin_1 SHARED
            test_plugins/test_plugin_1.cpp
            test_plugins/test_component_a_intf.h
            test_plugins/test_component_b_intf.h)
set_target_properties(test_cpp_plugin_1 PROPERTIES FOLDER "test/private/foreign_components/cpp/test_plugins")
target_link_libraries(test_cpp_plugin_1 PRIVATE fep3_participant_cpp_plugin)
target_include_directories(test_cpp_plugin_1 PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

add_library(test_cpp_plugin_2 SHARED
            test_plugins/test_plugin_2.cpp
            test_plugins/test_component_b_intf.h)
set_target_properties(test_cpp_plugin_2 PROPERTIES FOLDER "test/private/foreign_components/cpp/test_plugins")
target_link_libraries(test_cpp_plugin_2 PRIVATE fep3_participant_cpp_plugin)

##################################################################
#  the test itself
##################################################################

project(tester_foreign_components_cpp_plugin)

add_executable(${PROJECT_NAME} tester_foreign_components.cpp)
add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::gtest_main
    fep3_participant_private_lib
    dev_essential::pkg_rpc
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/foreign_components/cpp")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PLUGIN_1="$<TARGET_FILE:test_cpp_plugin_1>"
    PLUGIN_2="$<TARGET_FILE:test_cpp_plugin_2>"
    TEST_BUILD_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
