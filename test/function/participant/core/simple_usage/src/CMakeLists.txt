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


add_executable(test_simple_usage_tooling tester_tooling_participant.cpp)
add_test(NAME test_simple_usage_tooling
    COMMAND test_simple_usage_tooling
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
)
target_link_libraries(test_simple_usage_tooling PRIVATE GTest::gtest_main)
set_target_properties(test_simple_usage_tooling PROPERTIES FOLDER "test/function/participant/core")

target_link_libraries(test_simple_usage_tooling PRIVATE fep3_participant_core)
fep3_participant_deploy(test_simple_usage_tooling)
