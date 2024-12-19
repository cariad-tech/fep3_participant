# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

project(tester_cpp_plugin_helpers)

add_executable(${PROJECT_NAME}
    tester_cpp_plugin_component_factory.cpp
    tester_cpp_plugin_component_wrapper.cpp
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/function/plugin/cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE
    fep3_participant_cpp_plugin
    fep3_components_test
    GTest::gtest_main
    GTest::gmock
)
gtest_discover_tests(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES TIMEOUT 10)