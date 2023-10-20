#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)

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

# plugin 3 implements a FEP Super Component
add_library(test_cpp_plugin_3 SHARED
    test_plugins/test_plugin_3.cpp
    test_plugins/test_component_a_intf.h
    test_plugins/test_component_b_intf.h
)
set_target_properties(test_cpp_plugin_3 PROPERTIES FOLDER "test/private/foreign_components/cpp/test_plugins")
target_link_libraries(test_cpp_plugin_3 PRIVATE fep3_participant_cpp_plugin)

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
    PLUGIN_3="$<TARGET_FILE:test_cpp_plugin_3>"
    TEST_BUILD_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

set_source_files_properties(
        tester_foreign_components.cpp
        PROPERTIES COMPILE_DEFINITIONS $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)
