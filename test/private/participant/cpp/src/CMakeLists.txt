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


##################################################################
# tester_job
##################################################################

add_executable(tester_job tester_job.cpp tester_datajob.cpp)
add_test(NAME tester_job
    COMMAND tester_job
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_job PRIVATE
    GTest::gtest_main
    ${CMAKE_DL_LIBS}
    participant_private_test_utils
    participant_test_utils
    fep3_participant_cpp
)
set_target_properties(tester_job PROPERTIES FOLDER "test/private/participant/cpp")
fep3_participant_deploy(tester_job)

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
set_target_properties(tester_data_reader_backlog PROPERTIES FOLDER "test/private/participant/cpp")
fep3_participant_deploy(tester_data_reader_backlog)

##################################################################
# tester_data_job_element
##################################################################

add_executable(tester_data_job_element tester_data_job_element.cpp)
add_test(NAME tester_data_job_element
    COMMAND tester_data_job_element
    TIMEOUT 10
)
target_link_libraries(tester_data_job_element PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_test_utils
    fep3_participant_cpp
)
set_target_properties(tester_data_job_element PROPERTIES FOLDER "test/private/participant/cpp")
fep3_participant_deploy(tester_data_job_element)