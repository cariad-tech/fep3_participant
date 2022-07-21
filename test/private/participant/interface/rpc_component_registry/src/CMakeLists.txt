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


project(test_rpc_component_registry)

add_executable(${PROJECT_NAME} tester_rpc_component_registry.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
                           $<TARGET_PROPERTY:fep3_participant,INTERFACE_INCLUDE_DIRECTORIES>
                           ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/participant/interface/rpc_component_registry")

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::gmock_main
    ${CMAKE_DL_LIBS}
    fep3_participant_private_lib
    participant_test_utils
    fep_test_helper
)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES TIMEOUT 10)



