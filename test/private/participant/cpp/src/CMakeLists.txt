# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
internal_fep3_participant_create_copy_target(private_participant_cpp)
set_target_properties(fep_participant_file_copy_private_participant_cpp PROPERTIES FOLDER "test/private/participant/cpp")

##################################################################
# tester_job
##################################################################

add_executable(tester_job tester_job.cpp tester_datajob.cpp)
gtest_discover_tests(tester_job
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

if(MSVC)
    set_source_files_properties(tester_job.cpp tester_datajob.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

target_link_libraries(tester_job PRIVATE
    GTest::gtest_main
    GTest::gmock
    ${CMAKE_DL_LIBS}
    participant_private_test_utils
    participant_test_utils
    fep3_participant_cpp
    fep3_component_registry
)
add_dependencies(tester_job fep_participant_file_copy_private_participant_cpp)
set_target_properties(tester_job PROPERTIES FOLDER "test/private/participant/cpp")
set_target_properties(tester_job PROPERTIES INSTALL_RPATH "$ORIGIN")

##################################################################
# tester_data_job_element
##################################################################

add_executable(tester_data_job_element tester_data_job_element.cpp)
gtest_discover_tests(tester_data_job_element
    TIMEOUT 10
)
target_link_libraries(tester_data_job_element PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_test_utils
    fep3_participant_cpp
    fep3_components_test
)
add_dependencies(tester_data_job_element fep_participant_file_copy_private_participant_cpp)
set_target_properties(tester_data_job_element PROPERTIES FOLDER "test/private/participant/cpp")
set_target_properties(tester_data_job_element PROPERTIES INSTALL_RPATH "$ORIGIN")
