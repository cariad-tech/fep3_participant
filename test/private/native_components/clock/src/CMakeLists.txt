# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


##################################################################
# Test of the native clock implementations
##################################################################

add_executable(tester_simulation_clock
               tester_simulation_clock.cpp
)


gtest_discover_tests(tester_simulation_clock
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_simulation_clock PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    participant_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

set_target_properties(tester_simulation_clock PROPERTIES FOLDER "test/private/native_components/clock/unit")

##################################################################
# Test of the SystemClock implementation
##################################################################

add_executable(tester_system_clock
               tester_system_clock.cpp
)

gtest_discover_tests(tester_system_clock
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_system_clock PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    participant_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

set_target_properties(tester_system_clock PROPERTIES FOLDER "test/private/native_components/clock/unit")

##################################################################
# Test of the native clock registry implementation
##################################################################
add_executable(tester_clock_registry
               tester_clock_registry.cpp
)

gtest_discover_tests(tester_clock_registry
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_clock_registry PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

set_target_properties(tester_clock_registry PROPERTIES FOLDER "test/private/native_components/clock/unit")

##################################################################
# Test of the native clock service implementation
##################################################################
add_executable(tester_clock_service
               tester_clock_service.cpp
)

gtest_discover_tests(tester_clock_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_clock_service PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
    participant_test_utils
    fep3_components_test
)

set_target_properties(tester_clock_service PROPERTIES FOLDER "test/private/native_components/clock")

##################################################################
# Test of the native clock service rpc implementation
##################################################################

find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)

add_executable(tester_rpc_clock_service
    tester_rpc_clock_service.cpp
)

gtest_discover_tests(tester_rpc_clock_service
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_rpc_clock_service PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
    fep3_components_test
)

target_include_directories(tester_rpc_clock_service PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_rpc_clock_service PROPERTIES FOLDER "test/private/native_components/clock")

##################################################################
# Test of the clock sync service rpc implementation
##################################################################

add_executable(tester_rpc_clock_sync_service
    tester_rpc_clock_sync_service.cpp
)

gtest_discover_tests(tester_rpc_clock_sync_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_rpc_clock_sync_service PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
    fep3_components_test
)

target_include_directories(tester_rpc_clock_sync_service PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_rpc_clock_sync_service PROPERTIES FOLDER "test/private/native_components/clock")

##################################################################
# Test of the clock sync client rpc implementation
##################################################################

add_executable(tester_rpc_clock_sync_client
    tester_rpc_clock_sync_client.cpp
)

gtest_discover_tests(tester_rpc_clock_sync_client
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_rpc_clock_sync_client PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
    fep3_components_test
)

target_include_directories(tester_rpc_clock_sync_client PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_rpc_clock_sync_client PROPERTIES FOLDER "test/private/native_components/clock")

##################################################################
# Test of the clock event sink registry
##################################################################

add_executable(tester_clock_event_sink_registry
    tester_clock_event_sink_registry.cpp
)

gtest_discover_tests(tester_clock_event_sink_registry
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_clock_event_sink_registry PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
)
set_target_properties(tester_clock_event_sink_registry PROPERTIES FOLDER "test/private/native_components/clock")
