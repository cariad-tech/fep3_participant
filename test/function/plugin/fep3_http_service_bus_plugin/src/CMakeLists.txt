#
# Copyright @ 2022 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

set(FEP3_PARTICIPANT_PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR})

project(tester_fep3_http_service_bus_plugin)

add_executable(${PROJECT_NAME}
    tester_fep3_http_service_bus_plugin.cpp
    ${TEST_TEMPLATES_DIR}/fep_components_plugin_test.cpp
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/function/plugin/fep3_http_service_bus_plugin")
target_link_libraries(${PROJECT_NAME} PRIVATE
    fep3_participant_cpp_plugin
    fep3_components_test
    participant_test_utils
    GTest::gtest_main
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${FEP3_PARTICIPANT_PROJECT_BINARY_DIR}/include
    ${TEST_TEMPLATES_DIR}
)
if(MSVC)
else()
target_link_libraries(${PROJECT_NAME} PRIVATE
     pthread
     ${CMAKE_DL_LIBS})
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE
    FEP_COMPONENTS_PLUGIN_FILE_PATH="$<TARGET_FILE:fep3_http_service_bus>"
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES TIMEOUT 10)

internal_fep3_participant_deploy(${PROJECT_NAME})