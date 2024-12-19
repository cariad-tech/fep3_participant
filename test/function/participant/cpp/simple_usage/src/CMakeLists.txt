# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_executable(test_simple_usage_cpp tester_intf_cpp_participant.cpp)
target_link_libraries(test_simple_usage_cpp PRIVATE GTest::gtest_main)
set_target_properties(test_simple_usage_cpp PROPERTIES FOLDER "test/function/participant/cpp")

target_link_libraries(test_simple_usage_cpp PRIVATE fep3_participant_cpp fep_test_helper)
internal_fep3_participant_deploy(test_simple_usage_cpp)
gtest_discover_tests(test_simple_usage_cpp
    TIMEOUT 20
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
)
