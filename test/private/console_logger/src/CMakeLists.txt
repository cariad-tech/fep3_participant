# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


project(tester_console_logger)

add_executable(${PROJECT_NAME} tester_console_logger.cpp)

gtest_discover_tests(${PROJECT_NAME}
    TIMEOUT 10
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::gtest_main
    fep3_participant_private_lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/console_logger")