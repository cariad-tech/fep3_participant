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


add_executable(test_sim_bus tester_sim_bus.cpp)
set_target_properties(test_sim_bus PROPERTIES FOLDER "test/private/native_components")
target_link_libraries(test_sim_bus PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
    participant_test_utils
)
target_include_directories(test_sim_bus PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME test_sim_bus COMMAND test_sim_bus WORKING_DIRECTORY "..")
set_target_properties(test_sim_bus PROPERTIES TIMEOUT 10)



