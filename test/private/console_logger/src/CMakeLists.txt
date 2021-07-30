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


project(tester_console_logger)

add_executable(${PROJECT_NAME} tester_console_logger.cpp)

add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::gtest_main
    fep3_participant_private_lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/console_logger")