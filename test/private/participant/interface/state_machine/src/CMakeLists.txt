# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


project(test_participant_state_machine)

add_executable(${PROJECT_NAME} tester_participant_state_machine.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
                           $<TARGET_PROPERTY:fep3_participant,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/participant/interface/state_machine")

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::gmock_main
    ${CMAKE_DL_LIBS}
    fep3_participant_private_lib
    participant_test_utils
)
gtest_discover_tests(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES TIMEOUT 10)



