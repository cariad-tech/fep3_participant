# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


##################################################################
# tester_scheduling
##################################################################


add_executable(tester_scheduling tester_scheduling.cpp)

gtest_discover_tests(tester_scheduling
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_scheduling PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    participant_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

set_target_properties(tester_scheduling PROPERTIES FOLDER "test/private/native_components/scheduler/integration")