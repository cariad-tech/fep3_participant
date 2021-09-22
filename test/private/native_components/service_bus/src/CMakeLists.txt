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


find_package(dev_essential REQUIRED COMPONENTS pkg_rpc process)

jsonrpc_generate_client_stub(${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json
                             test::rpc_stubs::TestInterfaceClient
                             ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h)
jsonrpc_generate_server_stub(${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json
                             test::rpc_stubs::TestInterfaceServer
                             ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h)


add_executable(test_service_bus tester_service_bus.cpp
                                tester_find_free_port.cpp
                                tester_service_registration.cpp
                                ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h
                                ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h)

set_target_properties(test_service_bus PROPERTIES FOLDER "test/private/native_components")
target_link_libraries(test_service_bus PRIVATE GTest::gtest_main
                                               fep3_participant_private_lib
                                               dev_essential::process)

target_include_directories(test_service_bus PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME test_service_bus COMMAND test_service_bus WORKING_DIRECTORY "..")
set_target_properties(test_service_bus PROPERTIES TIMEOUT 10)
