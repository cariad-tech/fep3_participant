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


find_package(dev_essential REQUIRED COMPONENTS pkg_rpc)

add_executable(test_distributed_scheduling_scenarios test_distributed_scheduling_scenarios.cpp)
set_target_properties(test_distributed_scheduling_scenarios PROPERTIES FOLDER "test/function/scenarios")
target_link_libraries(test_distributed_scheduling_scenarios PRIVATE
    fep3_participant_cpp
    participant_test_utils
    GTest::gtest_main
    GTest::gmock
    dev_essential::pkg_rpc
)
add_test(NAME test_distributed_scheduling_scenarios COMMAND test_distributed_scheduling_scenarios WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/")
set_target_properties(test_distributed_scheduling_scenarios PROPERTIES TIMEOUT 10)

fep3_participant_deploy(test_distributed_scheduling_scenarios)
