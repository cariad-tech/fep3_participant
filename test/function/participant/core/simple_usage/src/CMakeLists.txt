#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


add_executable(test_simple_usage_tooling tester_tooling_participant.cpp)
add_test(NAME test_simple_usage_tooling
    COMMAND test_simple_usage_tooling
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
)
target_link_libraries(test_simple_usage_tooling PRIVATE GTest::gtest_main)
set_target_properties(test_simple_usage_tooling PROPERTIES FOLDER "test/function/participant/core")

target_link_libraries(test_simple_usage_tooling PRIVATE fep3_participant_core)
internal_fep3_participant_deploy(test_simple_usage_tooling)

target_compile_options(test_simple_usage_tooling PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
                                                         $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>)

add_executable(test_default_job tester_default_job.cpp)
add_test(NAME test_default_job
    COMMAND test_default_job
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
)
target_link_libraries(test_default_job PRIVATE GTest::gtest_main)
set_target_properties(test_default_job PROPERTIES FOLDER "test/function/participant/core")

target_link_libraries(test_default_job PRIVATE fep3_participant_core)
internal_fep3_participant_deploy(test_default_job)

target_compile_options(test_default_job PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
                                                         $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>)
