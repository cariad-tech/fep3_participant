# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)

add_executable(test_distributed_scheduling_scenarios test_distributed_scheduling_scenarios.cpp)
set_target_properties(test_distributed_scheduling_scenarios PROPERTIES FOLDER "test/function/scenarios")
target_link_libraries(test_distributed_scheduling_scenarios PRIVATE
    fep3_participant_cpp
    participant_test_utils
    GTest::gtest_main
    GTest::gmock
    dev_essential::pkg_rpc
)

set_target_properties(test_distributed_scheduling_scenarios PROPERTIES TIMEOUT 10)

internal_fep3_participant_deploy(test_distributed_scheduling_scenarios)
gtest_discover_tests(test_distributed_scheduling_scenarios
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/")
