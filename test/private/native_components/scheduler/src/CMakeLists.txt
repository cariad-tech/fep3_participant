#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


##################################################################
# tester_job_runner
##################################################################

find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)

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
    fep3_components_test
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
    fep3_components_test
)

set_target_properties(tester_scheduler_registry PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_timer_scheduler
##################################################################
find_package(Boost 1.73.0 REQUIRED COMPONENTS thread headers)

add_executable(tester_task_executor
    test_data_triggered_executor.cpp
    test_asynchronous_task_executor.cpp
    test_asynchronous_task_executor_invoker.cpp
    test_synchronous_task_executor.cpp
    test_common_task_executor.cpp
    test_synchronous_task_executor_invoker.cpp)

add_test(NAME tester_task_executor
    COMMAND tester_task_executor
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_task_executor PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
    participant_test_utils
    Boost::thread
    Boost::headers
)

set_target_properties(tester_task_executor PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

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
    fep3_components_test
    Boost::thread
    Boost::headers
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
    participant_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

set_target_properties(tester_scheduler_service PROPERTIES FOLDER "test/private/native_components/scheduler")

##################################################################
# Test of the native scheduler service rpc implementation
##################################################################

add_custom_target(tester_scheduler_service_rpc_stub_generator)
fep_generate_rpc_stubs_before_target(
    TARGET tester_scheduler_service_rpc_stub_generator
    INPUT_FILE "${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/scheduler_service/scheduler_service.json"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    CLIENT_CLASS_NAME test::rpc_stubs::TestSchedulerServiceClientStub
    CLIENT_FILE_NAME test_scheduler_service_client_stub.h
)
add_executable(tester_scheduler_service_rpc tester_scheduler_service_rpc.cpp)
add_dependencies(tester_scheduler_service_rpc tester_scheduler_service_rpc_stub_generator)

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
    fep3_components_test
)

target_include_directories(tester_scheduler_service_rpc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_scheduler_service_rpc PROPERTIES FOLDER "test/private/native_components/scheduler")
