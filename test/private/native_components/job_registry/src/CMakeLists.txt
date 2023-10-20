#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


##################################################################
# tester_job_registry
##################################################################

find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)

add_executable(tester_job_registry_impl tester_job_registry_impl.cpp)

add_test(NAME tester_job_registry_impl
    COMMAND tester_job_registry_impl
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_job_registry_impl PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_participant_core
)
set_target_properties(tester_job_registry_impl PROPERTIES FOLDER "test/private/native_components/job_registry/unit")

##################################################################
# tester_job_configurations
##################################################################


add_executable(tester_job_configurations tester_job_configurations.cpp)

add_test(NAME tester_job_configurations
    COMMAND tester_job_configurations
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_job_configurations PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

target_compile_definitions(tester_job_configurations PRIVATE
    TEST_FILES_DIRECTORY="${fep3_participant_DIR}/test/private/native_components/job_registry/files/"
)

set_target_properties(tester_job_configurations PROPERTIES FOLDER "test/private/native_components/job_registry/unit")

##################################################################
# tester_job_registry_service
##################################################################


add_executable(tester_job_registry tester_job_registry.cpp)

add_test(NAME tester_job_registry
    COMMAND tester_job_registry
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_job_registry PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

target_compile_definitions(tester_job_registry PRIVATE
    TEST_FILES_DIRECTORY="${fep3_participant_DIR}/test/private/native_components/job_registry/files/"
)

set_target_properties(tester_job_registry PROPERTIES FOLDER "test/private/native_components/job_registry")

##################################################################
# Test of the native job registry rpc implementation
##################################################################

add_custom_target(tester_job_registry_rpc_stub_generator)
fep_generate_rpc_stubs_before_target(
    TARGET tester_job_registry_rpc_stub_generator
    INPUT_FILE "${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/job_registry/job_registry.json"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    CLIENT_CLASS_NAME test::rpc_stubs::TestJobRegistryClientStub
    CLIENT_FILE_NAME test_job_registry_client_stub.h
)
add_executable(tester_job_registry_rpc tester_job_registry_rpc.cpp)
add_dependencies(tester_job_registry_rpc tester_job_registry_rpc_stub_generator)

add_test(NAME tester_job_registry_rpc
    COMMAND tester_job_registry_rpc
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_job_registry_rpc PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
    fep3_components_test
)

target_include_directories(tester_job_registry_rpc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_job_registry_rpc PROPERTIES FOLDER "test/private/native_components/job_registry")
