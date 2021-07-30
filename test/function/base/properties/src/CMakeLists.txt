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


set(TEST_NAME tester_properties)

add_executable(${TEST_NAME}
    tester_properties.cpp
    tester_properties_helper.cpp
)

set_target_properties(${TEST_NAME} PROPERTIES FOLDER "test/function/base/properties")
target_link_libraries(${TEST_NAME} PRIVATE
    fep3_participant
    participant_test_utils
    GTest::gtest_main
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
set_target_properties(${TEST_NAME} PROPERTIES TIMEOUT 10)

fep3_participant_deploy(${TEST_NAME})





