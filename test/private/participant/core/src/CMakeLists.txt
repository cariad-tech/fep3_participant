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

fep3_participant_create_copy_target(private_participant_core)
set_target_properties(fep_participant_file_copy_private_participant_core PROPERTIES FOLDER "test/private/participant/core")

##################################################################
# tester_data_reader_backlog
##################################################################

add_executable(tester_data_reader_backlog tester_data_reader_backlog.cpp)
add_test(NAME tester_data_reader_backlog
    COMMAND tester_data_reader_backlog
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_data_reader_backlog PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_core
)
add_dependencies(tester_data_reader_backlog fep_participant_file_copy_private_participant_core)
set_target_properties(tester_data_reader_backlog PROPERTIES FOLDER "test/private/participant/core")
set_target_properties(tester_data_reader_backlog PROPERTIES INSTALL_RPATH "$ORIGIN")
