#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


##################################################################
#  the test itself
##################################################################

add_definitions(-DCURRENT_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../")

add_executable(test_components_configuration tester_components_configuration.cpp)

set_target_properties(test_components_configuration PROPERTIES FOLDER "test/private/components_configuration")
target_link_libraries(test_components_configuration PRIVATE
    fep3_participant_private_lib
    GTest::gtest_main
)

add_test(NAME test_components_configuration COMMAND test_components_configuration WORKING_DIRECTORY "..")
set_target_properties(test_components_configuration PROPERTIES TIMEOUT 10)