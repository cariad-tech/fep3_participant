# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


find_package(dev_essential 1.3.0 REQUIRED COMPONENTS process)
find_package(Boost 1.73.0 REQUIRED COMPONENTS thread)

add_custom_target(test_service_bus_stub_generator)
fep_generate_rpc_stubs_before_target(
    TARGET test_service_bus_stub_generator
    INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    CLIENT_CLASS_NAME test::rpc_stubs::TestInterfaceClient
    CLIENT_FILE_NAME testclientstub.h
    SERVER_CLASS_NAME test::rpc_stubs::TestInterfaceServer
    SERVER_FILE_NAME testserverstub.h
)
add_executable(test_service_bus tester_service_bus.cpp
                                tester_find_free_port.cpp
                                tester_service_registration.cpp
                                test_http_server.cpp
                                tester_service_bus_discovery_lssdp.cpp
                                tester_service_bus_create_access_server.cpp
                                tester_service_bus_service_update_event.cpp
                                tester_service_bus_native_and_base_mocks.h
                                test_system_access.cpp
                                tester_service_update_sink_registry.cpp
                                tester_update_event_sink_proxy.cpp)
add_dependencies(test_service_bus test_service_bus_stub_generator)

set_target_properties(test_service_bus PROPERTIES FOLDER "test/private/native_components/service_bus")
target_link_libraries(test_service_bus PRIVATE GTest::gtest_main
                                               GTest::gmock_main
                                               fep3_lssdp_service_discovery_lib
                                               fep3_participant_private_lib
                                               dev_essential::process
                                               fep_test_helper
                                               Boost::thread
                                               fep3_components_test
                                               participant_test_utils
                                               participant_private_test_utils)


target_include_directories(test_service_bus PRIVATE
                           ${CMAKE_CURRENT_BINARY_DIR}
                           ${PROJECT_SOURCE_DIR}/src/fep3/components/logging/mock/)
set_target_properties(test_service_bus PROPERTIES TIMEOUT 10)
target_compile_definitions(test_service_bus PUBLIC LSSDP_SERVICE_DISCOVERY)
gtest_discover_tests(test_service_bus WORKING_DIRECTORY ".." TEST_PREFIX LSSDP_)

# Service Bus RPC
add_custom_target(test_http_server_rpc_stub_generator)
fep_generate_rpc_stubs_before_target(
    TARGET test_http_server_rpc_stub_generator
    INPUT_FILE "${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/service_bus/http_server.json"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    CLIENT_CLASS_NAME test::rpc_stubs::TestHttpServerClientStub
    CLIENT_FILE_NAME test_http_server_client_stub.h
)
add_executable(test_http_server_rpc tester_http_server_rpc.cpp)
add_dependencies(test_http_server_rpc test_http_server_rpc_stub_generator)

gtest_discover_tests(test_http_server_rpc
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(test_http_server_rpc PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant_core
    fep3_participant_private_lib
    fep3_components_test
    participant_private_test_utils
    ${CMAKE_DL_LIBS}
)

target_include_directories(test_http_server_rpc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(test_http_server_rpc PROPERTIES FOLDER "test/private/native_components/service_bus")

internal_fep3_participant_deploy(test_http_server_rpc)

if (FEP3_USE_RTIDDS)
    ## DDS SERVICE BUS TEST
    add_executable(test_dds_service_bus tester_service_bus.cpp
                                    tester_service_registration.cpp
                                    tester_service_bus_discovery_dds.cpp
                                    tester_service_bus_service_update_event.cpp
                                    tester_service_bus_native_and_base_mocks.h
                                    ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h
                                    ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h)

    set_target_properties(test_dds_service_bus PROPERTIES FOLDER "test/private/native_components/service_bus")
    target_link_libraries(test_dds_service_bus PRIVATE GTest::gtest_main
                                                   GTest::gmock_main
                                                   fep3_participant_private_lib
                                                   fep3_dds_service_discovery_lib
                                                   dev_essential::process
                                                   Boost::thread
                                                   fep_test_helper
                                                   fep3_components_test
                                                   participant_test_utils)

    target_include_directories(test_dds_service_bus PRIVATE
                               ${CMAKE_CURRENT_BINARY_DIR}
                               ${PROJECT_SOURCE_DIR}/src/fep3/components/logging/mock/)

    if(MSVC)
        # we have to find the dds dlls for this test
        set(TEST_CONNEXT_DDS_DISCOVERY_WORKING_DIR "rti")
        set_target_properties(test_dds_service_bus PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:test_dds_service_bus>/${TEST_CONNEXT_DDS_DISCOVERY_WORKING_DIR})
    else()
        set(TEST_CONNEXT_DDS_DISCOVERY_WORKING_DIR "")
    endif()

    target_compile_definitions(test_dds_service_bus PUBLIC DDS_SERVICE_DISCOVERY)
    if(MSVC)
        # warning coming from dds headers
        target_compile_options(test_dds_service_bus PRIVATE /wd4245)
    endif()
    internal_fep3_participant_deploy_dds(test_dds_service_bus $<TARGET_FILE_DIR:test_dds_service_bus>)
    gtest_discover_tests(test_dds_service_bus
            DISCOVERY_TIMEOUT 10
            TEST_PREFIX DDS_
            PROPERTIES TIMEOUT 60
            WORKING_DIRECTORY $<TARGET_FILE_DIR:test_dds_service_bus>/${TEST_CONNEXT_DDS_DISCOVERY_WORKING_DIR})

endif()
