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


find_package(dev_essential REQUIRED COMPONENTS filesystem)

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
add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/function/participant/core")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    FAULTY_TEST_PLUGIN="$<TARGET_FILE:faulty_test_plugin>"
    TEST_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    fep3_participant_core
    participant_test_utils
    GTest::gtest_main
    dev_essential::filesystem
)
fep3_participant_deploy(${PROJECT_NAME})

# copy the plugins and component configuration files
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_copy_files)
add_custom_target(${PROJECT_NAME}_copy_files)
add_custom_command(TARGET ${PROJECT_NAME}_copy_files
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:faulty_test_plugin> ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:faulty_test_plugin>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../files ${CMAKE_CURRENT_BINARY_DIR}/files/
)


