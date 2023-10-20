#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
internal_fep3_participant_create_copy_target(native_components_base)
set_target_properties(fep_participant_file_copy_native_components_base PROPERTIES FOLDER "test/function/native_components/base")

add_executable(test_data_sample tester_data_sample.cpp)
set_target_properties(test_data_sample PROPERTIES FOLDER "test/function/native_components/base")
target_link_libraries(test_data_sample PRIVATE fep3_participant
                                               GTest::gtest_main)
add_dependencies(test_data_sample fep_participant_file_copy_native_components_base)
add_test(NAME test_data_sample COMMAND test_data_sample WORKING_DIRECTORY "..")
set_target_properties(test_data_sample PROPERTIES TIMEOUT 10)
set_target_properties(test_data_sample PROPERTIES INSTALL_RPATH "$ORIGIN")

##################################################################
##################################################################

add_executable(test_stream_type tester_stream_type.cpp)
set_target_properties(test_stream_type PROPERTIES FOLDER "test/function/native_components/base")
target_link_libraries(test_stream_type PRIVATE fep3_participant
                                               GTest::gtest_main)
add_dependencies(test_stream_type fep_participant_file_copy_native_components_base)
add_test(NAME test_stream_type COMMAND test_stream_type WORKING_DIRECTORY "..")
set_target_properties(test_stream_type PROPERTIES TIMEOUT 10)
set_target_properties(test_stream_type PROPERTIES INSTALL_RPATH "$ORIGIN")
