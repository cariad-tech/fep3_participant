# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

internal_fep3_participant_create_copy_target(rti_dds)
set_target_properties(fep_participant_file_copy_rti_dds PROPERTIES FOLDER "test/private/plugins")

include(../../../../../src/plugins/rti_dds/rti-macros.cmake)

changeHostSystemIfArm()
find_package(RTIConnextDDS 6.1.0 EXACT REQUIRED)
revertHostSystemIfArm()
find_package(dev_essential REQUIRED COMPONENTS memory)

##################################################################
# Test of StreamItemTopic
##################################################################

add_executable(test_connext_dds_stream_item_topic
    tester_dds_stream_item_topic.cpp
)

target_link_libraries(test_connext_dds_stream_item_topic PRIVATE
    GTest::gtest_main
    GTest::gmock
    participant_test_utils
    fep3_connext_dds_simulation_bus_object_lib
    fep3_participant_private_lib
)

target_include_directories(test_connext_dds_stream_item_topic PRIVATE
    ${PROJECT_SOURCE_DIR}/src/plugins/rti_dds/simulation_bus/stream_item_topic
)

set_target_properties(test_connext_dds_stream_item_topic PROPERTIES
    FOLDER "test/private/plugins/unit"
    INSTALL_RPATH "$ORIGIN"
)

target_compile_definitions(test_connext_dds_stream_item_topic PRIVATE
    TEST_FILES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../files/"
)

rti_deploy(test_connext_dds_stream_item_topic)

gtest_discover_tests(test_connext_dds_stream_item_topic
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
##################################################################
# Test of DDS Connext Plugin
##################################################################

add_executable(test_connext_dds_plugin tester_rti_common.cpp)
target_link_libraries(test_connext_dds_plugin PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant
    participant_test_utils
    participant_private_test_utils
)
set_target_properties(test_connext_dds_plugin PROPERTIES FOLDER "test/private/plugins")

target_link_libraries(test_connext_dds_plugin PRIVATE
    fep3_participant participant_test_utils
    participant_private_test_utils)

add_dependencies(test_connext_dds_plugin fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_plugin PROPERTIES INSTALL_RPATH "$ORIGIN")
gtest_discover_tests( test_connext_dds_plugin
    TIMEOUT 30
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
##################################################################
# Test of Connext DDS Simulation Bus
##################################################################

add_executable(test_connext_dds_simulation_bus
    tester_dds_simbus.cpp
    tester_dds_simbus_participant_name.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

target_link_libraries(test_connext_dds_simulation_bus PRIVATE
    GTest::gtest_main
    GTest::gmock_main
    RTIConnextDDS::cpp2_api
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
    participant_private_test_utils
    fep3_thread_utilities
)
set_target_properties(test_connext_dds_simulation_bus PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus PRIVATE
    FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>"
    TEST_FILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../files/"
)

add_dependencies(test_connext_dds_simulation_bus fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus PROPERTIES INSTALL_RPATH "$ORIGIN")
gtest_discover_tests( test_connext_dds_simulation_bus
    PROPERTIES TIMEOUT 80
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)

if(MSVC)
    # warning coming from dds headers
    target_compile_options(test_connext_dds_simulation_bus PRIVATE /wd4245 /wd4244 /wd4996)
endif()

##################################################################
# Test of Connext DDS Simulation Bus Domain ID
##################################################################

add_executable(test_connext_dds_simulation_bus_domain_id
    tester_dds_simbus_domain_id.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus_domain_id PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

gtest_discover_tests( test_connext_dds_simulation_bus_domain_id
    TIMEOUT 40
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_simulation_bus_domain_id PRIVATE
    GTest::gtest_main
    GTest::gmock_main
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
    participant_private_test_utils
)
set_target_properties(test_connext_dds_simulation_bus_domain_id PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_domain_id PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus_domain_id fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus_domain_id PROPERTIES INSTALL_RPATH "$ORIGIN")

##################################################################
# Test of Connext DDS Simulation Bus Late Jointer
##################################################################

add_executable(test_connext_dds_simulation_bus_late_joiner
    tester_dds_simbus_late_joiner.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus_late_joiner PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

target_link_libraries(test_connext_dds_simulation_bus_late_joiner PRIVATE
    GTest::gtest_main
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
    participant_private_test_utils
)
set_target_properties(test_connext_dds_simulation_bus_late_joiner PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_late_joiner PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus_late_joiner fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus_late_joiner PROPERTIES INSTALL_RPATH "$ORIGIN")
gtest_discover_tests(test_connext_dds_simulation_bus_late_joiner
    TIMEOUT 40
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)

##################################################################
# Test of Connext DDS Simulation Bus Version
##################################################################

add_executable(test_connext_dds_simulation_bus_version
    tester_dds_simbus_version.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus_version PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

target_link_libraries(test_connext_dds_simulation_bus_version PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
    participant_private_test_utils)

set_target_properties(test_connext_dds_simulation_bus_version PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_version PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus_version fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus_version PROPERTIES INSTALL_RPATH "$ORIGIN")

gtest_discover_tests(test_connext_dds_simulation_bus_version
    TIMEOUT 30
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)

##################################################################
# Test of Connext DDS Simulation Bus Performance
##################################################################

add_executable(test_connext_dds_simulation_bus_performance
    tester_dds_simbus_performance.cpp
    USER_QOS_PROFILES.xml)
target_compile_definitions(test_connext_dds_simulation_bus_performance PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

target_include_directories(test_connext_dds_simulation_bus_performance PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

# TODO: add this test again  ################################
#add_test(NAME test_connext_dds_simulation_bus_performance  #
    #COMMAND test_connext_dds_simulation_bus_performance    #
    #TIMEOUT 30                                             #
    #WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"       #
#)                                                          #
#############################################################
target_link_libraries(test_connext_dds_simulation_bus_performance PRIVATE
    GTest::gtest_main
    fep3_participant
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
    participant_private_test_utils
)
set_target_properties(test_connext_dds_simulation_bus_performance PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_performance PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus_performance fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus_performance PROPERTIES INSTALL_RPATH "$ORIGIN")

##################################################################
# Test of Connext DDS Service Discovery
##################################################################

add_executable(test_connext_dds_service_discovery
    tester_dds_service_discovery.cpp
    tester_dds_service_topic_handler.cpp
    tester_service_discovery_dds_topic_listener.cpp
    tester_dds_service_update_handler.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_service_discovery PRIVATE
    "${CMAKE_PROJ_DIR}/src/plugins/rti_dds"
    ${PROJECT_SOURCE_DIR}/test/utils/helper
    ${PROJECT_SOURCE_DIR}/src/fep3/components/logging/mock/
    ${PROJECT_SOURCE_DIR}/include)

if(MSVC)
    # we have to find the dds dlls for this test
    set(TEST_CONNEXT_DDS_DISCOVERY_WORKING_DIR "rti")
    set_target_properties(test_connext_dds_service_discovery PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:test_connext_dds_service_discovery>/${TEST_CONNEXT_DDS_DISCOVERY_WORKING_DIR})
else()
    set(TEST_CONNEXT_DDS_DISCOVERY_WORKING_DIR "")
endif()
target_link_libraries(test_connext_dds_service_discovery PRIVATE
    GTest::gtest_main
    GTest::gmock_main
    fep3_dds_service_discovery_lib
    fep_test_helper
    dev_essential::process
    participant_private_test_utils
    fep3_components_test)

set_target_properties(test_connext_dds_service_discovery PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_service_discovery PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_service_discovery fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_service_discovery PROPERTIES INSTALL_RPATH "$ORIGIN")

set_source_files_properties(
        tester_dds_service_discovery.cpp
        PROPERTIES COMPILE_DEFINITIONS $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)

if(MSVC)
    # warning coming from dds headers
    target_compile_options(test_connext_dds_service_discovery PRIVATE /wd4245 /wd4244 /wd4996)
endif()

gtest_discover_tests( test_connext_dds_service_discovery
    TIMEOUT 80
    WORKING_DIRECTORY $<TARGET_FILE_DIR:test_connext_dds_service_discovery>/${TEST_CONNEXT_DDS_DISCOVERY_WORKING_DIR}
)
