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


add_executable(test_component_registry tester_component_registry.cpp)

add_test(NAME test_component_registry
    COMMAND test_component_registry
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(test_component_registry PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_test_utils
    fep3_participant_private_lib
)

set_target_properties(test_component_registry PROPERTIES FOLDER "test/private/component_registry")

