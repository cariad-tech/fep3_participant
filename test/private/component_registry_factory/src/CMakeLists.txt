#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


project(test_component_registry_factory)

add_executable(${PROJECT_NAME} tester_component_registry_factory.cpp)
add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    fep3_environment_variable
    GTest::gtest_main
    GTest::gmock_main
    fep3_participant_private_lib
    participant_test_utils
    fep3_components_test
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:stdc++fs> # needed to use experimental/filesystem
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/component_registry_factory")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    TEST_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    $<TARGET_PROPERTY:test_cpp_plugin_1,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:test_cpp_plugin_2,INTERFACE_INCLUDE_DIRECTORIES>
)

# copy the plugins and component configuration files
add_dependencies(${PROJECT_NAME} copy_files)
add_custom_target(copy_files)
add_custom_command(TARGET copy_files
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:test_cpp_plugin_1> ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:test_cpp_plugin_1>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:test_cpp_plugin_2> ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:test_cpp_plugin_2>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../files ${CMAKE_CURRENT_BINARY_DIR}/files/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep_components_plugin> ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep3_participant> ${CMAKE_CURRENT_BINARY_DIR}
)


file(RELATIVE_PATH COMPONENT_TEST_REL_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../files_in/test_rel_path.fep_components.in ${CMAKE_CURRENT_BINARY_DIR}/files/test_rel_path.fep_components)

if (MSVC)
    add_custom_command(TARGET copy_files
        COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Debug>>,copy_if_different,true>     $<TARGET_PDB_FILE:fep3_participant> ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Debug>>,copy_if_different,true>     $<TARGET_PDB_FILE:fep_components_plugin> ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

set_target_properties(copy_files PROPERTIES FOLDER "test/private/component_registry_factory")

set_source_files_properties(
        tester_component_registry_factory.cpp
        PROPERTIES COMPILE_DEFINITIONS $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)