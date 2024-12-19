# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


add_library(faulty_test_plugin SHARED
            test_plugins/faulty_test_plugin.cpp
            test_plugins/faulty_test_plugin_intf.h)
set_target_properties(faulty_test_plugin PROPERTIES FOLDER "test/private/participant/core/test_plugins")
target_link_libraries(faulty_test_plugin PRIVATE fep3_participant_cpp_plugin)
target_include_directories(faulty_test_plugin PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

##################################################################
#  the test itself
##################################################################

project(tester_startup_error_handling)

add_executable(${PROJECT_NAME} tester_startup_error_handling.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/function/participant/core")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    FAULTY_TEST_PLUGIN="$<TARGET_FILE:faulty_test_plugin>"
    TEST_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    fep3_participant_core
    participant_test_utils
    GTest::gtest_main
)
internal_fep3_participant_deploy(${PROJECT_NAME})

# copy the plugins and component configuration files
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_copy_files)
add_custom_target(${PROJECT_NAME}_copy_files)
add_custom_command(TARGET ${PROJECT_NAME}_copy_files
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:faulty_test_plugin> ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:faulty_test_plugin>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../files ${CMAKE_CURRENT_BINARY_DIR}/files/
)

gtest_discover_tests(${PROJECT_NAME}
    TIMEOUT 10
)
