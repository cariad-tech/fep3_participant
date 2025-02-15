#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

internal_fep3_participant_create_copy_target(private_participant_core)
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
set_target_properties(tester_data_reader_backlog PROPERTIES 
    FOLDER "test/private/participant/core"
    INSTALL_RPATH "$ORIGIN"
)

##################################################################
# tester_data_reader
##################################################################

add_executable(tester_data_reader tester_data_reader.cpp)
add_test(NAME tester_data_reader
    COMMAND tester_data_reader
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_data_reader PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_core
    participant_test_utils
)
add_dependencies(tester_data_reader fep_participant_file_copy_private_participant_core)
set_target_properties(tester_data_reader PROPERTIES 
    FOLDER "test/private/participant/core"
    INSTALL_RPATH "$ORIGIN"
)

##################################################################
# tester_data_writer
##################################################################

add_executable(tester_data_writer tester_data_writer.cpp)
add_test(NAME tester_data_writer
    COMMAND tester_data_writer
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_data_writer PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_core
    participant_test_utils
    fep3_components_test
)
add_dependencies(tester_data_writer fep_participant_file_copy_private_participant_core)
set_target_properties(tester_data_writer PROPERTIES 
    FOLDER "test/private/participant/core"
    INSTALL_RPATH "$ORIGIN"
)

##################################################################
# tester_default_job_element
##################################################################

add_executable(tester_default_job_element tester_default_job_element.cpp)
add_test(NAME tester_default_job_element
    COMMAND tester_default_job_element
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_default_job_element PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_core
    participant_test_utils
    participant_private_test_utils
    fep3_components_test
)
add_dependencies(tester_default_job_element fep_participant_file_copy_private_participant_core)
set_target_properties(tester_default_job_element 
    PROPERTIES 
    FOLDER "test/private/participant/core"
    INSTALL_RPATH "$ORIGIN"
)

##################################################################
# tester_default_job
##################################################################

add_executable(tester_default_job tester_default_job.cpp)
add_test(NAME tester_default_job
    COMMAND tester_default_job
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_default_job PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_core
    participant_test_utils
    participant_private_test_utils
    fep3_components_test
)
add_dependencies(tester_default_job fep_participant_file_copy_private_participant_core)
set_target_properties(tester_default_job 
    PROPERTIES 
    FOLDER "test/private/participant/core"
    INSTALL_RPATH "$ORIGIN"
)

##################################################################
# tester_custom_element_factory
##################################################################

add_executable(tester_custom_element_factory tester_custom_element_factory.cpp)
add_test(NAME tester_custom_element_factory
    COMMAND tester_custom_element_factory
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_custom_element_factory PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_core
    participant_test_utils
    participant_private_test_utils
    fep3_components_test
)
add_dependencies(tester_custom_element_factory fep_participant_file_copy_private_participant_core)
set_target_properties(tester_custom_element_factory
    PROPERTIES 
    FOLDER "test/private/participant/core"
    INSTALL_RPATH "$ORIGIN"
)
