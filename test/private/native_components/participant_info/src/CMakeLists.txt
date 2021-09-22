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


##################################################################
# teser_job
##################################################################

add_executable(tester_participant_info tester_participant_info.cpp)
add_test(NAME tester_participant_info
    COMMAND tester_participant_info
    TIMEOUT 10
)
target_link_libraries(tester_participant_info PRIVATE GTest::gtest_main ${CMAKE_DL_LIBS} fep3_participant_core)
set_target_properties(tester_participant_info PROPERTIES FOLDER "test/private/native_components/participant_info")
fep3_participant_deploy(tester_participant_info)
