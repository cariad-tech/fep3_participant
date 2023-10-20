#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


##################################################################
# Test of the interpolation time implementation
##################################################################
add_executable(tester_interpolation_time
               tester_interpolation_time.cpp
)

add_test(NAME tester_interpolation_time
    COMMAND tester_interpolation_time
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_interpolation_time PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
)

set_target_properties(tester_interpolation_time PROPERTIES FOLDER "test/private/native_components/clock_sync/unit")

##################################################################
# Test of the native clock sync service implementation
##################################################################
add_executable(tester_clock_sync_service
               tester_clock_sync_service.cpp
)

add_test(NAME tester_clock_sync_service
    COMMAND tester_clock_sync_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_clock_sync_service PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
)

set_target_properties(tester_clock_sync_service PROPERTIES FOLDER "test/private/native_components/clock_sync/unit")

##################################################################
# Test of the clock sync client implementation
##################################################################
add_executable(tester_clock_sync_client
               tester_clock_sync_client.cpp
               tester_system_clock_client_reset.cpp
)

add_test(NAME tester_clock_sync_client
    COMMAND tester_clock_sync_client
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_clock_sync_client PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    fep3_participant_private_lib
    participant_test_utils
    fep3_components_test
)

set_target_properties(tester_clock_sync_client PROPERTIES FOLDER "test/private/native_components/clock_sync/unit")

##################################################################
# Test of the clock sync master implementation
##################################################################
add_executable(tester_clock_sync_master
               tester_clock_sync_master.cpp
)

add_test(NAME tester_clock_sync_master
    COMMAND tester_clock_sync_master
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_clock_sync_master PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_participant_core
    fep3_components_test
)

set_target_properties(tester_clock_sync_master PROPERTIES FOLDER "test/private/native_components/clock_sync/unit")

##################################################################
# Integration test of the clock sync service
##################################################################
add_executable(tester_clock_synchronization
               tester_clock_synchronization.cpp
)

add_test(NAME tester_clock_synchronization
    COMMAND tester_clock_synchronization
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
set_tests_properties(tester_clock_synchronization PROPERTIES TIMEOUT 30)
target_link_libraries(tester_clock_synchronization PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_participant_core
    fep3_components_test
)

set_target_properties(tester_clock_synchronization PROPERTIES FOLDER "test/private/native_components/clock_sync")
