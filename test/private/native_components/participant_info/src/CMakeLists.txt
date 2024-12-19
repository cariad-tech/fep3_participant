# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


##################################################################
# teser_job
##################################################################

add_executable(tester_participant_info tester_participant_info.cpp)

target_link_libraries(tester_participant_info PRIVATE GTest::gtest_main ${CMAKE_DL_LIBS} fep3_participant_core)
set_target_properties(tester_participant_info PROPERTIES FOLDER "test/private/native_components/participant_info")
internal_fep3_participant_deploy(tester_participant_info)
gtest_discover_tests(tester_participant_info
    TIMEOUT 10
)
