#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


##################################################################
# Command Line Interface Tester
##################################################################

add_executable(test_command_line_interface tester_command_line_interface.cpp)
add_test(NAME test_command_line_interface
    COMMAND test_command_line_interface
    TIMEOUT 40
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
)
target_link_libraries(test_command_line_interface PRIVATE GTest::gtest_main ${CMAKE_DL_LIBS} participant_private_test_utils fep3_participant_cpp)
set_target_properties(test_command_line_interface PROPERTIES FOLDER "test/private/participant/command_line_interface")
internal_fep3_participant_deploy(test_command_line_interface)

target_compile_options(test_command_line_interface PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
                                                           $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>)
