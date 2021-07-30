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
# Test of the native clock implementations
##################################################################

find_package(dev_essential REQUIRED COMPONENTS pkg_rpc)

add_executable(tester_clock_system
               tester_clock_system.cpp
               tester_clock_system_discrete.cpp
)

add_test(NAME tester_clock_system
    COMMAND tester_clock_system
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_clock_system PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_clock_system PROPERTIES FOLDER "test/private/native_components/clock/unit")

##################################################################
# Test of the native clock registry implementation
##################################################################
add_executable(tester_clock_registry
               tester_clock_registry.cpp
)

add_test(NAME tester_clock_registry
    COMMAND tester_clock_registry
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_clock_registry PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_clock_registry PROPERTIES FOLDER "test/private/native_components/clock/unit")

##################################################################
# Test of the native clock service implementation
##################################################################
add_executable(tester_clock_service
               tester_clock_service.cpp
)

add_test(NAME tester_clock_service
    COMMAND tester_clock_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_clock_service PRIVATE
    GTest::gtest_main
    participant_private_test_utils
    fep3_participant_private_lib
    participant_test_utils
)

set_target_properties(tester_clock_service PROPERTIES FOLDER "test/private/native_components/clock")

##################################################################
# Test of the native clock service rpc implementation
##################################################################

find_package(dev_essential REQUIRED COMPONENTS pkg_rpc)

jsonrpc_generate_client_stub(${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/clock/clock.json
                             test::rpc_stubs::TestClockServiceProxy
                             ${CMAKE_CURRENT_BINARY_DIR}/test_clock_service_proxy_stub.h)

add_executable(tester_clock_service_rpc tester_clock_service_rpc.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/test_clock_service_proxy_stub.h
)

add_test(NAME tester_clock_service_rpc
    COMMAND tester_clock_service_rpc
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_clock_service_rpc PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
)

target_include_directories(tester_clock_service_rpc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_clock_service_rpc PROPERTIES FOLDER "test/private/native_components/clock")
