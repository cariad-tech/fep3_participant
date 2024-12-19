# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.



##################################################################
# tester_health_service
##################################################################

find_package(Boost REQUIRED)

add_executable(tester_health_service
    tester_health_service.cpp
    tester_job_health_registry.cpp
)

gtest_discover_tests(tester_health_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_health_service PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
    Boost::headers
)

set_target_properties(tester_health_service PROPERTIES FOLDER "test/private/native_components/health")

##################################################################
# tester_health_service_rpc
##################################################################

find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)

add_custom_target(tester_health_service_rpc_stub_generator)
fep_generate_rpc_stubs_before_target(
    TARGET tester_health_service_rpc_stub_generator
    INPUT_FILE "${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/health/health.json"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    CLIENT_CLASS_NAME test::rpc_stubs::TestHealthServiceProxy
    CLIENT_FILE_NAME test_health_proxy_stub.h
)
add_executable(tester_health_service_rpc tester_health_service_rpc.cpp)
add_dependencies(tester_health_service_rpc tester_health_service_rpc_stub_generator)

gtest_discover_tests(tester_health_service_rpc
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_health_service_rpc PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

target_include_directories(tester_health_service_rpc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_health_service_rpc PROPERTIES FOLDER "test/private/native_components/health")
