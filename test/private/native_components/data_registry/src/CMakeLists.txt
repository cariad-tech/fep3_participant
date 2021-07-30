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


find_package(dev_essential REQUIRED COMPONENTS pkg_rpc ddl)

##################################################################
# Data Registry
##################################################################

add_executable(test_data_registry tester_data_registry.cpp
               data_registry_test_fixture.h
)

add_test(NAME test_data_registry
    COMMAND test_data_registry
    TIMEOUT 10
)

target_link_libraries(test_data_registry PRIVATE
    dev_essential::ddl
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
    participant_test_utils
)

set_target_properties(test_data_registry PROPERTIES FOLDER "test/private/native_components/data_registry")

##################################################################
# Mapping
##################################################################

add_executable(test_mapping tester_mapping.cpp
               data_registry_test_fixture.h
)

add_test(NAME test_mapping
    COMMAND test_mapping
    TIMEOUT 10
)

target_link_libraries(test_mapping PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    dev_essential::ddl
    fep3_participant_private_lib
)

target_compile_definitions(test_mapping PRIVATE
    TEST_FILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../files/"
)

set_target_properties(test_mapping PROPERTIES FOLDER "test/private/native_components/data_registry")
