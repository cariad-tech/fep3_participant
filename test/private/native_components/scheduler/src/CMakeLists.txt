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
# tester_job_runner
##################################################################

find_package(dev_essential REQUIRED COMPONENTS pkg_rpc)

add_executable(tester_job_runner tester_job_runner.cpp)

add_test(NAME tester_job_runner
    COMMAND tester_job_runner
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_job_runner PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_job_runner PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_scheduler_registry
##################################################################


add_executable(tester_scheduler_registry tester_scheduler_registry.cpp)

add_test(NAME tester_scheduler_registry
    COMMAND tester_scheduler_registry
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_scheduler_registry PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_scheduler_registry PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_timer_scheduler
##################################################################


add_executable(tester_timer_scheduler tester_timer_scheduler.cpp)

add_test(NAME tester_timer_scheduler
    COMMAND tester_timer_scheduler
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_timer_scheduler PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_timer_scheduler PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_clock_based_scheduler
##################################################################


add_executable(tester_clock_based_scheduler tester_clock_based_scheduler.cpp)

add_test(NAME tester_clock_based_scheduler
    COMMAND tester_clock_based_scheduler
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_clock_based_scheduler PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    participant_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_clock_based_scheduler PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_scheduler_service
##################################################################


add_executable(tester_scheduler_service tester_scheduler_service.cpp)

add_test(NAME tester_scheduler_service
    COMMAND tester_scheduler_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_scheduler_service PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_scheduler_service PROPERTIES FOLDER "test/private/native_components/scheduler")

##################################################################
# Test of the native scheduler service rpc implementation
##################################################################

jsonrpc_generate_client_stub(${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/scheduler_service/scheduler_service.json
                             test::rpc_stubs::TestSchedulerServiceClientStub
                             ${CMAKE_CURRENT_BINARY_DIR}/test_scheduler_service_client_stub.h)

add_executable(tester_scheduler_service_rpc tester_scheduler_service_rpc.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/test_scheduler_service_client_stub.h
)

add_test(NAME tester_scheduler_service_rpc
    COMMAND tester_scheduler_service_rpc
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_scheduler_service_rpc PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
)

target_include_directories(tester_scheduler_service_rpc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_scheduler_service_rpc PROPERTIES FOLDER "test/private/native_components/scheduler")
